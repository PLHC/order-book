#include "ServiceAsync.h"


RpcServiceAsync::RpcServiceAsync(grpc::ServerCompletionQueue *main_cq, Market *market, std::atomic<bool> *stopFlag)
        : main_cq_(main_cq),
          orderBookMap_( &(market->productToOrderBookMap_) ),
          stopFlag_(stopFlag) {}

void RpcServiceAsync::handleRpcs() {
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
    while (main_cq_->Next(&tag, &ok)) {
        static_cast<RequestHandlerBase*>(tag)->proceed();
    }
}







