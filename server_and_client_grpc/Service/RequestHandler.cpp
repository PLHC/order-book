#include "RequestHandler.h"

template RequestHandler<marketAccess::UpdateParameters, marketAccess::UpdateConfirmation>::RequestHandler(
        RpcMethod rpcMethod,
        marketAccess::Communication::AsyncService *service,
        grpc::ServerCompletionQueue *cq,
        std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> *orderBookMap,
        std::atomic<bool> *stopFlag
);

template RequestHandler<marketAccess::DisplayParameters, marketAccess::OrderBookContent>::RequestHandler(
        RpcMethod rpcMethod,
        marketAccess::Communication::AsyncService *service,
        grpc::ServerCompletionQueue *cq,
        std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> *orderBookMap,
        std::atomic<bool> *stopFlag
);

template RequestHandler<marketAccess::DeletionParameters, marketAccess::DeletionConfirmation>::RequestHandler(
        RpcMethod rpcMethod,
        marketAccess::Communication::AsyncService *service,
        grpc::ServerCompletionQueue *cq,
        std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> *orderBookMap,
        std::atomic<bool> *stopFlag
);

template RequestHandler<marketAccess::InsertionParameters, marketAccess::InsertionConfirmation>::RequestHandler(
        RpcMethod rpcMethod,
        marketAccess::Communication::AsyncService *service,
        grpc::ServerCompletionQueue *cq,
        std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> *orderBookMap,
        std::atomic<bool> *stopFlag
);

// Templated class to handle various request types
template<typename RequestParametersType, typename ResponseParametersType>
RequestHandler<RequestParametersType, ResponseParametersType>::RequestHandler(
        RpcMethod rpcMethod,
        marketAccess::Communication::AsyncService *service,
        grpc::ServerCompletionQueue *cq,
        std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> *orderBookMap,
        std::atomic<bool> *stopFlag)
        : service_(service)
        , cq_(cq)
        , orderBookMap_(orderBookMap)
        , responder_(&ctx_)
        , status_(CREATE)
        , rpcMethod_(rpcMethod)
        , requestNodeInCRQ_()
        , stopFlag_(stopFlag){}

template<typename RequestParametersType, typename ResponseParametersType>
void RequestHandler<RequestParametersType, ResponseParametersType>::proceed() {
    if (status_ == CREATE) {
        status_ = PROCESS;
        (service_->*rpcMethod_)(&ctx_, &requestParameters_, &responder_, cq_, cq_, this); // Register to receive next request

    } else if (status_ == PROCESS) {
        if(!stopFlag_->load()) {
            generateNewRequestHandler();
        }
        // Inspect metadata to decide on dispatch,
        // corrupted separations of metadata requires resizing based on size when using string_view
        static const auto productID = grpc::string_ref("product_id"); // because string_ref does not have a constexpr constructor
        auto productIter = ctx_.client_metadata().find(productID);
        std::string_view orderBookName {productIter->second.data(), productIter->second.length()};
        if (!stopFlag_->load() && !orderBookName.empty() && (*orderBookMap_).contains(orderBookName)) {
            insertNodeInCRQAndHandleRequest(orderBookName);
        } else {
            handleProductError();
        }

        if(!orderBookName.empty()) { // problem with empty requests waiting for new requests, when shutting down server
            responder_.Finish(responseParameters_, grpc::Status::OK, this);
        }

        status_ = FINISH;

    } else if (status_ == FINISH) {
        delete this;
    }
}

template<typename RequestParametersType, typename ResponseParametersType>
void RequestHandler<RequestParametersType, ResponseParametersType>::
insertNodeInCRQAndHandleRequest(std::string_view orderBookName) {
    auto orderBook = (*orderBookMap_).find(orderBookName)->second; // operator [] to accept stringview when key is string only available from c++26 onwards
    requestNodeInCRQ_ = orderBook->requestQueue_.insertNode();
    std::unique_lock<std::mutex> statusLock(requestNodeInCRQ_->statusMutex_);
    requestNodeInCRQ_->statusConditionVariable_.wait(statusLock, [this](){
        return requestNodeInCRQ_->status_ == PROCESSING_ALLOWED || requestNodeInCRQ_->status_ == CANCELLED;
        });

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
void RequestHandler<RequestParametersType, ResponseParametersType>::handleProductError() {
    responseParameters_.set_info(       std::move( std::to_string( requestParameters_.info())));
    responseParameters_.set_comment(    "Product is not available for trading");
    responseParameters_.set_validation( false);
}

// Handle valid requests
template<>
void RequestHandler<marketAccess::DisplayParameters, marketAccess::OrderBookContent>::
handleValidRequest(OrderBook* orderBook) {
    responseParameters_.set_info(       std::move( std::to_string( requestParameters_.info() ) ) );
    // displayOrderBook returns a rvalue because the compiler optimizes it using RVO (Return Value Optimization)
    responseParameters_.set_orderbook(  std::move( orderBook->displayOrderBook( requestParameters_.nboforderstodisplay())));
    responseParameters_.set_product(    orderBook->getterProductID()); // productID is returned as a ref and copied in set_product
    responseParameters_.set_validation( true);
}

template<>
void RequestHandler<marketAccess::DeletionParameters, marketAccess::DeletionConfirmation>::
handleValidRequest(OrderBook* orderBook) {
    auto orderPtr = orderBook->getterPointerToOrderFromID(requestParameters_.boid());
    if(orderPtr) { // in case already deleted by a trade, no need to delete
        orderBook->deletion(orderPtr, COMMUNICATED);
    }
    responseParameters_.set_info( std::move(std::to_string( requestParameters_.info() ) ) );
    responseParameters_.set_validation(     true);
    responseParameters_.set_product(        orderBook->getterProductID()); // productID is returned as a ref and copied in set_product
}

template<>
void RequestHandler<marketAccess::InsertionParameters, marketAccess::InsertionConfirmation>::
handleValidRequest(OrderBook* orderBook) {
    auto newGeneratedId = orderBook->genId_->nextID();
    auto newOrder = new Order(static_cast<orderDirection>(requestParameters_.buyorsell()),
                              requestParameters_.userid(),
                              newGeneratedId,
                              requestParameters_.price(),
                              requestParameters_.volume(),
                              orderBook->getterProductID(), // productID is returned as a ref and copied in Order definition
                              static_cast<orderType>( requestParameters_.botype() ) );

    responseParameters_.set_validation( orderBook->insertion(newOrder, COMMUNICATED));
    responseParameters_.set_info(       std::move( std::to_string( requestParameters_.info())));
    responseParameters_.set_boid(       newGeneratedId);
    responseParameters_.set_price(      newOrder->getterPrice());
    responseParameters_.set_volume(     newOrder->getterVolume());
    responseParameters_.set_version(    newOrder->getterVersion());
    responseParameters_.set_product(    newOrder->getterProductID());

    if(!responseParameters_.validation() || newOrder->getterVolume()==0){
        delete newOrder;
    }
}

template<>
void RequestHandler<marketAccess::UpdateParameters, marketAccess::UpdateConfirmation>::
handleValidRequest(OrderBook* orderBook) {
    responseParameters_.set_info(   std::move( std::to_string( requestParameters_.info())));
    responseParameters_.set_product(orderBook->getterProductID()); // productID is returned as a ref and copied in set_product

    auto newOrder = new Order(static_cast<orderDirection>(requestParameters_.buyorsell()),
                              requestParameters_.userid(), // cannot be moved because type is const string &
                              requestParameters_.boid(), // removed : generating a new ID on updates
                              requestParameters_.price(),
                              requestParameters_.volume(),
                              orderBook->getterProductID(), // productID is returned as a ref and copied in Order definition
                              static_cast<orderType>( requestParameters_.botype() ),
                              requestParameters_.version());

    auto updatedOrder = orderBook->getterPointerToOrderFromID(requestParameters_.boid());
    responseParameters_.set_validation(updatedOrder && orderBook->update(updatedOrder, newOrder));

    if(responseParameters_.validation()) {
        responseParameters_.set_boid(   newOrder->getterBoID());
        responseParameters_.set_version(newOrder->getterVersion());
        responseParameters_.set_volume( newOrder->getterVolume());
        responseParameters_.set_price(  newOrder->getterPrice());
    }else if(updatedOrder){ // case: update has older version than current one
        responseParameters_.set_boid(   updatedOrder->getterBoID());
        responseParameters_.set_version(updatedOrder->getterVersion());
        responseParameters_.set_volume( updatedOrder->getterVolume());
        responseParameters_.set_price(  updatedOrder->getterPrice());
        delete newOrder;
    }else{ // case: updated Order has been deleted already by a trade
        responseParameters_.set_boid(   newOrder->getterBoID());
        responseParameters_.set_version(newOrder->getterVersion());
        responseParameters_.set_volume( 0);
        responseParameters_.set_price(  newOrder->getterPrice());
        delete newOrder;
    }
}

// Generate new request handlers
template<typename RequestParametersType, typename ResponseParametersType>
void RequestHandler<RequestParametersType, ResponseParametersType>::generateNewRequestHandler() {
    (new RequestHandler<RequestParametersType, ResponseParametersType>(
            rpcMethod_, service_, cq_, orderBookMap_, stopFlag_)
    )->proceed();
}
