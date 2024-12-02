#include <memory>
#include <string>
#include <shared_mutex>
#include <grpcpp/grpcpp.h>
#include "gtstore.hpp"
#include "gtstore.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using gtstore::GTStoreStorageService;
using gtstore::StorageGetRequest;
using gtstore::StorageGetResponse;
using gtstore::StoragePutRequest;
using gtstore::StoragePutResponse;
using gtstore::GTStoreManagerService;
using gtstore::ManagerUpdateStatusRequest;
using gtstore::ManagerUpdateStatusResponse;

class GTStoreStorageImpl final : public GTStoreStorageService::Service {
    public:
		GTStoreStorageImpl(string node_address, std::shared_ptr<Channel> channel) : node_address (node_address), manager_stub(GTStoreManagerService::NewStub(channel)) {
			ManagerUpdateStatusRequest request;
			request.set_storage_node(node_address);
			ManagerUpdateStatusResponse response;
			ClientContext context;
			Status status = manager_stub->update_status(&context, request, &response);

			std::cout << "Storage node " << node_address << " initialized" << std::endl;
		}

		Status get(ServerContext* context, const StorageGetRequest* request, StorageGetResponse* response) override {
			std::string key = request->key();
			std::shared_lock<std::shared_mutex> lock(kv_store_mutex);
			std::vector<string> values = kv_store[key];

			for (auto& value : values) {
				response->add_values(value);
			}

			response->set_success(true);
			return Status::OK;
		}

		Status put(ServerContext* context, const StoragePutRequest* request, StoragePutResponse* response) override {
			std::string key = request->key();
			std::vector<string> values;

			for (const auto& value : request->values()) {
				values.push_back(value);
			}

			std::unique_lock<std::shared_mutex> lock(kv_store_mutex);
			kv_store[key] = values;
			response->set_success(true);
			return Status::OK;
		}

	private:
		string node_address;
		std::unordered_map<string, vector<string>> kv_store;
		std::shared_mutex kv_store_mutex;
		std::unique_ptr<GTStoreManagerService::Stub> manager_stub;
};

void GTStoreStorage::init(int node_id) {
	string manager_address = "0.0.0.0:50000";
	string node_address = "0.0.0.0:" + std::to_string(50000 + node_id);

	auto channel = grpc::CreateChannel(manager_address, grpc::InsecureChannelCredentials());
	GTStoreStorageImpl service(node_address, channel);

	ServerBuilder builder;
	builder.AddListeningPort(node_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server storage node on " << node_address << std::endl;
	server->Wait();
}

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <node_id>" << std::endl;
		return 1;
	}

	int node_id = std::stoi(argv[1]);

	GTStoreStorage storage;
	storage.init(node_id);
	
}
