#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}Running Single Node Failure Test...${NC}"

# Start service with 3 nodes and 2 replicas
./start_service.sh 3 2

# Run initial operations
echo "Test 3: Single Node Failure Test"

# Put initial data
./build/client --put key1 --val value1 --id 1 --verbose
./build/client --put key2 --val value2 --id 1 --verbose
./build/client --put key3 --val value3 --id 1 --verbose

# Verify data before failure
echo -e "\n${GREEN}Data before node failure:${NC}"
./build/client --get key1 --id 1 --verbose
./build/client --get key2 --id 1 --verbose
./build/client --get key3 --id 1 --verbose

# Kill a specific storage node (node 2)
echo -e "\n${GREEN}Killing storage node 2...${NC}"
pkill -f "./build/storage 2"
sleep 3

# Try operations after node failure
echo -e "\n${GREEN}Data after node failure:${NC}"
./build/client --get key1 --id 1 --verbose
./build/client --get key2 --id 1 --verbose
./build/client --get key3 --id 1 --verbose

# Try new operations after failure
echo -e "\n${GREEN}New operations after node failure:${NC}"
./build/client --put key4 --val value4 --id 1 --verbose
./build/client --get key4 --id 1 --verbose

# Clean up
./clean.sh