#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}Running Multi Node Failure Test...${NC}"

# Start service with 7 nodes and 3 replicas
./start_service.sh 7 3

# Run initial operations
echo "Test 4: Multi Node Failure Test"

# Put initial data
for i in {1..20}
do
    ./build/client --put key${i} --val value${i} --id 1 --verbose
done

# Overwrite data
echo -e "\n${GREEN}Overwrite data...${NC}"
./build/client --put key1 --val value21 --id 1 --verbose
./build/client --put key2 --val value22 --id 1 --verbose
./build/client --put key3 --val value23 --id 1 --verbose
./build/client --put key4 --val value24 --id 1 --verbose
./build/client --put key5 --val value25 --id 1 --verbose

# Verify data before failures
echo -e "\n${GREEN}Data before node failures:${NC}"
for i in {1..20}
do
    ./build/client --get key${i} --id 1 --verbose
done

# Kill first node
echo -e "\n${GREEN}Killing storage node 1...${NC}"
pkill -f "./build/storage 1"
sleep 3

# Kill second node
echo -e "\n${GREEN}Killing storage node 3...${NC}"
pkill -f "./build/storage 3"
sleep 3

# Verify data after node failures
echo -e "\n${GREEN}Data after node failures:${NC}"
for i in {1..20}
do
    ./build/client --get key${i} --id 1 --verbose
done

# Try new operations after failures
echo -e "\n${GREEN}New operations after node failures:${NC}"
./build/client --put key1 --val value26 --id 1 --verbose
./build/client --get key1 --id 1 --verbose

# Clean up
./clean.sh