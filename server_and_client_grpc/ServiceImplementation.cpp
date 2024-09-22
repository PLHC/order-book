#include "ServiceImplementation.h"


MyServiceImpl::MyServiceImpl(grpc::ServerCompletionQueue *main_cq, Market *market,
                             std::unordered_map<std::string, std::vector<std::string>> *orderBookVector)
        : main_cq_(main_cq),
          orderBookVector_(orderBookVector),
          orderBookMap_(&(market->ProductToOrderBookMap)) {}

void MyServiceImpl::HandleRpcs() {
    // Start listening for all RPC types asynchronously
    new CallDataDisplayRequest(
            &marketAccess::Communication::AsyncService::RequestDisplayRequest,
            this, main_cq_, orderBookMap_, orderBookVector_);
    new CallDataDeleteRequest(
            &marketAccess::Communication::AsyncService::RequestDeleteRequest,
            this, main_cq_, orderBookMap_, orderBookVector_);
    new CallDataInsertionRequest(
            &marketAccess::Communication::AsyncService::RequestInsertionRequest,
            this, main_cq_, orderBookMap_, orderBookVector_);
    new CallDataUpdateRequest(
            &marketAccess::Communication::AsyncService::RequestUpdateRequest,
            this, main_cq_, orderBookMap_, orderBookVector_);

    void *tag;
    bool ok;
    while (true) {
        GPR_ASSERT(main_cq_->Next(&tag, &ok));
        static_cast<CallDataBase *>(tag)->Proceed();
    }
}

// Templated class to handle various request/response_ types
template<typename RequestParametersType, typename ResponseParametersType>
MyServiceImpl::CallData<RequestParametersType, ResponseParametersType>::CallData(
        RequestRpcMethod request_method,
        marketAccess::Communication::AsyncService *service,
        grpc::ServerCompletionQueue *cq,
        std::unordered_map<std::string, OrderBook*> *orderBookMap,
        std::unordered_map<std::string, std::vector<std::string>> *orderBookVector)
        : service_(service), cq_(cq), orderBookMap_(orderBookMap),
          orderBookVector_(orderBookVector),
          responder_(&ctx_), status_(CREATE), request_method_(request_method) {
    Proceed();
}

template<typename RequestParametersType, typename ResponseParametersType>
void MyServiceImpl::CallData<RequestParametersType, ResponseParametersType>::Proceed() {

    if (status_ == CREATE) {
        status_ = PROCESS;
        (service_->*request_method_)(&ctx_, &request_, &responder_, cq_, cq_, this); // Register to receive next request

    } else if (status_ == PROCESS) {
        generateNewCallData();
        // Inspect metadata to decide on dispatch, corrupted separations of metadata requires resizing based on size
        auto product = ctx_.client_metadata().find("product_id");
        std::string orderBookName = (product != end(ctx_.client_metadata() ) ) ?
                                    std::string( product->second.data() ).substr( 0, product->second.length() ) :
                                    "";

        if (!orderBookName.empty() && (*orderBookMap_).count(orderBookName) ) {
//            std::cout<<"Processing proper request"<<std::endl;
            insertNodeInCRQAndHandleRequest(orderBookName);
        } else {
//            std::cout<<"Processing error"<<std::endl;
            handleProductError();
        }
        responder_.Finish(response_, grpc::Status::OK, this);
        status_ = FINISH;

    } else if (status_ == FINISH) {
        GPR_ASSERT(status_ == FINISH);
        delete this;
    }
}

template<typename RequestParametersType, typename ResponseParametersType>
void MyServiceImpl::CallData<RequestParametersType, ResponseParametersType>::insertNodeInCRQAndHandleRequest(
        std::string &OBname) {
    auto orderBook = (*orderBookMap_)[OBname];
    requestNodeInCRQ = orderBook->requestQueue_.insertNode();
    std::unique_lock<std::mutex> statusLock(requestNodeInCRQ->statusMutex_);
    requestNodeInCRQ->statusConditionVariable_.wait(statusLock, [this](){
        return requestNodeInCRQ->status_==PROCESSING_ALLOWED;});

    handleValidRequest(orderBook);

    requestNodeInCRQ->status_=PROCESSING_COMPLETED;
    statusLock.unlock();
    requestNodeInCRQ->statusConditionVariable_.notify_all();
}

// Handle Product error
template<typename RequestParametersType, typename ResponseParametersType>
void MyServiceImpl::CallData<RequestParametersType, ResponseParametersType>::handleProductError() {
    response_.set_comment("Product is not available for trading");
    response_.set_validation(false);
}

// Handle valid requests
void MyServiceImpl::CallDataDisplayRequest::handleValidRequest(OrderBook* orderBook) {
    std::cout << "Processing the Display Request" << std::endl;
    response_.set_orderbook(orderBook->displayOrderBook());
    response_.set_comment(("Display Request has been handled"));
    response_.set_validation(true);
}

void MyServiceImpl::CallDataDeleteRequest::handleValidRequest(OrderBook* orderBook) {
    std::cout << "Processing the Delete Request" << std::endl;
    orderBook->deletion(orderBook->getterPointerToOrderFromID(request_.boid()));
    response_.set_validation(true);
}

void MyServiceImpl::CallDataInsertionRequest::handleValidRequest(OrderBook* orderBook) {
    std::cout << "Processing the Insertion Request" << std::endl;
    auto newGeneratedID = orderBook->genID_->nextID();
    orderBook->insertion(new Order(request_.userid(),
                                            newGeneratedID,
                                            request_.price(),
                                            request_.volume(),
                                            orderBook->getterProductID(),
                                            static_cast<orderDirection>(request_.buyorsell()),
                                            static_cast<orderType>( request_.botype() ) ) );
    response_.set_validation(true);
    response_.set_boid(newGeneratedID);
}

void MyServiceImpl::CallDataUpdateRequest::handleValidRequest(OrderBook* orderBook) {
    std::cout << "Processing the Update Request" << std::endl;
    auto newGeneratedID = orderBook->genID_->nextID();
    orderBook->update(orderBook->getterPointerToOrderFromID(request_.boid()),
                            new Order(request_.userid(),
                                      newGeneratedID,
                                      request_.price(),
                                      request_.volume(),
                                      orderBook->getterProductID(),
                                      static_cast<orderDirection>(request_.buyorsell()),
                                      static_cast<orderType>( request_.botype() ) ) );
    response_.set_validation(true);
    response_.set_boid(newGeneratedID);
}

// Generate new request handlers
void MyServiceImpl::CallDataDisplayRequest::generateNewCallData() {
    new CallDataDisplayRequest(request_method_, service_, cq_,
                               orderBookMap_, orderBookVector_);
}

void MyServiceImpl::CallDataDeleteRequest::generateNewCallData() {
    new CallDataDeleteRequest(request_method_, service_, cq_,
                              orderBookMap_, orderBookVector_);
}

void MyServiceImpl::CallDataInsertionRequest::generateNewCallData() {
    new CallDataInsertionRequest(request_method_, service_, cq_,
                                 orderBookMap_, orderBookVector_);
}

void MyServiceImpl::CallDataUpdateRequest::generateNewCallData() {
    new CallDataUpdateRequest(request_method_, service_, cq_,
                              orderBookMap_, orderBookVector_);
}

