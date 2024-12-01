#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "gtstore.hpp"
#include "gtstore.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using gtstore::GTStore;
using gtstore::InitRequest;
using gtstore::InitResponse;
using gtstore::GetRequest;
using gtstore::GetResponse;
using gtstore::PutRequest;
using gtstore::PutResponse;
using gtstore::FinalizeRequest;
using gtstore::FinalizeResponse;

class GTStoreClientImpl {
    private:
        std::unique_ptr<GTStore::Stub> stub_;
        int client_id;

    public:
        GTStoreClientImpl(std::shared_ptr<Channel> channel)
            : stub_(GTStore::NewStub(channel)) {}

        void init(int id) {
            InitRequest request;
            request.set_client_id(id);
            client_id = id;

            InitResponse response;
            ClientContext context;

            Status status = stub_->Init(&context, request, &response);

            if (!status.ok()) {
                std::cout << "Init failed: " << status.error_message() << std::endl;
                return;
            }
        }

        val_t get(std::string key) {
            GetRequest request;
            request.set_key(key);

            GetResponse response;
            ClientContext context;

            Status status = stub_->Get(&context, request, &response);

            if (!status.ok()) {
                std::cout << "Get failed: " << status.error_message() << std::endl;
                return val_t();
            }

            val_t result;
            for (const auto& value : response.values()) {
                result.push_back(value);
            }
            return result;
        }

        bool put(std::string key, val_t value) {
            PutRequest request;
            request.set_key(key);
            for (const auto& v : value) {
                request.add_values(v);
            }

            PutResponse response;
            ClientContext context;

            Status status = stub_->Put(&context, request, &response);

            if (!status.ok()) {
                std::cout << "Put failed: " << status.error_message() << std::endl;
                return false;
            }

            return response.success();
        }

        void finalize() {
            FinalizeRequest request;
            request.set_client_id(client_id);

            FinalizeResponse response;
            ClientContext context;

            Status status = stub_->Finalize(&context, request, &response);

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
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
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
