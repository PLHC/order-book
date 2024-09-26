#include "ServiceAsync.h"


RpcServiceAsync::RpcServiceAsync(grpc::ServerCompletionQueue *main_cq, Market *market)
        : main_cq_(main_cq),
          orderBookMap_( &(market->productToOrderBookMap_) ) {}

void RpcServiceAsync::handleRpcs() {
    // Start listening for all RPC types asynchronously
    new DisplayRequestHandler(&marketAccess::Communication::AsyncService::RequestDisplay,
                              this, main_cq_, orderBookMap_);
    new DeleteRequestHandler(&marketAccess::Communication::AsyncService::RequestDelete,
                             this, main_cq_, orderBookMap_);
    new InsertionRequestHandler(&marketAccess::Communication::AsyncService::RequestInsertion,
                                this, main_cq_, orderBookMap_);
    new UpdateRequestHandler(&marketAccess::Communication::AsyncService::RequestUpdate,
                             this, main_cq_, orderBookMap_);
    void *tag;
    bool ok;
    while (true) {
        GPR_ASSERT(main_cq_->Next(&tag, &ok));
        static_cast<RequestHandlerBase *>(tag)->proceed();
    }
}

// Templated class to handle various request types
template<typename RequestParametersType, typename ResponseParametersType>
RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::RequestHandler(
        RpcMethod rpcMethod,
        marketAccess::Communication::AsyncService *service,
        grpc::ServerCompletionQueue *cq,
        std::unordered_map<std::string, OrderBook*> *orderBookMap)
        : service_(service),
          cq_(cq),
          orderBookMap_(orderBookMap),
          responder_(&ctx_),
          status_(CREATE),
          rpcMethod_(rpcMethod),
          requestNodeInCRQ_(){
    proceed();
}

template<typename RequestParametersType, typename ResponseParametersType>
void RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::proceed() {

    if (status_ == CREATE) {
        status_ = PROCESS;
        (service_->*rpcMethod_)(&ctx_, &requestParameters_, &responder_, cq_, cq_, this); // Register to receive next request

    } else if (status_ == PROCESS) {
        generateNewRequestHandler();
        // Inspect metadata to decide on dispatch,
        // corrupted separations of metadata requires resizing based on size
        auto productIter = ctx_.client_metadata().find("product_id");
        std::string orderBookName = (productIter != end(ctx_.client_metadata() ) ) ?
                        std::string(productIter->second.data() ).substr(0, productIter->second.length() ) :
                        "";
        std::cout<<orderBookName<<std::endl;
        if (!orderBookName.empty() && (*orderBookMap_).count(orderBookName) ) {
            insertNodeInCRQAndHandleRequest(orderBookName);
        } else {
            handleProductError();
        }
        responder_.Finish(responseParameters_, grpc::Status::OK, this);
        status_ = FINISH;

    } else if (status_ == FINISH) {
        GPR_ASSERT(status_ == FINISH);
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
        return requestNodeInCRQ_->status_ == PROCESSING_ALLOWED;});

    handleValidRequest(orderBook);

    requestNodeInCRQ_->status_=PROCESSING_COMPLETED;
    statusLock.unlock();
    requestNodeInCRQ_->statusConditionVariable_.notify_all();
}

// Handle Product error
template<typename RequestParametersType, typename ResponseParametersType>
void RpcServiceAsync::RequestHandler<RequestParametersType, ResponseParametersType>::handleProductError() {
    responseParameters_.set_comment("Product is not available for trading");
    responseParameters_.set_validation(false);
}

// Handle valid requests
void RpcServiceAsync::DisplayRequestHandler::handleValidRequest(OrderBook* orderBook) {
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_orderbook(orderBook->displayOrderBook());
    responseParameters_.set_comment(("Display Request has been handled"));
    responseParameters_.set_validation(true);
}

void RpcServiceAsync::DeleteRequestHandler::handleValidRequest(OrderBook* orderBook) {
    orderBook->deletion(orderBook->getterPointerToOrderFromID(requestParameters_.boid()));
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_validation(true);
}

void RpcServiceAsync::InsertionRequestHandler::handleValidRequest(OrderBook* orderBook) {
    auto newGeneratedId = orderBook->genId_->nextID();
    orderBook->insertion(new Order(requestParameters_.userid(),
                                   newGeneratedId,
                                   requestParameters_.price(),
                                   requestParameters_.volume(),
                                   orderBook->getterProductID(),
                                   static_cast<orderDirection>(requestParameters_.buyorsell()),
                                   static_cast<orderType>( requestParameters_.botype() ) ) );
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_validation(true);
    responseParameters_.set_boid(newGeneratedId);
}

void RpcServiceAsync::UpdateRequestHandler::handleValidRequest(OrderBook* orderBook) {
    auto newGeneratedID = orderBook->genId_->nextID();
    orderBook->update(orderBook->getterPointerToOrderFromID(requestParameters_.boid()),
                            new Order(requestParameters_.userid(),
                                      newGeneratedID,
                                      requestParameters_.price(),
                                      requestParameters_.volume(),
                                      orderBook->getterProductID(),
                                      static_cast<orderDirection>(requestParameters_.buyorsell()),
                                      static_cast<orderType>( requestParameters_.botype() ) ) );
    responseParameters_.set_info(std::to_string(requestParameters_.info()));
    responseParameters_.set_validation(true);
    responseParameters_.set_boid(newGeneratedID);
}

// Generate new request handlers
void RpcServiceAsync::DisplayRequestHandler::generateNewRequestHandler() {
    new DisplayRequestHandler(rpcMethod_, service_, cq_, orderBookMap_);
}

void RpcServiceAsync::DeleteRequestHandler::generateNewRequestHandler() {
    new DeleteRequestHandler(rpcMethod_, service_, cq_, orderBookMap_);
}

void RpcServiceAsync::InsertionRequestHandler::generateNewRequestHandler() {
    new InsertionRequestHandler(rpcMethod_, service_, cq_, orderBookMap_);
}

void RpcServiceAsync::UpdateRequestHandler::generateNewRequestHandler() {
    new UpdateRequestHandler(rpcMethod_, service_, cq_, orderBookMap_);
}

