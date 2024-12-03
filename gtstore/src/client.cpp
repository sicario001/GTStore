#include <memory>
#include <string>
#include "gtstore.hpp"

bool g_verbose = false;

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
				if (g_verbose) {
					std::cout << "Init failed: " << status.error_message() << std::endl;
				}
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

			val_t result;

			while (true) {
				ManagerGetResponse response;
				ClientContext context;

				Status status = manager_stub->get(&context, request, &response);

				if (!status.ok()) {
					if (g_verbose) {
						std::cout << "Get failed: " << status.error_message() << std::endl;
					}
					return val_t();
				}

				string storage_node = response.storage_node();
			
				StorageGetRequest storage_get_request;
				storage_get_request.set_key(key);

				StorageGetResponse storage_get_response;
				ClientContext storage_context;

				Status storage_status = storage_node_stubs[storage_node]->get(&storage_context, storage_get_request, &storage_get_response);

				if (!storage_status.ok() || !storage_get_response.success()) {
					// Report failure to manager
					ManagerReportFailureRequest report_failure_request;
					report_failure_request.set_storage_node(storage_node);
					ManagerReportFailureResponse report_failure_response;
					ClientContext report_failure_context;
					Status report_failure_status = manager_stub->report_failure(&report_failure_context, report_failure_request, &report_failure_response);

					if (!report_failure_status.ok()) {
						if (g_verbose) {
							std::cout << "Report failure failed: " << report_failure_status.error_message() << std::endl;
						}
						return val_t();
					}
				}
				else {
					if (g_verbose) std::cout << "<GET> " << request.key() << ", ";

					for (const auto& value : storage_get_response.values()) {
						result.push_back(value);
						if (g_verbose) std::cout << value << " ";
					}

					if (g_verbose) std::cout << ", from " << storage_node << std::endl;

					break;
				}
			}

            return result;
        }

        bool put(std::string key, val_t value) {
            ManagerPutRequest request;
            request.set_key(key);

			StoragePutRequest storage_put_request;
			storage_put_request.set_key(key);

			for (const auto& val : value) {
				storage_put_request.add_values(val);
			}

			while (true) {
                ManagerPutResponse response;
                ClientContext context;
            	Status status = manager_stub->put(&context, request, &response);

				if (!status.ok()) {
					if (g_verbose) {
						std::cout << "PUT failed: " << status.error_message() << std::endl;
					}
					return false;
				}

				std::vector<string> storage_nodes;
				for (const auto& storage_node : response.storage_nodes()) {
					storage_nodes.push_back(storage_node);
				}

				std::vector<string> storage_nodes_success;

				for (const auto& storage_node : storage_nodes) {
					StoragePutResponse storage_put_response;
					ClientContext storage_context;

					Status storage_status = storage_node_stubs[storage_node]->prepare_put(&storage_context, storage_put_request, &storage_put_response);

					if (!storage_status.ok()) {
						// Report failure to manager
						ManagerReportFailureRequest report_failure_request;
						report_failure_request.set_storage_node(storage_node);
						ManagerReportFailureResponse report_failure_response;
						ClientContext report_failure_context;
						Status report_failure_status = manager_stub->report_failure(&report_failure_context, report_failure_request, &report_failure_response);

						if (!report_failure_status.ok()) {
							if (g_verbose) {
								std::cout << "Report failure failed: " << report_failure_status.error_message() << std::endl;
							}
							return false;
						}
					}
					else {
						storage_nodes_success.push_back(storage_node);
					}
				}

				if (storage_nodes_success.size() != storage_nodes.size()) {
					for (const auto& storage_node : storage_nodes) {
						// Abort put transaction
						StorageAbortPutRequest abort_put_request;
						abort_put_request.set_key(key);
						StorageAbortPutResponse abort_put_response;
						ClientContext abort_put_context;
						Status abort_put_status = storage_node_stubs[storage_node]->abort_put(&abort_put_context, abort_put_request, &abort_put_response);
					}
				}
				else {
					if (g_verbose) {
						std::cout << "<PUT> " << key << ", ";

						for (const auto& val : value) {
							std::cout << val << " ";
						}

						std::cout << ", to ";
					}

					for (const auto& storage_node : storage_nodes_success) {
						// Commit put transaction
						StorageCommitPutRequest commit_put_request;
						commit_put_request.set_key(key);
						StorageCommitPutResponse commit_put_response;
						ClientContext commit_put_context;
						Status commit_put_status = storage_node_stubs[storage_node]->commit_put(&commit_put_context, commit_put_request, &commit_put_response);

						if (g_verbose) std::cout << storage_node << ", ";
					}

					if (g_verbose) std::cout << std::endl;
				}

				break;
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
                if (g_verbose) {
                    std::cout << "Finalize failed: " << status.error_message() << std::endl;
                }
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

void GTStoreClient::init(int id, bool verbose) {
    auto channel = grpc::CreateChannel("localhost:50000", grpc::InsecureChannelCredentials());
    impl = new GTStoreClientImpl(channel);
    impl->init(id);
    client_id = id;
	g_verbose = verbose;
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
