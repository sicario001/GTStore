#ifndef GTSTORE
#define GTSTORE

#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <unistd.h>
#include <sys/wait.h>

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
				void init(int id);
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
