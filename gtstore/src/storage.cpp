#include <memory>
#include <string>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include "gtstore.hpp"

class GTStoreStorageImpl final : public GTStoreStorageService::Service {
    public:
        GTStoreStorageImpl(string node_address, std::shared_ptr<Channel> channel) : node_address(node_address), manager_stub(GTStoreManagerService::NewStub(channel)) {
            ManagerUpdateStatusRequest request;
            request.set_storage_node(node_address);
            ManagerUpdateStatusResponse response;
            ClientContext context;
            Status status = manager_stub->update_status(&context, request, &response);
        }

        Status get(ServerContext* context, const StorageGetRequest* request, StorageGetResponse* response) override {
            std::string key = request->key();
            std::shared_lock<std::shared_mutex> lock(kv_store_mutex);

			if (kv_store.find(key) == kv_store.end()) {
				response->set_success(false);
				return Status::OK;
			}

            std::vector<string> values = kv_store[key];

            for (auto& value : values) {
                response->add_values(value);
            }

            response->set_success(true);
            return Status::OK;
        }

        Status prepare_put(ServerContext* context, const StoragePutRequest* request, StoragePutResponse* response) override {
            std::string key = request->key();
            std::vector<string> values;

            for (const auto& value : request->values()) {
                values.push_back(value);
            }

            {
                std::unique_lock<std::mutex> lock(transactions_mutex);
                transaction_cv.wait(lock, [this, &key] { 
                    return transactions.find(key) == transactions.end(); 
                });
                transactions[key] = values;
            }

            response->set_success(true);
            return Status::OK;
        }

        Status commit_put(ServerContext* context, const StorageCommitPutRequest* request, StorageCommitPutResponse* response) override {
            std::string key = request->key();
            {
                std::unique_lock<std::mutex> trans_lock(transactions_mutex);
                if (transactions.find(key) != transactions.end()) {
                    std::unique_lock<std::shared_mutex> kv_lock(kv_store_mutex);
                    kv_store[key] = transactions[key];
                    transactions.erase(key);
                }
                transaction_cv.notify_all();
            }
            response->set_success(true);
            return Status::OK;
        }

        Status abort_put(ServerContext* context, const StorageAbortPutRequest* request, StorageAbortPutResponse* response) override {
            std::string key = request->key();
            {
                std::unique_lock<std::mutex> lock(transactions_mutex);
                transactions.erase(key);
                transaction_cv.notify_all();
            }
            response->set_success(true);
            return Status::OK;
        }

    private:
        string node_address;
        std::unordered_map<string, vector<string>> kv_store;
        std::unordered_map<string, vector<string>> transactions;
        std::shared_mutex kv_store_mutex;
        std::mutex transactions_mutex;
        std::condition_variable transaction_cv;
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
    std::cout << "Storage node initialized on " << node_address << std::endl;
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
