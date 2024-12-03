#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}Running All Tests...${NC}"

# Run single server test
./tests/single_server_test.sh

# Run multi-server test
./tests/multi_server_test.sh

# Run single node failure test
./tests/single_node_failure_test.sh

# Run multi node failure test
./tests/multi_node_failure_test.sh

echo -e "${GREEN}All tests completed!${NC}"
