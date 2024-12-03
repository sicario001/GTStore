#include "gtstore.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <unordered_map>
#include <thread>
#include <getopt.h>
#include <iomanip>
#include <limits>
#include <atomic>

// Helper function to generate random strings
std::string random_string(int length) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    
    std::string str;
    str.reserve(length);
    for (int i = 0; i < length; ++i) {
        str += alphanum[dis(gen)];
    }
    return str;
}

void print_usage() {
    std::cout << "Usage: benchmark [options]\n"
              << "Options:\n"
              << "  --throughput [replicas] Run single client throughput benchmark\n"
              << "  --concurrent [replicas] [threads] Run concurrent throughput benchmark\n"
              << "  --loadbalance                    Run load balance benchmark\n"
              << "  --help                           Show this help message\n";
}

void client_thread(int thread_id, int ops_per_thread, std::atomic<int>& successful_ops) {
    GTStoreClient client;
    client.init(thread_id);

    for (int i = 0; i < ops_per_thread/2; i++) {
        std::string key = "key" + std::to_string(thread_id) + "_" + std::to_string(i);
        std::string value = "val" + std::to_string(i);
        
        try {
            if (!client.put(key, {value}).empty()) {
                successful_ops++;
            }
        }
        catch (const std::exception& e) {
            // Operation failed, continue
        }

        try {
            val_t val = client.get(key);
            if (val.size() == 1 && val[0] == value) {
                successful_ops++;
            }
        }
        catch (const std::exception& e) {
            // Operation failed, continue
        }
    }

    client.finalize();
}

void throughput_test(int num_ops, int replicas, int num_threads) {
    std::ofstream outfile("throughput_results.txt", std::ios::app);
    
    std::cout << "\n=== Running concurrent test with " << replicas << " replicas and "
              << num_threads << " threads ===" << std::endl;
    
    std::atomic<int> successful_ops(0);
    std::vector<std::thread> threads;
    
    int ops_per_thread = num_ops / num_threads;
    
    // Start timing and create threads
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(client_thread, i, ops_per_thread, std::ref(successful_ops));
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double throughput = static_cast<double>(successful_ops) / (duration.count() / 1000.0);
    std::cout << "Total duration: " << duration.count() << " ms" << std::endl;
    std::cout << "Throughput with " << replicas << " replicas: " << std::fixed 
              << std::setprecision(2) << throughput << " ops/sec (success rate: " 
              << (successful_ops * 100.0 / num_ops) << "%)" << std::endl;
    
    outfile << replicas << " " << num_threads << " " << throughput << std::endl;
    
    // Sleep to allow system to stabilize
    std::cout << "Waiting for system to stabilize..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

void single_client_throughput_test(int num_ops, int replicas) {
    std::ofstream outfile("single_client_results.txt", std::ios::app);
    
    std::cout << "\n=== Running single client throughput test with " << replicas << " replicas ===" << std::endl;
    
    GTStoreClient client;
    client.init(1);
    
    auto get_duration = std::chrono::microseconds(0);
    auto put_duration = std::chrono::microseconds(0);
    auto start = std::chrono::high_resolution_clock::now();
    
    int successful_ops = 0;
    for (int i = 0; i < num_ops/2; i++) {
        std::string key = "key" + std::to_string(i % 1000); // Use 1000 different keys
        std::string value = "val" + std::to_string(i);
        
        try {
            // PUT
            auto op_start = std::chrono::high_resolution_clock::now();
            auto nodes = client.put(key, {value});
            auto op_end = std::chrono::high_resolution_clock::now();
            put_duration += std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start);
            if (!nodes.empty()) {
                successful_ops++;
            }
            else {
                std::cout << "PUT operation failed for key " << key << std::endl;
            }
        }
        catch (const std::exception& e) {
            // Operation failed, continue with next one
        }

        try {
            // GET
            auto op_start = std::chrono::high_resolution_clock::now();
            val_t val = client.get(key);
            auto op_end = std::chrono::high_resolution_clock::now();
            get_duration += std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start);
            if (val.size() == 1 && val[0] == value) {
                successful_ops++;
            }
        }
        catch (const std::exception& e) {
            // Operation failed, continue with next one
        }
        
        if ((2 * i) % 100 == 0) {
            double progress = (2 * i * 100.0) / num_ops;
            std::cout << "\rProgress: " << std::fixed << std::setprecision(1) << progress << "% (" 
                     << 2*i << "/" << num_ops << " operations)" << std::flush;
        }
    }
    std::cout << "\rProgress: 100% (" << num_ops << "/" << num_ops << " operations)" << std::endl;
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double throughput = static_cast<double>(successful_ops) / (duration.count() / 1000.0);
    double put_throughput = static_cast<double>(successful_ops/2) / (put_duration.count() / 1000000.0);
    double get_throughput = static_cast<double>(successful_ops/2) / (get_duration.count() / 1000000.0);
    
    std::cout << "Total duration: " << duration.count() << " ms" << std::endl;
    std::cout << "PUT throughput: " << std::fixed << std::setprecision(2) << put_throughput << " ops/sec" << std::endl;
    std::cout << "GET throughput: " << std::fixed << std::setprecision(2) << get_throughput << " ops/sec" << std::endl;
    std::cout << "Overall throughput with " << replicas << " replicas: " << std::fixed << std::setprecision(2) 
              << throughput << " ops/sec (success rate: " 
              << (successful_ops * 100.0 / num_ops) << "%)" << std::endl;
    
    outfile << replicas << " " << throughput << " " << put_throughput << " " << get_throughput << std::endl;
    
    client.finalize();
    
    // Sleep to allow system to stabilize between tests
    std::cout << "Waiting for system to stabilize..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

void loadbalance_test(int num_inserts) {
    GTStoreClient client;
    client.init(1);
    
    std::unordered_map<std::string, int> node_counts;
    std::ofstream outfile("loadbalance_results.txt");
    
    std::cout << "\n=== Running load balance test ===" << std::endl;
    
    // Perform inserts with random keys
    for (int i = 0; i < num_inserts; i++) {
        std::string key = random_string(10);
        std::string value = random_string(10);
        
        try {
            std::vector<std::string> nodes = client.put(key, {value});
            for (const auto& node : nodes) {
                node_counts[node]++;
            }
        }
        catch (const std::exception& e) {
            // Skip failed operations
        }
        
        if (i % 1000 == 0) {
            double progress = (i * 100.0) / num_inserts;
            std::cout << "\rProgress: " << std::fixed << std::setprecision(1) << progress << "% (" 
                     << i << "/" << num_inserts << " inserts)" << std::flush;
        }
    }
    std::cout << "\rProgress: 100% (" << num_inserts << "/" << num_inserts << " inserts)" << std::endl;
    
    // Calculate statistics
    int total_keys = 0;
    int min_keys = std::numeric_limits<int>::max();
    int max_keys = 0;
    for (const auto& pair : node_counts) {
        total_keys += pair.second;
        min_keys = std::min(min_keys, pair.second);
        max_keys = std::max(max_keys, pair.second);
    }
    
    double avg_keys = static_cast<double>(total_keys) / node_counts.size();
    double imbalance = static_cast<double>(max_keys - min_keys) / avg_keys * 100.0;
    
    std::cout << "\nLoad Balance Statistics:" << std::endl;
    std::cout << "- Number of nodes: " << node_counts.size() << std::endl;
    std::cout << "- Average keys per node: " << std::fixed << std::setprecision(2) << avg_keys << std::endl;
    std::cout << "- Min keys on a node: " << min_keys << std::endl;
    std::cout << "- Max keys on a node: " << max_keys << std::endl;
    std::cout << "- Imbalance factor: " << std::fixed << std::setprecision(2) << imbalance << "%" << std::endl;
    
    // Write results
    for (const auto& pair : node_counts) {
        outfile << pair.first << " " << pair.second << std::endl;
    }
    
    outfile.close();
    client.finalize();
}

int main(int argc, char** argv) {
    static struct option long_options[] = {
        {"throughput", required_argument, 0, 't'},
        {"concurrent", required_argument, 0, 'c'},
        {"loadbalance", no_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    bool run_throughput = false;
    bool run_concurrent = false;
    bool run_loadbalance = false;
    int replicas = 0;
    int num_threads = 1;

    int opt;
    while ((opt = getopt_long(argc, argv, "t:c:lh", long_options, nullptr)) != -1) {
        switch (opt) {
            case 't':
                run_throughput = true;
                replicas = std::atoi(optarg);
                break;
            case 'c':
                run_concurrent = true;
                if (optind < argc) {
                    replicas = std::atoi(optarg);
                    num_threads = std::atoi(argv[optind]);
                }
                break;
            case 'l':
                run_loadbalance = true;
                break;
            case 'h':
                print_usage();
                return 0;
            default:
                print_usage();
                return 1;
        }
    }

    if (!run_throughput && !run_concurrent && !run_loadbalance) {
        std::cerr << "Error: Must specify either --throughput <replicas>, --concurrent <replicas> <threads>, or --loadbalance\n";
        return 1;
    }

    if (run_throughput) {
        if (replicas <= 0) {
            std::cerr << "Error: Number of replicas must be positive\n";
            return 1;
        }
        single_client_throughput_test(2000, replicas);
    }

    if (run_concurrent) {
        if (replicas <= 0 || num_threads <= 0) {
            std::cerr << "Error: Number of replicas and threads must be positive\n";
            return 1;
        }
        throughput_test(2000, replicas, num_threads);
    }

    if (run_loadbalance) {
        loadbalance_test(1000);
    }

    return 0;
}
