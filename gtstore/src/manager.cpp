#include <memory>
#include <string>
#include <shared_mutex>
#include "gtstore.hpp"

class GTStoreManagerImpl final : public GTStoreManagerService::Service {
    public:
		GTStoreManagerImpl(int num_nodes, int num_replicas) {
			this->num_nodes = num_nodes;
			this->num_replicas = num_replicas;

			for (int i = 1; i < num_nodes; i++) {
				std::string server_address = "0.0.0.0:" + std::to_string(50000 + i);
				storage_node_status[server_address] = true;
			}
		}

		Status init(ServerContext* context, const ManagerInitRequest* request, ManagerInitResponse* response) {
			response->set_success(true);
			std::shared_lock<std::shared_mutex> lock(storage_node_mutex);
			for (auto& [node_address, status] : storage_node_status) {
				response->add_storage_nodes(node_address);
			}
			std::cout << "Client " << request->client_id() << " initialized" << std::endl;
			return Status::OK;
		}

		Status update_status(ServerContext* context, const ManagerUpdateStatusRequest* request, ManagerUpdateStatusResponse* response) {
			response->set_success(true);
			std::string node_address = request->storage_node();
			std::unique_lock<std::shared_mutex> lock(storage_node_mutex);
			storage_node_status[node_address] = true;
			return Status::OK;
		}

		Status get(ServerContext* context, const ManagerGetRequest* request, ManagerGetResponse* response) {
			std::string key = request->key();
			std::string storage_node = retrieve_get_storage_node();
			if (storage_node == "") {
				response->set_success(false);
				std::cout << "Get request for key: " << request->key() << " failed" << std::endl;
				return Status::OK;
			}
			response->set_storage_node(storage_node);
			response->set_success(true);
			std::cout << "Get request for key: " << request->key() << " routed to " << storage_node << std::endl;
			return Status::OK;
		}

		Status put(ServerContext* context, const ManagerPutRequest* request, ManagerPutResponse* response) {
			std::string key = request->key();
			std::vector<string> storage_nodes = retrieve_put_storage_nodes();
			if (storage_nodes.size() == 0) {
				response->set_success(false);
				std::cout << "Put request for key: " << request->key() << " failed" << std::endl;
				return Status::OK;
			}
			for (auto& storage_node : storage_nodes) {
				response->add_storage_nodes(storage_node);
			}
			response->set_success(true);
			std::cout << "Put request for key: " << request->key() << std::endl;
			return Status::OK;
		}

		Status report_failure(ServerContext* context, const ManagerReportFailureRequest* request, ManagerReportFailureResponse* response) {
			response->set_success(true);
			std::string node_address = request->storage_node();
			std::unique_lock<std::shared_mutex> lock(storage_node_mutex);
			storage_node_status[node_address] = false;
			return Status::OK;
		}

		Status finalize(ServerContext* context, const ManagerFinalizeRequest* request, ManagerFinalizeResponse* response) {
			response->set_success(true);
			std::cout << "Client " << request->client_id() << " finalized" << std::endl;
			return Status::OK;
		}

	private:
		int num_nodes;
		int num_replicas;
		std::map<string, bool> storage_node_status;
		std::shared_mutex storage_node_mutex;

		std::string retrieve_get_storage_node() {
			std::shared_lock<std::shared_mutex> lock(storage_node_mutex);
			for (auto& [node_address, status] : storage_node_status) {
				if (status) {
					return node_address;
				}
			}
			return "";
		}

		std::vector<string> retrieve_put_storage_nodes() {
			std::shared_lock<std::shared_mutex> lock(storage_node_mutex);
			std::vector<string> storage_nodes;
			for (auto& [node_address, status] : storage_node_status) {
				if (status) {
					storage_nodes.push_back(node_address);
					if (storage_nodes.size() == num_replicas) {
						break;
					}
				}
			}
			return storage_nodes;
		}
};

void GTStoreManager::init(int num_nodes, int num_replicas) {
	std::string server_address("0.0.0.0:50000");
	GTStoreManagerImpl service(num_nodes, num_replicas);

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <num_nodes> <num_replicas>" << std::endl;
		return 1;
	}

	int num_nodes = std::stoi(argv[1]);
	int num_replicas = std::stoi(argv[2]);

	GTStoreManager manager;
	manager.init(num_nodes, num_replicas);
    return 0;
}
