#include "gtstore.hpp"
#include <iostream>
#include <string>
#include <vector>

int main() {
    GTStoreClient client;
    
    // Initialize client
    std::cout << "Initializing client..." << std::endl;
    client.init(1);

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

    return 0;
}
