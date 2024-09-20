#ifndef ORDERBOOK_CLIENTASYNC_H
#define ORDERBOOK_CLIENTASYNC_H

#include <grpcpp/grpcpp.h>
#include <grpcpp/alarm.h>
#include <grpcpp/impl/codegen/sync_stream.h>
#include "proto/MarketAccess.grpc.pb.h"
#include <thread>
#include <mutex>

class Client {
public:
    // Constructor accepting a gRPC channel
    Client(std::shared_ptr<grpc::Channel> channel);

    // Destructor to handle proper shutdown of resources
    ~Client();

    // Method to handle async communication with the HandleTypeB RPC
    void HandleDisplayRequestAsync(std::string&& message, std::string&& orderBookName);

private:
    // Internal structure to store data for each RPC call
    struct RpcData {
        grpc::ClientContext* context;
        marketAccess::OrderBookContent* response;
        grpc::Status* status;
    };

    // Method to process the completion queue for finished RPC calls
    void AsyncCompleteRpc();
    // Stub for the Communication
    std::unique_ptr<marketAccess::Communication::Stub> stub_;
    // gRPC CompletionQueue to handle asynchronous operations
    grpc::CompletionQueue cq_;
    // Thread for processing the completion queue
    std::thread cq_thread_;
    // Flag to indicate if the client is shutting down
    bool is_shutting_down_;
};

#endif //ORDERBOOK_CLIENTASYNC_H
