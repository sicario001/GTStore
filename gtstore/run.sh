#/bin/bash

make clean
make

# Args: nodes, replicas
nodes=$1
replicas=$2

# Start the service
./start_service.sh $nodes $replicas

# Launch the client testing app
# Usage: ./test_app <test> <client_id>
./bin/test_app single_set_get 1 &
./bin/test_app single_set_get 2 &
./bin/test_app single_set_get 3 

