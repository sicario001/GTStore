#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}Building GTStore...${NC}"

# Create build directory if it doesn't exist
mkdir -p build

# Build using CMake
cmake -S gtstore/ -B build/ -DCMAKE_BUILD_TYPE=Release
make -C build

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi