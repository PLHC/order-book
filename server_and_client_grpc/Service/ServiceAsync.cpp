#include "ServiceAsync.h"


RpcServiceAsync::RpcServiceAsync(grpc::ServerCompletionQueue *main_cq, Market *market)
        : main_cq_(main_cq),
          orderBookMap_( &(market->productToOrderBookMap_) ),
          stopFlag_(&(market->stopFlag_) ){}

void RpcServiceAsync::handleRpcs() {
    // Start listening for all RPC types asynchronously
    new DisplayRequestHandler(&marketAccess::Communication::AsyncService::RequestDisplay,
                              this, main_cq_, orderBookMap_, stopFlag_);
    new DeleteRequestHandler(&marketAccess::Communication::AsyncService::RequestDelete,
                             this, main_cq_, orderBookMap_, stopFlag_);
    new InsertionRequestHandler(&marketAccess::Communication::AsyncService::RequestInsertion,
                                this, main_cq_, orderBookMap_, stopFlag_);
    new UpdateRequestHandler(&marketAccess::Communication::AsyncService::RequestUpdate,
                             this, main_cq_, orderBookMap_, stopFlag_);
    void *tag;
    bool ok;
    while (main_cq_->Next(&tag, &ok)) {
        static_cast<RequestHandlerBase *>(tag)->proceed();
    }
}

// Templated class to handle various request types
template<typename RequestParametersType, typename ResponseParametersType>
RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::RequestHandler(
        RpcMethod rpcMethod,
        marketAccess::Communication::AsyncService *service,
        grpc::ServerCompletionQueue *cq,
        std::unordered_map<std::string,
        OrderBook*> *orderBookMap,
        std::atomic<bool> *stopFlag)
        : service_(service),
          cq_(cq),
          orderBookMap_(orderBookMap),
          responder_(&ctx_),
          status_(CREATE),
          rpcMethod_(rpcMethod),
          requestNodeInCRQ_(),
          stopFlag_(stopFlag){
    proceed();
}

template<typename RequestParametersType, typename ResponseParametersType>
void RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::proceed() {
    if (status_ == CREATE) {
        status_ = PROCESS;
        (service_->*rpcMethod_)(&ctx_, &requestParameters_, &responder_, cq_, cq_, this); // Register to receive next request

    } else if (status_ == PROCESS) {
        if(!*stopFlag_) { // process only if stop not engaged
            generateNewRequestHandler();
        }
        // Inspect metadata to decide on dispatch,
        // corrupted separations of metadata requires resizing based on size
        auto productIter = ctx_.client_metadata().find("product_id");
        std::string orderBookName = (productIter != end(ctx_.client_metadata())) ?
                std::string(productIter->second.data()).substr(0, productIter->second.length()) : "";

        if (!*stopFlag_ && !orderBookName.empty() && (*orderBookMap_).count(orderBookName)) {
            insertNodeInCRQAndHandleRequest(orderBookName);
        } else {
            handleProductError();
        }

        if(!orderBookName.empty()) { // problem with empty request when shutting down server
            responder_.Finish(responseParameters_, grpc::Status::OK, this);
        }
        status_ = FINISH;

    } else if (status_ == FINISH) {
//        GPR_ASSERT(status_ == FINISH);
        delete this;
    }
}

template<typename RequestParametersType, typename ResponseParametersType>
void RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::
                        insertNodeInCRQAndHandleRequest(std::string &orderBookName) {
    auto orderBook = (*orderBookMap_)[orderBookName];
    requestNodeInCRQ_ = orderBook->requestQueue_.insertNode();
    std::unique_lock<std::mutex> statusLock(requestNodeInCRQ_->statusMutex_);
    requestNodeInCRQ_->statusConditionVariable_.wait(statusLock, [this](){
        return requestNodeInCRQ_->status_ == PROCESSING_ALLOWED || requestNodeInCRQ_->status_ == CANCELLED;});

    if(requestNodeInCRQ_->status_!= CANCELLED) {// if destructor started, skip the processing and trigger FINISH process
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
void RpcServiceAsync::DisplayRequestHandler::handleValidRequest(OrderBook* orderBook) {
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_orderbook(orderBook->displayOrderBook());
    responseParameters_.set_product(orderBook->getterProductID());
    responseParameters_.set_validation(true);
}

void RpcServiceAsync::DeleteRequestHandler::handleValidRequest(OrderBook* orderBook) {

    auto orderPtr = orderBook->getterPointerToOrderFromID(requestParameters_.boid());
    if(orderPtr) { // in case already deleted by a trade, no need to delete
        orderBook->deletion(orderPtr);
    }
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_validation(true);
    responseParameters_.set_product(orderBook->getterProductID());
}

void RpcServiceAsync::InsertionRequestHandler::handleValidRequest(OrderBook* orderBook) {
    auto newGeneratedId = orderBook->genId_->nextID();
    auto newOrder = new Order(requestParameters_.userid(),
                              newGeneratedId,
                              requestParameters_.price(),
                              requestParameters_.volume(),
                              orderBook->getterProductID(),
                              static_cast<orderDirection>(requestParameters_.buyorsell()),
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

void RpcServiceAsync::UpdateRequestHandler::handleValidRequest(OrderBook* orderBook) {
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_product(orderBook->getterProductID());

    auto newGeneratedID = orderBook->genId_->nextID();
    auto newOrder = new Order(requestParameters_.userid(),
                              newGeneratedID,
                              requestParameters_.price(),
                              requestParameters_.volume(),
                              orderBook->getterProductID(),
                              static_cast<orderDirection>(requestParameters_.buyorsell()),
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
void RpcServiceAsync::DisplayRequestHandler::generateNewRequestHandler() {
    new DisplayRequestHandler(rpcMethod_, service_, cq_, orderBookMap_, stopFlag_);
}

void RpcServiceAsync::DeleteRequestHandler::generateNewRequestHandler() {
    new DeleteRequestHandler(rpcMethod_, service_, cq_, orderBookMap_, stopFlag_);
}

void RpcServiceAsync::InsertionRequestHandler::generateNewRequestHandler() {
    new InsertionRequestHandler(rpcMethod_, service_, cq_, orderBookMap_, stopFlag_);
}

void RpcServiceAsync::UpdateRequestHandler::generateNewRequestHandler() {
    new UpdateRequestHandler(rpcMethod_, service_, cq_, orderBookMap_, stopFlag_);
}

