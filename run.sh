#/bin/bash

cmake -S gtstore/ -B build/ -DCMAKE_BUILD_TYPE=Debug
make -C build

# Args: nodes, replicas
nodes=$1
replicas=$2

# Start the service
./start_service.sh $nodes $replicas

sleep 3

# Launch the client testing app
# Usage: ./test_app <test> <client_id>
./build/test_app single_set_get 1
# ./build/test_app single_set_get 2 &
# ./build/test_app single_set_get 3 

