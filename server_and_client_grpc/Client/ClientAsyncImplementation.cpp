#include "ClientAsyncImplementation.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

Client::Client(const std::shared_ptr<Channel>& channel)
        : stub_(marketAccess::Communication::NewStub(channel)),
          clientInternalId_(0),
          generatorLock_(),
          conditionVariableGeneratorLock_(),
          internalIdToRequestTypeMap_(),
          is_shutting_down_(false) {
    // Start a separate thread to process the CompletionQueue
    cq_thread_ = std::thread([this]() { this->AsyncCompleteRpc(); });
}

Client::~Client() {
    // Shutdown the completion queue and wait for the thread to finish
    is_shutting_down_ = true;
    cq_.Shutdown();
    if (cq_thread_.joinable()) {
        cq_thread_.join();
    }
}

u_int64_t Client::nextInternalID() {
    std::unique_lock<std::mutex> genLock (generatorLock_);
    conditionVariableGeneratorLock_.wait(genLock, [](){ return true;});

    auto newID = ++clientInternalId_;

    genLock.unlock();
    conditionVariableGeneratorLock_.notify_all();

    return newID;
}

template<typename ResponseParametersType>
Client::RequestData<ResponseParametersType>::RequestData(
        grpc::ClientContext* ctx,
        ResponseParametersType* responseParams,
        grpc::Status* status,
        Client& client)
        : context_(ctx),
          responseParams_(responseParams),
          status_(status),
          clientEnclosure_(client){}

template<typename ResponseParametersType>
Client::RequestData<ResponseParametersType>::~RequestData(){
    delete context_;
    delete responseParams_;
    delete status_;
}

void Client::AsyncCompleteRpc() {
    void* tag;
    bool ok;
    while (!is_shutting_down_) {
        // Block until the next result is available in the completion queue
        while (cq_.Next(&tag, &ok)) {
            auto* rpcData = static_cast<RequestDataBase*>(tag);
            if (ok) {
                rpcData->process();
            } else {
                std::cerr << "RPC error!" << std::endl;
            }
            delete rpcData;
        }
    }
}

void Client::generateDisplayRequestAsync(std::string&& message, std::string&& orderBookName) {
    // record internal ID to track results
    auto internalID = nextInternalID();
    internalIdToRequestTypeMap_[internalID] = "Display";

    // create request
    marketAccess::DisplayParameters request;
    request.set_info(internalID);

    // Create a new context
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", orderBookName);

    // Create an async responseParameters_ reader
    auto response = new marketAccess::OrderBookContent;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<marketAccess::OrderBookContent>> rpc(
            stub_->AsyncDisplay(context, request, &cq_));
        // RequestHandler the async call to finish
    rpc->Finish(response, status, (void*)new RequestData{context, response, status, *this});
}

void Client::generateInsertionRequestAsync(std::string&& orderBookName,
                                           int userID,
                                           double price,
                                           double volume,
                                           orderDirection buyOrSell,
                                           orderType boType) {
    // record internal ID to track results
    auto internalID = nextInternalID();
    internalIdToRequestTypeMap_[internalID] = "Insertion";

    //Create request
    marketAccess::InsertionParameters request;
    request.set_info(internalID);
    request.set_userid(userID);
    request.set_price(price);
    request.set_volume(volume);
    request.set_buyorsell(static_cast<marketAccess::orderDirection>(buyOrSell));
    request.set_botype(static_cast<marketAccess::orderType>(boType));

    // Create a new context_
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", orderBookName);

    // Create an async responseParameters_ reader
    auto response = new marketAccess::InsertionConfirmation;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<marketAccess::InsertionConfirmation>> rpc(
            stub_->AsyncInsertion(context, request, &cq_));
    // RequestHandler the async call to finish
    rpc->Finish(response, status, (void*)new RequestData{context, response, status, *this});
}

void Client::generateUpdateRequestAsync(std::string&& orderBookName,
                                           int userID,
                                           uint64_t updatedBO,
                                           double price,
                                           double volume,
                                           orderDirection buyOrSell,
                                           orderType boType) {
    // record internal ID to track results
    auto internalID = nextInternalID();
    internalIdToRequestTypeMap_[internalID] = "Update";

    //Create request
    marketAccess::UpdateParameters request;
    request.set_info(internalID);
    request.set_userid(userID);
    request.set_boid(updatedBO);
    request.set_price(price);
    request.set_volume(volume);
    request.set_buyorsell(static_cast<marketAccess::orderDirection>(buyOrSell));
    request.set_botype(static_cast<marketAccess::orderType>(boType));

    // Create a new context_
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", orderBookName);

    // Create an async responseParameters_ reader
    auto response = new marketAccess::UpdateConfirmation;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<marketAccess::UpdateConfirmation>> rpc(
            stub_->AsyncUpdate(context, request, &cq_));
    // RequestHandler the async call to finish
    rpc->Finish(response, status, (void*)new RequestData{context,  response, status, *this});
}

void Client::generateDeleteRequestAsync(std::string&& orderBookName,
                                           int userID,
                                           uint64_t deletedID) {
    // record internal ID to track results
    auto internalID = nextInternalID();
    internalIdToRequestTypeMap_[internalID] = "Deletion";

    //Create request
    marketAccess::DeletionParameters request;
    request.set_info(internalID);
    request.set_userid(userID);
    request.set_boid(deletedID);

    // Create a new context_
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", orderBookName);

    // Create an async responseParameters_ reader
    auto response = new marketAccess::DeletionConfirmation;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<marketAccess::DeletionConfirmation>> rpc(
            stub_->AsyncDelete(context, request, &cq_));
    // RequestHandler the async call to finish
    rpc->Finish(response, status, (void*)new RequestData{context, response, status, *this});
}

template<typename ResponseParametersType>
void Client::RequestData<ResponseParametersType>::process(){
    if (status_->ok()) {
        std::cout << "Response from Type B: " << responseParams_->comment() << std::endl;
        auto requestID = stoi(responseParams_->info());
        auto requestType = clientEnclosure_.internalIdToRequestTypeMap_.find(requestID);
        if( requestType == end( clientEnclosure_.internalIdToRequestTypeMap_ ) ){
            std::cout<<"Response received for non registered request: "<<requestID;
            return;
        }
        if((*requestType).second == "Display"){
            handleResponse();
        }
        clientEnclosure_.internalIdToRequestTypeMap_.erase(requestID);
    } else {
        std::cerr << "RPC failed: " << status_->error_message() << std::endl;
        // problem: does not delete from internalIdToRequestTypeMap_
    }

}

template<>
void Client::RequestData<marketAccess::InsertionConfirmation>::handleResponse() {
    if (responseParams_->has_boid()) {
        std::cout << responseParams_->validation() << " new BO ID: " << responseParams_->boid();
    }else {
        std::cout << "Insertion request failed" << std::endl;
    }
}

template<>
void Client::RequestData<marketAccess::DeletionConfirmation>::handleResponse() {
    if (responseParams_->has_boid()) {
        std::cout << responseParams_->validation() << " new BO ID: " << responseParams_->boid();
    }else {
        std::cout << "Deletion request failed" << std::endl;
    }
}

template<>
void Client::RequestData<marketAccess::UpdateConfirmation>::handleResponse() {
    if (responseParams_->has_boid()) {
        std::cout << responseParams_->validation() << " new BO ID: " << responseParams_->boid();
    }else {
        std::cout << "Update request failed" << std::endl;
    }
}

template<>
void Client::RequestData<marketAccess::OrderBookContent>::handleResponse() {
    if (responseParams_->validation()) {
        std::cout << "Orderbook: " << std::endl << responseParams_->orderbook();
    }else {
        std::cout << "Display request failed" << std::endl;
    }
}
