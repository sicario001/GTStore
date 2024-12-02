cmake_minimum_required(VERSION 3.10)
project(GTStore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Generate protobuf and gRPC files
set(PROTO_FILES proto/gtstore.proto)
get_filename_component(PROTO_FILE_ABS ${PROTO_FILES} ABSOLUTE)
get_filename_component(PROTO_PATH "${PROTO_FILE_ABS}" PATH)

set(GENERATED_PROTO_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GENERATED_PROTO_DIR})

add_custom_command(
    OUTPUT "${GENERATED_PROTO_DIR}/gtstore.pb.cc"
           "${GENERATED_PROTO_DIR}/gtstore.pb.h"
           "${GENERATED_PROTO_DIR}/gtstore.grpc.pb.cc"
           "${GENERATED_PROTO_DIR}/gtstore.grpc.pb.h"
    COMMAND protobuf::protoc
    ARGS --grpc_out "${GENERATED_PROTO_DIR}"
         --cpp_out "${GENERATED_PROTO_DIR}"
         -I "${PROTO_PATH}"
         --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin
         "${PROTO_FILE_ABS}"
    DEPENDS ${PROTO_FILES}
)

# Create library for generated files
add_library(gtstore_proto
    ${GENERATED_PROTO_DIR}/gtstore.pb.cc
    ${GENERATED_PROTO_DIR}/gtstore.grpc.pb.cc
)

target_link_libraries(gtstore_proto
    gRPC::grpc++
    protobuf::libprotobuf
)

target_include_directories(gtstore_proto PUBLIC
    ${GENERATED_PROTO_DIR}
)

# Client library
add_library(gtstore_client
    src/client.cpp
)

target_link_libraries(gtstore_client
    gtstore_proto
)

# Manager executable
add_executable(manager
    src/manager.cpp
)

target_link_libraries(manager
    gtstore_proto
)

# Storage executable
add_executable(storage
    src/storage.cpp
)

target_link_libraries(storage
    gtstore_proto
)

# Test application
add_executable(test_app
    src/test_app.cpp
)

target_link_libraries(test_app
    gtstore_client
)
