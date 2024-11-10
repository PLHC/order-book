cmake_minimum_required(VERSION 3.28)
project(OrderBook)

set(CMAKE_CXX_STANDARD 20 "${CMAKE_CXX_FLAGS} -pedantic-error -Walls  ")

# MongoDB C++ driver
find_package(mongocxx REQUIRED HINTS /opt/homebrew/opt/mongo-cxx-driver/include)
find_package(bsoncxx REQUIRED HINTS /opt/homebrew/opt/mongo-cxx-driver/include)

# gRPC required packages
find_package(OpenSSL REQUIRED HINTS /opt/homebrew/opt/openssl@3)
include_directories(${OPENSSL_INCLUDE_DIR})
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
        OpenSSL::SSL
        OpenSSL::Crypto
)
target_include_directories(myproto PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/proto
)

# Compile protobuf and grpc files in myproto target to cpp
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET myproto LANGUAGE cpp)
protobuf_generate(TARGET myproto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

# Market files
file(GLOB MARKET_FILES "${CMAKE_SOURCE_DIR}/market/*")
file(GLOB ORDER_FILES "${CMAKE_SOURCE_DIR}/market/order/*")
file(GLOB CRQUEUE_FILES "${CMAKE_SOURCE_DIR}/market/CustomerRequestQueue/*")

# server and clients
file(GLOB SERVICE_FILES "${CMAKE_SOURCE_DIR}/server_and_client_grpc/Service/*")
file(GLOB CLIENT_FILES "${CMAKE_SOURCE_DIR}/server_and_client_grpc/Client/*")
file(GLOB RANDOM_CLIENT_FILES "${CMAKE_SOURCE_DIR}/server_and_client_grpc/Client/RandomizerClient/*")
file(GLOB DISPLAY_CLIENT_FILES "${CMAKE_SOURCE_DIR}/server_and_client_grpc/Client/DisplayClient/*")

# database interface
file(GLOB DATABASE "${CMAKE_SOURCE_DIR}/database/*")
file(GLOB LFQ_FILES "${CMAKE_SOURCE_DIR}/lock_free_queue/*")

# Add executable and link libraries
add_executable(trading_platform
        main.cpp
        ${MARKET_FILES}
        ${ORDER_FILES}
        ${CRQUEUE_FILES}
        ${SERVICE_FILES}
        ${ORDER_FILES}
        ${DATABASE}
        ${LFQ_FILES}
)

# Link executable, PRIVATE because trading_platform is not linked afterwards
target_link_libraries(trading_platform PRIVATE
        myproto
        mongo::mongocxx_shared
        mongo::bsoncxx_shared
)

# Add executable for random client
add_executable(random_order_generator_client
        random_order_generator_client.cpp
        ${SERVICE_FILES}
        ${MARKET_FILES}
        ${ORDER_FILES}
        ${CRQUEUE_FILES}
        ${CLIENT_FILES}
        ${RANDOM_CLIENT_FILES}
        ${DATABASE}
        ${LFQ_FILES}
)
target_link_libraries(random_order_generator_client PRIVATE
        myproto
        mongo::mongocxx_shared
        mongo::bsoncxx_shared
)

# Add executable for display client
add_executable(display_client
        display_client.cpp
        ${SERVICE_FILES}
        ${MARKET_FILES}
        ${ORDER_FILES}
        ${CRQUEUE_FILES}
        ${CLIENT_FILES}
        ${DISPLAY_CLIENT_FILES}
        ${DATABASE}
        ${LFQ_FILES}
)
target_link_libraries(display_client PRIVATE
        myproto
        mongo::mongocxx_shared
        mongo::bsoncxx_shared
)

add_custom_target(build_all DEPENDS
        trading_platform
        random_order_generator_client
        display_client
)