#include <memory>
#include <string>
#include <unordered_map>
#include <set>
#include <shared_mutex>
#include "gtstore.hpp"

class GTStoreManagerImpl final : public GTStoreManagerService::Service {
    public:
		GTStoreManagerImpl(int num_nodes, int num_replicas, int num_virtual_replicas = 5) {
			this->num_nodes = num_nodes;
			this->num_replicas = num_replicas;
			this->num_virtual_replicas = num_virtual_replicas;

			for (int i = 1; i < num_nodes; i++) {
				std::string server_address = "0.0.0.0:" + std::to_string(50000 + i);
				storage_node_status[server_address] = false;
			}
		}

		Status init(ServerContext* context, const ManagerInitRequest* request, ManagerInitResponse* response) {
			response->set_success(true);
			std::shared_lock<std::shared_mutex> lock(storage_mutex);
			for (auto& [node_address, status] : storage_node_status) {
				response->add_storage_nodes(node_address);
			}
			return Status::OK;
		}

		Status update_status(ServerContext* context, const ManagerUpdateStatusRequest* request, ManagerUpdateStatusResponse* response) {
			response->set_success(true);
			std::string node_address = request->storage_node();
			std::unique_lock<std::shared_mutex> lock(storage_mutex);
			storage_node_status[node_address] = true;

			for (int j = 0; j < num_virtual_replicas; j++) {
				std::string virtual_node_address = node_address + "_" + std::to_string(j);
				size_t address_hash = hasher(virtual_node_address);
				address_hashes.insert(address_hash);
				hash_to_address[address_hash] = node_address;
			}

			return Status::OK;
		}

		Status get(ServerContext* context, const ManagerGetRequest* request, ManagerGetResponse* response) {
			std::string key = request->key();
			std::string storage_node = retrieve_get_storage_node(key);
			if (storage_node == "") {
				response->set_success(false);
				return Status::OK;
			}
			response->set_storage_node(storage_node);
			response->set_success(true);
			return Status::OK;
		}

		Status put(ServerContext* context, const ManagerPutRequest* request, ManagerPutResponse* response) {
			std::string key = request->key();
			std::vector<string> storage_nodes = retrieve_put_storage_nodes(key);
			if (storage_nodes.size() == 0) {
				response->set_success(false);
				return Status::OK;
			}
			for (auto& storage_node : storage_nodes) {
				response->add_storage_nodes(storage_node);
			}
			response->set_success(true);
			return Status::OK;
		}

		Status report_failure(ServerContext* context, const ManagerReportFailureRequest* request, ManagerReportFailureResponse* response) {
			response->set_success(true);
			std::string node_address = request->storage_node();
			std::unique_lock<std::shared_mutex> lock(storage_mutex);
			storage_node_status[node_address] = false;

			for (int j = 0; j < num_virtual_replicas; j++) {
				std::string virtual_node_address = node_address + "_" + std::to_string(j);
				size_t address_hash = hasher(virtual_node_address);
				address_hashes.erase(address_hash);
			}

			return Status::OK;
		}

		Status finalize(ServerContext* context, const ManagerFinalizeRequest* request, ManagerFinalizeResponse* response) {
			response->set_success(true);
			return Status::OK;
		}

	private:
		int num_nodes;
		int num_replicas;
		int num_virtual_replicas;

		std::unordered_map<string, bool> storage_node_status;
		std::shared_mutex storage_mutex;

		std::hash<std::string> hasher;
		std::set<size_t> address_hashes;
		std::unordered_map<size_t, string> hash_to_address;

		std::string retrieve_get_storage_node(std::string& key) {
			size_t key_hash = hasher(key);
			std::shared_lock<std::shared_mutex> lock(storage_mutex);

			if (address_hashes.empty()) {
				return "";
			}

			auto it = address_hashes.lower_bound(key_hash);

			if (it == address_hashes.end()) {
				it = address_hashes.begin();
			}

			std::string storage_node = hash_to_address[*it];
			return storage_node;
		}

		std::vector<string> retrieve_put_storage_nodes(std::string& key) {
			size_t key_hash = hasher(key);
			std::set<string> storage_nodes;
			std::shared_lock<std::shared_mutex> lock(storage_mutex);
			
			if (address_hashes.empty()) {
				return std::vector<string>();
			}

			auto it_begin = address_hashes.lower_bound(key_hash);

			if (it_begin == address_hashes.end()) {
				it_begin = address_hashes.begin();
			}

			auto it_end = it_begin;

			do {
				storage_nodes.insert(hash_to_address[*it_end]);
				it_end++;

				if (it_end == address_hashes.end()) {
					it_end = address_hashes.begin();
				}
			}
			while (it_end != it_begin && storage_nodes.size() < num_replicas);
			
			return std::vector<string>(storage_nodes.begin(), storage_nodes.end());
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
