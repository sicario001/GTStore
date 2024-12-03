#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# Function to run throughput test
run_throughput_test() {
    local replicas=$1
    echo -e "\n${GREEN}Running throughput test with $replicas replicas...${NC}"
    
    # Start service with 7 nodes and specified replicas
    ./start_service.sh 7 $replicas
    sleep 3
    
    # Run benchmark
    ./build/benchmark --throughput $replicas
    
    # Clean up
    ./clean.sh
    sleep 2
}

# Function to run load balance test
run_loadbalance_test() {
    echo -e "\n${GREEN}Running load balance test...${NC}"
    
    # Start service with 7 nodes and 1 replica
    ./start_service.sh 7 1
    sleep 3
    
    # Run benchmark
    ./build/benchmark --loadbalance
    
    # Clean up
    ./clean.sh
    sleep 2
}

# Function to run single client test
run_single_client_test() {
    local replicas=$1
    echo -e "\n${GREEN}Running single client test with $replicas replicas...${NC}"
    
    # Start service
    ./start_service.sh 7 $replicas
    sleep 3

    # Run benchmark
    ./build/benchmark --throughput $replicas

    # Clean up
    ./clean.sh
    sleep 2
}

# Function to run concurrent test
run_concurrent_test() {
    local replicas=$1
    local clients=$2
    echo -e "\n${GREEN}Running concurrent test with $replicas replicas and $clients clients...${NC}"

    # Start service
    ./start_service.sh 7 $replicas
    sleep 3

    # Run benchmark
    ./build/benchmark --concurrent $replicas $clients
    
    # Clean up
    ./clean.sh
    sleep 2
}

# Main execution
echo -e "${GREEN}Starting GTStore Performance Benchmarks${NC}"

# Remove old results
rm -f throughput_results.txt loadbalance_results.txt single_client_results.txt

# Run single client tests for different replica counts
echo -e "${GREEN}Running single client tests...${NC}"
for replicas in 1 3 5; do
    run_single_client_test $replicas
done

# Run concurrent tests for different replica counts
echo -e "${GREEN}Running concurrent tests...${NC}"
for replicas in 1 3 5; do
    run_concurrent_test $replicas 8
done

# Run load balance test
run_loadbalance_test

# Remove old plots
rm -f *.png

# Plot results
python3 plot_results.py

echo -e "${GREEN}Benchmarks completed! Results saved in throughput.png and loadbalance.png${NC}"
