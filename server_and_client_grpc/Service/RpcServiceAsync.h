#ifndef ORDERBOOK_RPCSERVICEASYNC_H
#define ORDERBOOK_RPCSERVICEASYNC_H

#include <thread>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

#include "RequestHandler.h"


class RpcServiceAsync final : public marketAccess::Communication::AsyncService {
    std::atomic<bool> *stopFlag_;
    grpc::ServerCompletionQueue *main_cq_;
    std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> *orderBookMap_;

public:
    // constructor to initialize the server completion queue and mappings
    RpcServiceAsync(grpc::ServerCompletionQueue *main_cq, Market *market, std::atomic<bool> *stopFlag)
            : main_cq_{ main_cq }
            , orderBookMap_{ &( market->productToOrderBookMap_ ) }
            , stopFlag_{ stopFlag } {}

    // method to handle incoming RPCs
    void handleRpcs() {
        // Start listening for all RPC types asynchronously
        (new RequestHandler<marketAccess::DisplayParameters, marketAccess::OrderBookContent>(
                &marketAccess::Communication::AsyncService::RequestDisplay, this, main_cq_, orderBookMap_, stopFlag_)
                    )->proceed();
        (new RequestHandler<marketAccess::DeletionParameters, marketAccess::DeletionConfirmation>(
                &marketAccess::Communication::AsyncService::RequestDelete, this, main_cq_, orderBookMap_, stopFlag_)
                    )->proceed();
        (new RequestHandler<marketAccess::InsertionParameters, marketAccess::InsertionConfirmation>(
                &marketAccess::Communication::AsyncService::RequestInsertion, this, main_cq_, orderBookMap_, stopFlag_)
                    )->proceed();
        (new RequestHandler<marketAccess::UpdateParameters, marketAccess::UpdateConfirmation>(
                &marketAccess::Communication::AsyncService::RequestUpdate, this, main_cq_, orderBookMap_, stopFlag_)
                    )->proceed();
        void *tag;
        bool ok;
        while ( main_cq_->Next(&tag, &ok) ) {
            static_cast<RequestHandlerBase*>(tag)->proceed();
        }
    }
};

#endif //ORDERBOOK_RPCSERVICEASYNC_H
