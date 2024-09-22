#ifndef ORDERBOOK_CLIENTASYNC_H
#define ORDERBOOK_CLIENTASYNC_H

#include <grpcpp/grpcpp.h>
#include <grpcpp/alarm.h>
#include <grpcpp/impl/codegen/sync_stream.h>
#include "proto/MarketAccess.grpc.pb.h"
#include "../../market/Order.h"

#include <thread>
#include <mutex>
#include <string>

class Client {
public:
    // Constructor accepting a gRPC channel
    explicit Client(const std::shared_ptr<grpc::Channel>& channel);

    // Destructor to handle proper shutdown of resources
    ~Client();

    // Method to handle async communication with the HandleTypeB RPC
    std::string HandleDisplayRequestAsync(std::string&& message,
                                   std::string&& orderBookName);
    std::pair<bool, u_int64_t> HandleInsertionRequestAsync(std::string&& orderBookName,
                                                           int userID,
                                                           double price,
                                                           double volume,
                                                           orderDirection buyOrSell,
                                                           orderType boType);

private:
    // Internal structure to store data for each RPC call
    class RpcDataBase{
    public:
        virtual ~RpcDataBase() = default;
        virtual void process() = 0;
    };

    template<typename RequestResponseType>
    class RpcData : public RpcDataBase{
        grpc::ClientContext* context_;
        RequestResponseType* response_;
        grpc::Status* status_;

    public:
        RpcData(grpc::ClientContext* c, RequestResponseType* r, grpc::Status* s);
        ~RpcData() override;

        void process() override;
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
