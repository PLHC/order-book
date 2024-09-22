#ifndef ORDERBOOK_SERVICEIMPLEMENTATION_H
#define ORDERBOOK_SERVICEIMPLEMENTATION_H

#include <grpcpp/grpcpp.h>
#include "proto/MarketAccess.grpc.pb.h"
#include "../market/Market.h"
#include <thread>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>



// MyServiceImpl class derived from the generated AsyncService class
class MyServiceImpl final : public marketAccess::Communication::AsyncService {
    grpc::ServerCompletionQueue *main_cq_;
    std::unordered_map<std::string, OrderBook*> *orderBookMap_;
    std::unordered_map<std::string, std::vector<std::string>> *orderBookVector_;
public:
    // Constructor to initialize the server completion queue and mappings
    explicit MyServiceImpl(grpc::ServerCompletionQueue *main_cq,
                           Market *market,
                           std::unordered_map<std::string, std::vector<std::string>> *orderBookVector);

    // Method to handle incoming RPCs
    void HandleRpcs();

    // Base class to handle common RPC logic
    class CallDataBase {
    public:
        virtual ~CallDataBase() = default;
        virtual void Proceed() = 0;
    };

    // Templated class to handle specific request/response_ types
    template<typename RequestParametersType, typename ResponseParametersType>
    class CallData : public CallDataBase {
    public:
        using RequestRpcMethod = void (marketAccess::Communication::AsyncService::*)(
                grpc::ServerContext *, RequestParametersType *,
                grpc::ServerAsyncResponseWriter<ResponseParametersType> *,
                grpc::CompletionQueue *,
                grpc::ServerCompletionQueue *, void *);

        CallData(RequestRpcMethod request_method, marketAccess::Communication::AsyncService *service,
                 grpc::ServerCompletionQueue *cq,
                 std::unordered_map<std::string, OrderBook*> *orderBookMap,
                 std::unordered_map<std::string, std::vector<std::string>> *orderBookVector);

        void Proceed() override;

    protected:
        virtual void handleValidRequest(OrderBook* orderBook) = 0;
        virtual void generateNewCallData() = 0;
        void handleProductError();
        void insertNodeInCRQAndHandleRequest(std::string & OBname);

        marketAccess::Communication::AsyncService *service_;
        grpc::ServerCompletionQueue *cq_;
        std::unordered_map<std::string, OrderBook*> *orderBookMap_;
        std::unordered_map<std::string, std::vector<std::string>> *orderBookVector_;
        RequestNode *requestNodeInCRQ;

        grpc::ServerContext ctx_;
        RequestParametersType request_;
        ResponseParametersType response_;
        grpc::ServerAsyncResponseWriter<ResponseParametersType> responder_;

        enum CallStatus {
            CREATE, PROCESS, FINISH
        };
        CallStatus status_;
        RequestRpcMethod request_method_;
    };

    class CallDataDisplayRequest : public CallData<marketAccess::DisplayParameters, marketAccess::OrderBookContent>{
        using CallData<marketAccess::DisplayParameters, marketAccess::OrderBookContent>::CallData;
    protected:
        void handleValidRequest(OrderBook* orderBook) override;
        void generateNewCallData() override;
    };

    class CallDataDeleteRequest : public CallData<marketAccess::DeletionParameters, marketAccess::Confirmation>{
        using CallData<marketAccess::DeletionParameters, marketAccess::Confirmation>::CallData;
    protected:
        void handleValidRequest(OrderBook* orderBook) override;
        void generateNewCallData() override;
    };

    class CallDataInsertionRequest : public CallData<marketAccess::InsertionParameters, marketAccess::Confirmation>{
        using CallData<marketAccess::InsertionParameters, marketAccess::Confirmation>::CallData;
    protected:
        void handleValidRequest(OrderBook* orderBook) override;
        void generateNewCallData() override;
    };

    class CallDataUpdateRequest : public CallData<marketAccess::UpdateParameters, marketAccess::Confirmation>{
        using CallData<marketAccess::UpdateParameters, marketAccess::Confirmation>::CallData;
    protected:
        void handleValidRequest(OrderBook* orderBook) override;
        void generateNewCallData() override;
    };
};

#endif //ORDERBOOK_SERVICEIMPLEMENTATION_H
