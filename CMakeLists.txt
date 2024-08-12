cmake_minimum_required(VERSION 3.28)
project(OrderBook)

set(CMAKE_CXX_STANDARD 17)

find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Automatically include all .proto files
file(GLOB PROTO_FILES "${CMAKE_SOURCE_DIR}/proto/*.proto")

# Add Library target with protobuf sources
add_library(myproto ${PROTO_FILES})
target_link_libraries(myproto
        PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(myproto PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/proto)

# Compile protobuf and grpc files in myproto target to cpp
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET myproto LANGUAGE cpp)
protobuf_generate(TARGET myproto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

# Automatically include all .cpp and .h files from the market directory
file(GLOB MARKET_FILES "${CMAKE_SOURCE_DIR}/market/*")

# Add executable and link libraries
add_executable(OrderBook
        main.cpp
        ${MARKET_FILES}
)

# Link executable with myproto, PRIVATE beacuse OrderBook is not linked afterwards
target_link_libraries(OrderBook
        PRIVATE
        myproto
)