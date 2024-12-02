#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "gtstore.hpp"
#include "gtstore.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using gtstore::GTStoreManagerService;
using gtstore::ManagerInitRequest;
using gtstore::ManagerInitResponse;
using gtstore::ManagerGetRequest;
using gtstore::ManagerGetResponse;
using gtstore::ManagerPutRequest;
using gtstore::ManagerPutResponse;
using gtstore::ManagerFinalizeRequest;
using gtstore::ManagerFinalizeResponse;
using gtstore::GTStoreStorageService;
using gtstore::StorageGetRequest;
using gtstore::StorageGetResponse;
using gtstore::StoragePutRequest;
using gtstore::StoragePutResponse;

class GTStoreClientImpl {
    private:
        std::unique_ptr<GTStoreManagerService::Stub> manager_stub;
        int client_id;
		std::map<std::string, std::unique_ptr<GTStoreStorageService::Stub>> storage_node_stubs;

    public:
        GTStoreClientImpl(std::shared_ptr<Channel> channel)
            : manager_stub(GTStoreManagerService::NewStub(channel)) {}

        void init(int id) {
            ManagerInitRequest request;
            request.set_client_id(id);
            client_id = id;

            ManagerInitResponse response;
            ClientContext context;

            Status status = manager_stub->init(&context, request, &response);

            if (!status.ok()) {
                std::cout << "Init failed: " << status.error_message() << std::endl;
                return;
            }

			for (const auto& storage_node : response.storage_nodes()) {
				auto channel = grpc::CreateChannel(storage_node, grpc::InsecureChannelCredentials());
				storage_node_stubs[storage_node] = GTStoreStorageService::NewStub(channel);
			}
        }

        val_t get(std::string key) {
            ManagerGetRequest request;
            request.set_key(key);

            ManagerGetResponse response;
            ClientContext context;

            Status status = manager_stub->get(&context, request, &response);

            if (!status.ok()) {
                std::cout << "Get failed: " << status.error_message() << std::endl;
                return val_t();
            }

            string storage_node = response.storage_node();
			std::cout << "Get request for key: " << request.key() << " routed to " << storage_node << std::endl;
		
			val_t result;
			
			StorageGetRequest storage_get_request;
			storage_get_request.set_key(key);

			StorageGetResponse storage_get_response;
			ClientContext storage_context;

			Status storage_status = storage_node_stubs[storage_node]->get(&storage_context, storage_get_request, &storage_get_response);

			if (!storage_status.ok()) {
				std::cout << "Get failed: " << storage_status.error_message() << std::endl;
				return val_t();
			}

			for (const auto& value : storage_get_response.values()) {
				result.push_back(value);
			}

            return result;
        }

        bool put(std::string key, val_t value) {
            ManagerPutRequest request;
            request.set_key(key);

            ManagerPutResponse response;
            ClientContext context;

            Status status = manager_stub->put(&context, request, &response);

            if (!status.ok()) {
                std::cout << "Put failed: " << status.error_message() << std::endl;
                return false;
            }

			std::cout << "Put request for key: " << request.key() << " routed to: ";
			std::vector<std::string> storage_nodes;

			for (const auto& storage_node : response.storage_nodes()) {
				storage_nodes.push_back(storage_node);
				std::cout << storage_node << " ";
			}
			std::cout << std::endl;

			for (const auto& storage_node : storage_nodes) {
				StoragePutRequest storage_put_request;
				storage_put_request.set_key(key);

				for (const auto& val : value) {
					storage_put_request.add_values(val);
				}

				StoragePutResponse storage_put_response;
				ClientContext storage_context;

				Status storage_status = storage_node_stubs[storage_node]->put(&storage_context, storage_put_request, &storage_put_response);

				if (!storage_status.ok()) {
					std::cout << "Put failed: " << storage_status.error_message() << std::endl;
					return false;
				}
			}

            return true;
        }

        void finalize() {
            ManagerFinalizeRequest request;
            request.set_client_id(client_id);

            ManagerFinalizeResponse response;
            ClientContext context;

            Status status = manager_stub->finalize(&context, request, &response);

            if (!status.ok()) {
                std::cout << "Finalize failed: " << status.error_message() << std::endl;
                return;
            }
        }
};

// GTStoreClient implementation using the gRPC client

GTStoreClient::GTStoreClient() {
	impl = nullptr;
}

GTStoreClient::~GTStoreClient() {
	finalize();
}

void GTStoreClient::init(int id) {
    auto channel = grpc::CreateChannel("localhost:50000", grpc::InsecureChannelCredentials());
    impl = new GTStoreClientImpl(channel);
    impl->init(id);
    client_id = id;
}

void GTStoreClient::finalize() {
    if (impl) {
        impl->finalize();
        delete impl;
        impl = nullptr;
    }
}

val_t GTStoreClient::get(string key) {
    if (!impl) return val_t();
    return impl->get(key);
}

bool GTStoreClient::put(string key, val_t value) {
    if (!impl) return false;
    return impl->put(key, value);
}
