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
    (new RpcServiceAsync::RequestHandler<marketAccess::DeletionParameters, marketAccess::DeletionConfirmation>(
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






// Templated class to handle various request types
template<typename RequestParametersType, typename ResponseParametersType>
RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::RequestHandler(
        RpcMethod rpcMethod,
        marketAccess::Communication::AsyncService *service,
        grpc::ServerCompletionQueue *cq,
        std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> *orderBookMap,
        std::atomic<bool> *stopFlag)
        : service_(service),
          cq_(cq),
          orderBookMap_(orderBookMap),
          responder_(&ctx_),
          status_(CREATE),
          rpcMethod_(rpcMethod),
          requestNodeInCRQ_(),
          stopFlag_(stopFlag){}

template<typename RequestParametersType, typename ResponseParametersType>
void RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::proceed() {
    if (status_ == CREATE) {
        status_ = PROCESS;
        (service_->*rpcMethod_)(&ctx_, &requestParameters_, &responder_, cq_, cq_, this); // Register to receive next request

    } else if (status_ == PROCESS) {
        if(!stopFlag_->load()) {
            generateNewRequestHandler();
        }
        // Inspect metadata to decide on dispatch,
        // corrupted separations of metadata requires resizing based on size
        auto productIter = ctx_.client_metadata().find("product_id");
        std::string_view orderBookName {productIter->second.data(), productIter->second.length()};

        if (!stopFlag_->load() && !orderBookName.empty() && (*orderBookMap_).contains(orderBookName)) {
            insertNodeInCRQAndHandleRequest(orderBookName);
        } else {
            handleProductError();
        }

        if(!orderBookName.empty()) { // problem with empty request when shutting down server
            responder_.Finish(responseParameters_, grpc::Status::OK, this);
        }

        status_ = FINISH;

    } else if (status_ == FINISH) {
        delete this;
    }
}

template<typename RequestParametersType, typename ResponseParametersType>
void RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::
                        insertNodeInCRQAndHandleRequest(const std::string_view orderBookName) {
    auto orderBook = (*orderBookMap_)[{orderBookName.data(), orderBookName.length()}];
    requestNodeInCRQ_ = orderBook->requestQueue_.insertNode();
    std::unique_lock<std::mutex> statusLock(requestNodeInCRQ_->statusMutex_);
    requestNodeInCRQ_->statusConditionVariable_.wait(statusLock, [this](){
        return requestNodeInCRQ_->status_ == PROCESSING_ALLOWED || requestNodeInCRQ_->status_ == CANCELLED;});

    // if destructor started, skip the processing and trigger FINISH process
    if(requestNodeInCRQ_->status_!= CANCELLED) {
        handleValidRequest(orderBook);
        requestNodeInCRQ_->status_=PROCESSING_COMPLETED;
    }

    statusLock.unlock();
    requestNodeInCRQ_->statusConditionVariable_.notify_all();
}

// Handle Product error
template<typename RequestParametersType, typename ResponseParametersType>
void RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::handleProductError() {
    responseParameters_.set_info( std::to_string( requestParameters_.info() ) );
    responseParameters_.set_comment("Product is not available for trading");
    responseParameters_.set_validation(false);
}

// Handle valid requests
template<>
void RpcServiceAsync::RequestHandler<marketAccess::DisplayParameters, marketAccess::OrderBookContent>::
        handleValidRequest(OrderBook* orderBook) {
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_orderbook(orderBook->displayOrderBook(requestParameters_.nboforderstodisplay()));
    responseParameters_.set_product(orderBook->getterProductID());
    responseParameters_.set_validation(true);
}

template<>
void RpcServiceAsync::RequestHandler<marketAccess::DeletionParameters, marketAccess::DeletionConfirmation>::
        handleValidRequest(OrderBook* orderBook) {
    auto orderPtr = orderBook->getterPointerToOrderFromID(requestParameters_.boid());
    if(orderPtr) { // in case already deleted by a trade, no need to delete
        orderBook->deletion(orderPtr);
    }
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_validation(true);
    responseParameters_.set_product(orderBook->getterProductID());
}

template<>
void RpcServiceAsync::RequestHandler<marketAccess::InsertionParameters, marketAccess::InsertionConfirmation>::
        handleValidRequest(OrderBook* orderBook) {
    auto newGeneratedId = orderBook->genId_->nextID();
    auto newOrder = new Order(static_cast<orderDirection>(requestParameters_.buyorsell()),
                              requestParameters_.userid(),
                              newGeneratedId,
                              requestParameters_.price(),
                              requestParameters_.volume(),
                              orderBook->getterProductID(),
                              static_cast<orderType>( requestParameters_.botype() ) );

    responseParameters_.set_validation(orderBook->insertion(newOrder));
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_boid(newGeneratedId);
    responseParameters_.set_price(newOrder->getterPrice());
    responseParameters_.set_volume(newOrder->getterVolume());
    responseParameters_.set_version(newOrder->getterVersion());
    responseParameters_.set_product(newOrder->getterProductID());

    if(!responseParameters_.validation() || newOrder->getterVolume()==0){
        delete newOrder;
    }
}

template<>
void RpcServiceAsync::RequestHandler<marketAccess::UpdateParameters, marketAccess::UpdateConfirmation>::
        handleValidRequest(OrderBook* orderBook) {
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_product(orderBook->getterProductID());

    auto newGeneratedID = orderBook->genId_->nextID();
    auto newOrder = new Order(static_cast<orderDirection>(requestParameters_.buyorsell()),
                              requestParameters_.userid(),
                              newGeneratedID,
                              requestParameters_.price(),
                              requestParameters_.volume(),
                              orderBook->getterProductID(),
                              static_cast<orderType>( requestParameters_.botype() ),
                              requestParameters_.version());

    auto updatedOrder = orderBook->getterPointerToOrderFromID(requestParameters_.boid());
    responseParameters_.set_validation(updatedOrder && orderBook->update(updatedOrder, newOrder));

    if(responseParameters_.validation()) {
        responseParameters_.set_boid(newOrder->getterBoID());
        responseParameters_.set_version(newOrder->getterVersion());
        responseParameters_.set_volume(newOrder->getterVolume());
        responseParameters_.set_price(newOrder->getterPrice());
    }else if(updatedOrder){ // case: update has older version than current one
        responseParameters_.set_boid(updatedOrder->getterBoID());
        responseParameters_.set_version(updatedOrder->getterVersion());
        responseParameters_.set_volume(updatedOrder->getterVolume());
        responseParameters_.set_price(updatedOrder->getterPrice());
        delete newOrder;
    }else{ // case: updated Order has been deleted already by a trade
        responseParameters_.set_boid(newOrder->getterBoID());
        responseParameters_.set_version(newOrder->getterVersion());
        responseParameters_.set_volume(0);
        responseParameters_.set_price(newOrder->getterPrice());
        delete newOrder;
    }
}

// Generate new request handlers
template<typename RequestParametersType, typename ResponseParametersType>
void RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::generateNewRequestHandler() {
    (new RequestHandler<RequestParametersType, ResponseParametersType>(
            rpcMethod_, service_, cq_, orderBookMap_, stopFlag_)
                )->proceed();
}

