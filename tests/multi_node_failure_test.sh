#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}Running Multi Node Failure Test...${NC}"

# Start service with 5 nodes and 3 replicas
./start_service.sh 5 3

# Run initial operations
echo "Test 4: Multi Node Failure Test"

# Put initial data
./build/client --put key1 --val value1 --id 1 --verbose
./build/client --put key2 --val value2 --id 1 --verbose
./build/client --put key3 --val value3 --id 1 --verbose
./build/client --put key4 --val value4 --id 1 --verbose
./build/client --put key5 --val value5 --id 1 --verbose

# Verify data before failures
echo -e "\n${GREEN}Data before node failures:${NC}"
./build/client --get key1 --id 1 --verbose
./build/client --get key2 --id 1 --verbose
./build/client --get key3 --id 1 --verbose
./build/client --get key4 --id 1 --verbose
./build/client --get key5 --id 1 --verbose

# Kill first node
echo -e "\n${GREEN}Killing storage node 1...${NC}"
pkill -f "./build/storage 1"
sleep 3

# Verify data after first failure
echo -e "\n${GREEN}Data after first node failure:${NC}"
./build/client --get key1 --id 1 --verbose
./build/client --get key2 --id 1 --verbose
./build/client --get key3 --id 1 --verbose
./build/client --get key4 --id 1 --verbose
./build/client --get key5 --id 1 --verbose

# Kill second node
echo -e "\n${GREEN}Killing storage node 3...${NC}"
pkill -f "./build/storage 3"
sleep 3

# Verify data after second failure
echo -e "\n${GREEN}Data after second node failure:${NC}"
./build/client --get key1 --id 1 --verbose
./build/client --get key2 --id 1 --verbose
./build/client --get key3 --id 1 --verbose
./build/client --get key4 --id 1 --verbose
./build/client --get key5 --id 1 --verbose

# Try new operations after failures
echo -e "\n${GREEN}New operations after node failures:${NC}"
./build/client --put key6 --val value6 --id 1 --verbose
./build/client --get key6 --id 1 --verbose

# Clean up
./clean.sh