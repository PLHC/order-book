#ifndef ORDERBOOK_SERVICEASYNC_H
#define ORDERBOOK_SERVICEASYNC_H

#include <grpcpp/grpcpp.h>
#include "proto/MarketAccess.grpc.pb.h"
#include "../../market/Market.h"
#include <thread>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>


// RpcServiceAsync class derived from the generated AsyncService class
class RpcServiceAsync final : public marketAccess::Communication::AsyncService {
    grpc::ServerCompletionQueue *main_cq_;
    std::unordered_map<std::string, OrderBook*> *orderBookMap_;
    std::atomic<bool> *stopFlag_;
public:
    // Constructor to initialize the server completion queue and mappings
    RpcServiceAsync(grpc::ServerCompletionQueue *main_cq, Market *market);

    // Method to handle incoming RPCs
    void handleRpcs();

    // Base class to handle common RPC logic
    class RequestHandlerBase {

    public:
        virtual ~RequestHandlerBase() = default;
        virtual void proceed() = 0;
    };

    // Templated class to handle specific request/response types
    template<typename RequestParametersType, typename ResponseParametersType>
    class RequestHandler : public RequestHandlerBase {
    public:
        using RpcMethod = void (marketAccess::Communication::AsyncService::*)(
                grpc::ServerContext *,
                RequestParametersType *,
                grpc::ServerAsyncResponseWriter<ResponseParametersType> *,
                grpc::CompletionQueue *,
                grpc::ServerCompletionQueue *,
                void *);

        RequestHandler(RpcMethod rpcMethod,
                       marketAccess::Communication::AsyncService *service,
                       grpc::ServerCompletionQueue *cq,
                       std::unordered_map<std::string, OrderBook*> *orderBookMap,
                       std::atomic<bool> *stopFlag);

        void proceed() override;

    protected:
        virtual void handleValidRequest(OrderBook* orderBook) = 0;
        virtual void generateNewRequestHandler() = 0;
        void handleProductError();
        void insertNodeInCRQAndHandleRequest(std::string & orderBookName);

        marketAccess::Communication::AsyncService *service_;
        grpc::ServerCompletionQueue *cq_;
        std::unordered_map<std::string, OrderBook*> *orderBookMap_;
        RequestNode *requestNodeInCRQ_;
        std::atomic<bool> *stopFlag_;

        enum RequestStatus { CREATE, PROCESS, FINISH };
        RequestStatus status_;
        RpcMethod rpcMethod_;
        grpc::ServerContext ctx_;
        RequestParametersType requestParameters_;
        ResponseParametersType responseParameters_;
        grpc::ServerAsyncResponseWriter<ResponseParametersType> responder_;
    };

    class DisplayRequestHandler : public RequestHandler<marketAccess::DisplayParameters, marketAccess::OrderBookContent>{
        using RequestHandler<marketAccess::DisplayParameters, marketAccess::OrderBookContent>::RequestHandler;
    protected:
        void handleValidRequest(OrderBook* orderBook) override;
        void generateNewRequestHandler() override;
    };

    class DeleteRequestHandler : public RequestHandler<marketAccess::DeletionParameters, marketAccess::DeletionConfirmation>{
        using RequestHandler<marketAccess::DeletionParameters, marketAccess::DeletionConfirmation>::RequestHandler;
    protected:
        void handleValidRequest(OrderBook* orderBook) override;
        void generateNewRequestHandler() override;
    };

    class InsertionRequestHandler : public RequestHandler<marketAccess::InsertionParameters, marketAccess::InsertionConfirmation>{
        using RequestHandler<marketAccess::InsertionParameters, marketAccess::InsertionConfirmation>::RequestHandler;
    protected:
        void handleValidRequest(OrderBook* orderBook) override;
        void generateNewRequestHandler() override;
    };

    class UpdateRequestHandler : public RequestHandler<marketAccess::UpdateParameters, marketAccess::UpdateConfirmation>{
        using RequestHandler<marketAccess::UpdateParameters, marketAccess::UpdateConfirmation>::RequestHandler;
    protected:
        void handleValidRequest(OrderBook* orderBook) override;
        void generateNewRequestHandler() override;
    };
};

#endif //ORDERBOOK_SERVICEASYNC_H
