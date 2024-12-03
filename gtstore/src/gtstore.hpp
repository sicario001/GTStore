#ifndef GTSTORE
#define GTSTORE

#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

#include <grpcpp/grpcpp.h>
#include "gtstore.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
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
using gtstore::ManagerReportFailureRequest;
using gtstore::ManagerReportFailureResponse;
using gtstore::ManagerUpdateStatusRequest;
using gtstore::ManagerUpdateStatusResponse;
using gtstore::GTStoreStorageService;
using gtstore::StorageGetRequest;
using gtstore::StorageGetResponse;
using gtstore::StoragePutRequest;
using gtstore::StoragePutResponse;
using gtstore::StorageCommitPutRequest;
using gtstore::StorageCommitPutResponse;
using gtstore::StorageAbortPutRequest;
using gtstore::StorageAbortPutResponse;

#define MAX_KEY_BYTE_PER_REQUEST 20
#define MAX_VALUE_BYTE_PER_REQUEST 1000

using namespace std;

typedef vector<string> val_t;

// Forward declaration of implementation class
class GTStoreClientImpl;

class GTStoreClient {
		private:
				int client_id;
				val_t value;
				GTStoreClientImpl* impl;  // Pointer to implementation
		public:
				GTStoreClient();
				~GTStoreClient();
				void init(int id, bool verbose = false);
				void finalize();
				val_t get(string key);
				bool put(string key, val_t value);
};

class GTStoreManager {
		public:
				void init(int num_nodes, int num_replicas);
};

class GTStoreStorage {
		public:
				void init(int node_id);
};

#endif
