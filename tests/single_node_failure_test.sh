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
for i in {1..10}
do
    ./build/client --put key${i} --val value${i} --id 1 --verbose
done

# Overwrite data
echo -e "\n${GREEN}Overwrite data...${NC}"
./build/client --put key1 --val value11 --id 1 --verbose

# Verify data before failure
echo -e "\n${GREEN}Data before node failure:${NC}"
for i in {1..10}
do
    ./build/client --get key${i} --id 1 --verbose
done

# Kill a specific storage node (node 3)
echo -e "\n${GREEN}Killing storage node 3...${NC}"
pkill -f "./build/storage 3"
sleep 3

# Try operations after node failure
echo -e "\n${GREEN}Data after node failure:${NC}"
for i in {1..10}
do
    ./build/client --get key${i} --id 1 --verbose
done

# Try new operations after failure
echo -e "\n${GREEN}New operations after node failure:${NC}"
./build/client --put key1 --val value12 --id 1 --verbose
./build/client --get key1 --id 1 --verbose

# Clean up
./clean.sh