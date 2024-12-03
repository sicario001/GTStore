#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}Running Multi-Server Test...${NC}"

# Start service with 5 nodes and 3 replicas
./start_service.sh 5 3

# Run test sequence
echo "Test 2: Multi-Server GET/PUT"

./build/client --put key1 --val value1 --id 1 --verbose
./build/client --get key1 --id 1 --verbose
./build/client --put key1 --val value2 --id 1 --verbose
./build/client --put key3 --val value3 --id 1 --verbose
./build/client --put key5 --val value4 --id 1 --verbose
./build/client --get key1 --id 1 --verbose
./build/client --get key3 --id 1 --verbose
./build/client --get key5 --id 1 --verbose

# Clean up
./clean.sh
