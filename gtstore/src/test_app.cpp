#include "gtstore.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <getopt.h>

void print_usage() {
    std::cout << "Usage: client [options]\n"
              << "Options:\n"
              << "  --put <key>         Put a key\n"
              << "  --val <value>       Value for put operation (required with --put)\n"
              << "  --get <key>         Get a key\n"
              << "  --id <client_id>    Client ID (default: 1)\n"
              << "  --verbose           Enable verbose output\n"
              << "  --help              Show this help message\n";
}

int main(int argc, char** argv) {
    static struct option long_options[] = {
        {"put", required_argument, 0, 'p'},
        {"val", required_argument, 0, 'v'},
        {"get", required_argument, 0, 'g'},
        {"id", required_argument, 0, 'i'},
        {"verbose", no_argument, 0, 'V'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    std::string key;
    std::string value;
    int client_id = 1;
    bool is_put = false;
    bool is_get = false;
    bool verbose = false;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "p:v:g:i:Vh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                is_put = true;
                key = optarg;
                break;
            case 'v':
                value = optarg;
                break;
            case 'g':
                is_get = true;
                key = optarg;
                break;
            case 'i':
                client_id = std::stoi(optarg);
                break;
            case 'V':
                verbose = true;
                break;
            case 'h':
                print_usage();
                return 0;
            default:
                print_usage();
                return 1;
        }
    }

    // Validate arguments
    if (is_put && is_get) {
        std::cerr << "Error: Cannot specify both --put and --get\n";
        return 1;
    }

    if (!is_put && !is_get) {
        std::cerr << "Error: Must specify either --put or --get\n";
        return 1;
    }

    if (is_put && value.empty()) {
        std::cerr << "Error: Must specify --val with --put\n";
        return 1;
    }

    // Initialize client
    GTStoreClient client;
    client.init(client_id, verbose);

    // Perform operation
    if (is_get) {
        val_t result = client.get(key);
        if (!result.empty()) {
            return 0;
        } else {
            std::cerr << "Error: Get operation failed\n";
            return 1;
        }
    } else if (is_put) {
        bool success = client.put(key, {value});
        if (success) {
            return 0;
        } else {
            std::cerr << "Error: Put operation failed\n";
            return 1;
        }
    }

    client.finalize();
    return 0;
}
