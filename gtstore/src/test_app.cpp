#include "gtstore.hpp"
#include <iostream>
#include <string>
#include <vector>

void single_set_get(int client_id) {
	// Create client
    GTStoreClient client;
    
    // Initialize client
    std::cout << "Initializing client..." << std::endl;
    client.init(client_id);

    // Test put operation
    std::cout << "\nTesting put operation..." << std::endl;
    val_t test_value = {"value1", "value2", "value3"};
    bool put_result = client.put("test_key", test_value);
    std::cout << "Put result: " << (put_result ? "success" : "failure") << std::endl;

    // Test get operation
    std::cout << "\nTesting get operation..." << std::endl;
    val_t retrieved_value = client.get("test_key");
    std::cout << "Retrieved values: ";
    for (const auto& value : retrieved_value) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    // Finalize client
    std::cout << "\nFinalizing client..." << std::endl;
    client.finalize();
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <test_name> <client_id>" << std::endl;
		return 1;
	}

	std::string test_name = argv[1];
	int client_id = std::stoi(argv[2]);

	if (test_name == "single_set_get") {
		single_set_get(client_id);
	}
    else {
		std::cerr << "Invalid test name: " << test_name << std::endl;
		return 1;
	}

    return 0;
}
