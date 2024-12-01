# Args: nodes, replicas
nodes=$1
replicas=$2

# Launch the GTStore Manager
./bin/manager $nodes $replicas &
sleep 5

# Launch <nodes> storage nodes
for id in $(seq 1 $nodes)
do
    ./bin/storage $id &
done