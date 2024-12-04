FROM ubuntu:20.04

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install basic build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    autoconf \
    libtool \
    curl \
    wget \
    unzip \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Install gRPC and Protocol Buffers
WORKDIR /tmp
RUN git clone --recurse-submodules -b v1.54.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc && \
    cd grpc && \
    mkdir -p cmake/build && \
    cd cmake/build && \
    cmake -DgRPC_INSTALL=ON \
          -DgRPC_BUILD_TESTS=OFF \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          ../.. && \
    make -j$(nproc) && \
    make install && \
    cd /tmp && \
    rm -rf grpc

# Set up working directory
WORKDIR /app

# Copy the project files
COPY . .

# Set environment variables
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV PATH=/usr/local/bin:$PATH

# Build command (to be run when container starts)
CMD ["bash"]

# Install Python dependencies
RUN pip3 install matplotlib
