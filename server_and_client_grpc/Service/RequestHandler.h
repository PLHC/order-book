#ifndef ORDERBOOK_REQUESTHANDLER_H
#define ORDERBOOK_REQUESTHANDLER_H

#include <grpcpp/grpcpp.h>
#include "proto/MarketAccess.grpc.pb.h"
#include "../../market/Market.h"

// base class to handle common RPC logic
class RequestHandlerBase {

public:
    virtual ~RequestHandlerBase() = default;
    virtual void proceed() = 0;
};


// templated class to handle specific request/response types
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
                   std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> *orderBookMap,
                   std::atomic<bool> *stopFlag);

    ~RequestHandler() override = default;

    void proceed() override;

protected:
    void handleValidRequest(OrderBook* orderBook);
    void generateNewRequestHandler();
    void handleProductError();
    void insertNodeInCRQAndHandleRequest(std::string_view orderBookName);

    marketAccess::Communication::AsyncService *service_;
    grpc::ServerCompletionQueue *cq_;
    std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> *orderBookMap_;
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


#endif //ORDERBOOK_REQUESTHANDLER_H
