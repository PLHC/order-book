#include "ClientAsync.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using marketAccess::Communication;
using marketAccess::OrderBookContent;

Client::Client(const std::shared_ptr<Channel>& channel)
        : stub_(Communication::NewStub(channel)),
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

template<typename RequestResponseType>
Client::RequestData<RequestResponseType>::RequestData(grpc::ClientContext* c,
                                                      RequestResponseType* r,
                                                      grpc::Status* s,
                                                      Client& client):
        context_(c),
        response_(r),
        status_(s),
        clientEnclosure_(client){}

template<typename RequestResponseType>
Client::RequestData<RequestResponseType>::~RequestData(){
    delete context_;
    delete response_;
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
                std::cerr << "RPC encountered an error!" << std::endl;
            }
            delete rpcData;
        }
    }
}

void Client::generateDisplayRequestAsync(std::string&& message, std::string&& orderBookName) {
    // record internal ID to track results
    auto internalId = nextInternalID();
    internalIdToRequestTypeMap_[internalId] = "Display";

    // create request
    marketAccess::DisplayParameters request;
    request.set_info(internalId);

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
    auto internalId = nextInternalID();
    internalIdToRequestTypeMap_[internalId] = "Insertion";

    //Create request
    marketAccess::InsertionParameters request;
    request.set_info(internalId);
    request.set_userid(userID);
    request.set_price(price);
    request.set_volume(volume);
    request.set_buyorsell(static_cast<marketAccess::orderDirection>(buyOrSell));
    request.set_botype(static_cast<marketAccess::orderType>(boType));

    // Create a new context_
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", orderBookName);

    // Create an async responseParameters_ reader
    auto response = new marketAccess::Confirmation;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<marketAccess::Confirmation>> rpc(
            stub_->AsyncInsertion(context, request, &cq_));
    // RequestHandler the async call to finish
    rpc->Finish(response, status, (void*)new RequestData{context, response, status, *this});
}



template<typename RequestResponseType>
void Client::RequestData<RequestResponseType>::process(){
    if (status_->ok()) {
        std::cout << "Response from Type B: " << response_->comment() << std::endl;
        auto requestID = stoi(response_->info());
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
void Client::RequestData<marketAccess::Confirmation>::handleResponse() {
    if (response_->has_boid()) {
        std::cout << response_->validation() << " new BO ID: " << response_->boid();
    }else {
        std::cout << "Insertion/Update/Deletion request failed" << std::endl;
    }
}

template<>
void Client::RequestData<marketAccess::OrderBookContent>::handleResponse() {
    if (response_->validation()) {
        std::cout << "Orderbook: " << std::endl << response_->orderbook();
    }else {
        std::cout << "Display request failed" << std::endl;
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

int main(int argc, char** argv) {
    // argv:
    // 1: client number
    // 2: orderbook 1
    // 3: orderbook 2
    Client client1(grpc::CreateChannel("localhost:50051",
                                                grpc::InsecureChannelCredentials()));

    int i = 1;
    while (i < 4) {
        client1.generateInsertionRequestAsync(argv[2], std::stoi(argv[1]),
                                              20, 1, BUY, GOOD_TIL_CANCELLED);
        client1.generateDisplayRequestAsync(std::to_string(i), argv[2]);
        i++;
    }


    // Give the client some time to complete all RPCs before shutdown
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}