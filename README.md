# GTStore - Distributed Key-Value Store

GTStore is a distributed key-value store system that provides scalability, availability, and resilience to temporary node failures. It uses gRPC for network communication and supports replication for fault tolerance.

## Environment Setup

You can set up the GTStore development environment either manually or using Docker.

### Manual Setup

#### Prerequisites

- CMake
- C++17 compatible compiler
- gRPC
- Protobuf

### Docker Setup (Recommended)

To set up GTStore using Docker, follow these steps:

#### Build the docker container:
```bash
docker build -t gtstore-dev .
```

This will:
- Build the development environment with all dependencies
- Mount your project directory to `/app` in the container

#### Run the container
```bash
# In a separate terminal
docker run -it \
  --name gtstore \
  -v "$(pwd)":/app \
  -v build_cache:/app/build \
  -w /app \
  gtstore-dev \
  bash
```

## Building the Project

To build the project, run:

```bash
./build.sh
```

This will create a `build` directory and compile all the necessary components:
- `manager`: Manager service
- `storage`: Storage node service
- `client`: Client application

## Running the System

1. Start the service with N storage nodes and R replicas:
```bash
./start_service.sh <num_nodes> <num_replicas>
```
Example: `./start_service.sh 3 2` starts the system with 3 storage nodes and 2 replicas

2. Use the client application:
```bash
# Put a key-value pair
./build/client --put <key> --val <value> [--id <client_id>] [--verbose]

# Get a value
./build/client --get <key> [--id <client_id>] [--verbose]
```

Examples:
```bash
# Put a key-value pair
./build/client --put key1 --val value1 --verbose

# Get a value
./build/client --get key1 --verbose
```

3. Clean up all processes:
```bash
./clean.sh
```

## Running Tests

The project includes several test scripts in the `tests` directory:

1. Single Server Test:
```bash
./tests/single_server_test.sh
```
Tests basic operations on a single server setup.

2. Multi-Server Test:
```bash
./tests/multi_server_test.sh
```
Tests operations with multiple storage nodes.

3. Single Node Failure Test:
```bash
./tests/single_node_failure_test.sh
```
Tests system behavior when one storage node fails.

4. Multi Node Failure Test:
```bash
./tests/multi_node_failure_test.sh
```
Tests system behavior when multiple storage nodes fail.

5. Run All Tests:
```bash
./tests/run_all_tests.sh
```
Runs all test scenarios in sequence.

## Client Options

```
Usage: client [options]
Options:
  --put <key>         Put a key
  --val <value>       Value for put operation (required with --put)
  --get <key>         Get a key
  --id <client_id>    Client ID (default: 1)
  --verbose           Enable verbose output
  --help              Show this help message
```

## Running Benchmarks

The project includes a benchmarks to evaluate system performance:

Use the following command to run all benchmarks:
```bash
./tests/benchmark_test.sh
```

Generates the following plots:
- single_client_throughput.png
- concurrent_throughput.png
- loadbalance.png

### Benchmark Types

1. Single Client Throughput Test:
```bash
./build/benchmark --throughput <replicas>
```
Tests the performance of a single client with a specified number of replicas.

2. Concurrent Throughput Test:
```bash
./build/benchmark --concurrent <replicas> <threads>
```
Tests the performance of multiple clients with a specified number of replicas and threads. Defaults to 8 threads.

3. Load Balance Test:
```bash
./build/benchmark --loadbalance
```
Tests the distribution of keys across nodes after a large number of insertions.

**You will need to start the service before running the individual benchmarks.**