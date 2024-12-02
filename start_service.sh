# Args: nodes, replicas
nodes=$1
replicas=$2

# Launch the GTStore Manager
./build/manager $nodes $replicas &
sleep 3

# Launch <nodes> storage nodes
for id in $(seq 1 $nodes)
do
    ./build/storage $id &
done