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

    void *tag;
    bool ok;
    while (true) {
        GPR_ASSERT(main_cq_->Next(&tag, &ok));
        static_cast<CallDataBase *>(tag)->Proceed();
    }
}

// Templated class to handle various request/response types
template<typename RequestParametersType, typename ResponseParametersType>
MyServiceImpl::CallData<RequestParametersType, ResponseParametersType>::CallData(RequestRpcMethod request_method, marketAccess::Communication::AsyncService *service,
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
        (service_->*request_method_)(&ctx_, &request_, &responder_, cq_, cq_, this); // Register for the request

    } else if (status_ == PROCESS) {
        // Inspect metadata to decide on dispatch, corrupted separations of metadata requires resizing based on size
        auto product = ctx_.client_metadata().find("product_id");
        std::string OBname = ( product != end( ctx_.client_metadata() ) ) ?
                             std::string( product->second.data() ).substr( 0, product->second.length() ) : "";

        if ( (*orderBookMap_).count(OBname) ) {
            generateNewCallData();
            HandleValidRequest();

        } else {
            generateNewCallData();
            HandleProductError();
        }
        status_ = FINISH;

    } else if (status_ == FINISH) {
        delete this;  // Cleanup after finishing
    }
}

template<typename RequestParametersType, typename ResponseParametersType>
void MyServiceImpl::CallData<RequestParametersType, ResponseParametersType>::HandleProductError() {
//    response_.set_orderBook("Product is not available for trading");
    responder_.Finish(response_, grpc::Status::OK, this);
}

// Display request
void MyServiceImpl::CallDataDisplayRequest::HandleValidRequest() {
    std::cout << "Processing the Display Request" << std::endl;
    response_.set_orderbook(("Display Request has been handled"));
    response_.set_validation(true);
    responder_.Finish(response_, grpc::Status::OK, this);
}

void MyServiceImpl::CallDataDisplayRequest::generateNewCallData() {
    response_.set_orderbook(("unknown order book"));
    response_.set_validation(false);
    new CallDataDisplayRequest(request_method_, service_, cq_,
                               orderBookMap_, orderBookVector_);
}

// Delete request
void MyServiceImpl::CallDataDeleteRequest::HandleValidRequest() {
    std::cout << "Processing the Delete Request" << std::endl;
    responder_.Finish(response_, grpc::Status::OK, this);
}

void MyServiceImpl::CallDataDeleteRequest::generateNewCallData() {
    new CallDataDeleteRequest(request_method_, service_, cq_,
                              orderBookMap_, orderBookVector_);
}

// Insertion request
void MyServiceImpl::CallDataInsertionRequest::HandleValidRequest() {
    std::cout << "Processing the Insertion Request" << std::endl;
    responder_.Finish(response_, grpc::Status::OK, this);
}

void MyServiceImpl::CallDataInsertionRequest::generateNewCallData() {
    new CallDataInsertionRequest(request_method_, service_, cq_,
                                 orderBookMap_, orderBookVector_);
}

// Update request
void MyServiceImpl::CallDataUpdateRequest::HandleValidRequest() {
    std::cout << "Processing the Update Request" << std::endl;
    responder_.Finish(response_, grpc::Status::OK, this);
}

void MyServiceImpl::CallDataUpdateRequest::generateNewCallData() {
    new CallDataUpdateRequest(request_method_, service_, cq_,
                              orderBookMap_, orderBookVector_);
}

