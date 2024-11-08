#ifndef ORDERBOOK_CLIENTASYNC_H
#define ORDERBOOK_CLIENTASYNC_H

#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>
#include <grpcpp/alarm.h>
#include <grpcpp/impl/codegen/sync_stream.h>
#include <thread>
#include <mutex>
#include <string>

#include "proto/MarketAccess.grpc.pb.h"
#include "../../market/order/Order.h"


class ClientAsync {
    // ClientAsync internal request ID
    std::mutex internalIdLock_;
    int64_t clientInternalId_;
    // Thread for processing the completion queue
    std::thread cq_thread_;
    // Flag to indicate if the client is shutting down
    std::atomic<bool> is_shutting_down_;

    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    std::vector<std::thread> threadPool_;

protected:
    // Stub for the Communication
    std::unique_ptr<marketAccess::Communication::Stub> stub_;
    // gRPC CompletionQueue to handle asynchronous operations
    grpc::CompletionQueue cq_;

public:
    ClientAsync(const std::shared_ptr<grpc::Channel>& channel, const uint32_t nbOfThreadsInThreadPool);
    ~ClientAsync();

    // Methods to generate async communication
    void generateDisplayRequestAsync(std::string&& orderBookName,
                                     uint32_t nbOfOrdersToDisplay);

    void generateInsertionRequestAsync(std::string&& orderBookName,
                                       std::string userID,
                                       double price,
                                       double volume,
                                       orderDirection buyOrSell,
                                       orderType boType);

    void generateUpdateRequestAsync(std::string&& orderBookName,
                                    std::string userID,
                                    uint64_t updatedBO,
                                    double price,
                                    double volume,
                                    orderDirection buyOrSell,
                                    orderType boType);

    void generateDeleteRequestAsync(std::string&& orderBookName,
                                    std::string userID,
                                    uint64_t deletedID);


protected:
    // Internal structure to store data for each RPC call
    // Base class
    class RequestDataBase{
    public:
        virtual ~RequestDataBase() = default;
        virtual void process() = 0; // pure virtual
    };

    template<typename ResponseParametersType>
    class RequestData : public RequestDataBase{
        grpc::ClientContext* context_;
        ResponseParametersType* responseParams_;
        grpc::Status* status_;
        ClientAsync& nestingClient_;

    public:
        RequestData(grpc::ClientContext* ctx,
                    ResponseParametersType* responseParams,
                    grpc::Status* status,
                    ClientAsync& client);
        ~RequestData() override;

        void process() override;
        void dispatchResponse(){ nestingClient_.handleResponse(responseParams_); }
    };

    // Methods
    [[nodiscard]] int64_t nextInternalID();
    // Method to process the completion queue for finished RPC calls
    void AsyncCompleteRpc();
    // Methods to override to the needs of the client class
    // need to be 4 functions instead of using a template because templated functions cannot be virtual
    virtual void handleResponse(marketAccess::OrderBookContent *responseParams); // non const for DisplayClient
    virtual void handleResponse(const marketAccess::InsertionConfirmation* responseParams) = 0;
    virtual void handleResponse(const marketAccess::UpdateConfirmation* responseParams) = 0;
    virtual void handleResponse(const marketAccess::DeletionConfirmation* responseParams) = 0;
};
#endif //ORDERBOOK_CLIENTASYNC_H