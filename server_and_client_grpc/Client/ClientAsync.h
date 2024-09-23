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

    // Methods to generate async communication
    void generateDisplayRequestAsync(std::string&& message,
                                     std::string&& orderBookName);
    void generateInsertionRequestAsync(std::string&& orderBookName,
                                       int userID,
                                       double price,
                                       double volume,
                                       orderDirection buyOrSell,
                                       orderType boType);

private:
    // Internal structure to store data for each RPC call
    // Base class
    class RequestDataBase{
    public:
        virtual ~RequestDataBase() = default;
        virtual void process() = 0;
    };

    //
    template<typename RequestResponseType>
    class RequestData : public RequestDataBase{
        grpc::ClientContext* context_;
        RequestResponseType* response_;
        grpc::Status* status_;
        Client& clientEnclosure_;

    public:
        RequestData(grpc::ClientContext* c, RequestResponseType* r, grpc::Status* s, Client& client);
        ~RequestData() override;

        void process() override;
        void handleResponse();
    };

    [[nodiscard]] u_int64_t nextInternalID();
    // Method to process the completion queue for finished RPC calls
    void AsyncCompleteRpc();

    // Client internal request ID
    std::mutex generatorLock_;
    std::condition_variable conditionVariableGeneratorLock_;
    std::uint64_t clientInternalId_;
    // map of currently executing orders and their corresponding content (type of request)
    std::unordered_map<std::uint64_t, std::string> internalIdToRequestTypeMap_;
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
