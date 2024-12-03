#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}Running Single Server Test...${NC}"

# Start service with 1 node and 1 replica
./start_service.sh 1 1

# Run test sequence
echo "Test 1: Basic Single Server GET/PUT"

./build/client --put key1 --val value1 --id 1 --verbose
./build/client --get key1 --id 1 --verbose
./build/client --put key1 --val value2 --id 1 --verbose
./build/client --put key2 --val value3 --id 1 --verbose
./build/client --put key3 --val value4 --id 1 --verbose
./build/client --get key1 --id 1 --verbose
./build/client --get key2 --id 1 --verbose
./build/client --get key3 --id 1 --verbose

# Clean up
./clean.sh
