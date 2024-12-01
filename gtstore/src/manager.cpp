#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "gtstore.hpp"
#include "gtstore.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
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

class GTStoreServiceImpl final : public GTStore::Service {
    public:
    Status Init(ServerContext* context, const InitRequest* request, InitResponse* response) {
        response->set_success(true);
        std::cout << "Client " << request->client_id() << " initialized" << std::endl;
        return Status::OK;
    }

    Status Get(ServerContext* context, const GetRequest* request, GetResponse* response) {
        response->set_success(true);
        response->add_values("dummy_value");
        std::cout << "Get request for key: " << request->key() << std::endl;
        return Status::OK;
    }

    Status Put(ServerContext* context, const PutRequest* request, PutResponse* response) {
        response->set_success(true);
        std::cout << "Put request for key: " << request->key() << std::endl;
        return Status::OK;
    }

    Status Finalize(ServerContext* context, const FinalizeRequest* request, FinalizeResponse* response) {
        response->set_success(true);
        std::cout << "Client " << request->client_id() << " finalized" << std::endl;
        return Status::OK;
    }
};

void GTStoreManager::init() {
	std::string server_address("0.0.0.0:50051");
	GTStoreServiceImpl service;

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
}

int main(int argc, char** argv) {
	GTStoreManager manager;
	manager.init();
    return 0;
}
