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
          activeOrders(),
          activeOrdersListLock_(),
          conditionVariableActiveOrdersListLock_(),
          is_shutting_down_(false) {
    // Start a separate thread to process the CompletionQueue
    activeOrders.insert(0); // 0 used for dispatch requests
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
    conditionVariableGeneratorLock_.wait(genLock, [](){ return true; });

    auto newID = ++clientInternalId_;
    activeOrders.insert(newID);

    genLock.unlock();
    conditionVariableGeneratorLock_.notify_all();

    return newID;
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
    // create request
    marketAccess::DisplayParameters request;
    request.set_info(0); // Dispatch requests are tagged with internalID 0

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
                                           uint32_t userID,
                                           double price,
                                           double volume,
                                           orderDirection buyOrSell,
                                           orderType boType) {
    // record internal ID to track results
    auto internalID = nextInternalID();

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
                                        uint32_t userID,
                                        uint64_t updatedBO,
                                        double price,
                                        double volume,
                                        orderDirection buyOrSell,
                                        orderType boType) {
    // record internal ID to track results
    auto internalID = nextInternalID();

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
                                        uint32_t userID,
                                        uint64_t deletedID) {
    // record internal ID to track results
    auto internalID = nextInternalID();

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

void Client::handleResponse(const marketAccess::OrderBookContent *responseParams) {
    if (responseParams->validation()) {
        std::cout << "Orderbook: " << std::endl << responseParams->orderbook() << std::endl;
    }else {
        std::cout << "Display request for Orderbook "<<responseParams->info()<<" failed" << std::endl;
    }
}

void Client::handleResponse(const marketAccess::InsertionConfirmation *responseParams) {
    if (responseParams->validation()) {
        std::cout <<"Insertion request for order: "<<responseParams->info()<<" successful, new BO ID: " <<
                        responseParams->boid() << std::endl;
    }else {
        std::cout << "Insertion request for order: "<<responseParams->info()<<" failed" << std::endl;
    }
}

void Client::handleResponse(const marketAccess::UpdateConfirmation *responseParams) {
    if (responseParams->validation()) {
        std::cout <<"Update request for order: "<<responseParams->info()<<" successful, new BO ID: " <<
                        responseParams->boid() << std::endl;
    }else {
        std::cout << "Update request for order: "<<responseParams->info()<<" failed" << std::endl;
    }
}

void Client::handleResponse(const marketAccess::DeletionConfirmation *responseParams) {
    if (responseParams->validation()) {
        std::cout << "Deletion of order "<<responseParams->info()<<" successful" << std::endl;
    }else {
        std::cout << "Deletion of order "<<responseParams->info()<<" failed" << std::endl;
    }
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

template<typename ResponseParametersType>
void Client::RequestData<ResponseParametersType>::process(){
    if (status_->ok()) {
        auto requestID = stoi(responseParams_->info());

        std::unique_lock<std::mutex> listLock(clientEnclosure_.activeOrdersListLock_);
        clientEnclosure_.conditionVariableActiveOrdersListLock_.wait(listLock, [](){return true;});

        auto isIdAnActiveOrder = clientEnclosure_.activeOrders.count(requestID);

        listLock.unlock();
        clientEnclosure_.conditionVariableGeneratorLock_.notify_all();

        if( !isIdAnActiveOrder ){
            std::cout<<"Response received for non registered request: "<<requestID;
            return;
        }

        dispatchResponse();
        if(requestID!=0) {
            std::unique_lock<std::mutex> genLock (clientEnclosure_.generatorLock_);
            clientEnclosure_.conditionVariableGeneratorLock_.wait(genLock, [](){ return true; });

            clientEnclosure_.activeOrders.erase(requestID);

            genLock.unlock();
            clientEnclosure_.conditionVariableGeneratorLock_.notify_all();
        }
    } else {
        std::cerr << "RPC failed: " << status_->error_message() << std::endl;
        // problem: does not delete from activeOrders
    }

}
