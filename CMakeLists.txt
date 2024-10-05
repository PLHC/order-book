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

# Automatically include all .cpp and .h files from the market directory
file(GLOB CRQUEUE_FILES "${CMAKE_SOURCE_DIR}/market/CustomerRequestQueue/*")

# service definition files
file(GLOB SERVICE_FILES "${CMAKE_SOURCE_DIR}/server_and_client_grpc/Service/*")

# client  files
file(GLOB CLIENT_FILES "${CMAKE_SOURCE_DIR}/server_and_client_grpc/Client/*")

# random client files
file(GLOB RANDOM_CLIENT_FILES "${CMAKE_SOURCE_DIR}/server_and_client_grpc/Client/RandomizerClient/*")

# display client files
file(GLOB DISPLAY_CLIENT_FILES "${CMAKE_SOURCE_DIR}/server_and_client_grpc/Client/DisplayClient/*")

# Add executable and link libraries
add_executable(trading_platform
        main.cpp
        ${MARKET_FILES}
        ${CRQUEUE_FILES}
        ${SERVICE_FILES}
)

# Link executable with myproto, PRIVATE because OrderBook is not linked afterwards
target_link_libraries(trading_platform
        PUBLIC
        myproto
)

# Add executable for random client
add_executable(random_order_generator_client
        randomOrderGeneratorClient.cpp
        ${SERVICE_FILES}
        ${MARKET_FILES}
        ${CRQUEUE_FILES}
        ${CLIENT_FILES}
        ${RANDOM_CLIENT_FILES}
)
target_link_libraries(random_order_generator_client
        myproto
)

# Add executable for display client
add_executable(display_client
        display_client.cpp
        ${SERVICE_FILES}
        ${MARKET_FILES}
        ${CRQUEUE_FILES}
        ${CLIENT_FILES}
        ${DISPLAY_CLIENT_FILES}
)
target_link_libraries(display_client
        myproto
)

add_custom_target(build_all
        DEPENDS trading_platform random_order_generator_client display_client
)