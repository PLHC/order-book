#include "ClientAsync.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;

// Explicit instantiations of the constructors,
// in the cpp file so instantiation happens only once in the entire program
template ClientAsync::RequestData<marketAccess::UpdateConfirmation>::RequestData(
        grpc::ClientContext* ctx,
        marketAccess::UpdateConfirmation* responseParams,
        grpc::Status* status,
        ClientAsync& client
);

template ClientAsync::RequestData<marketAccess::DeletionConfirmation>::RequestData(
        grpc::ClientContext* ctx,
        marketAccess::DeletionConfirmation* responseParams,
        grpc::Status* status,
        ClientAsync& client
);

template ClientAsync::RequestData<marketAccess::InsertionConfirmation>::RequestData(
        grpc::ClientContext* ctx,
        marketAccess::InsertionConfirmation* responseParams,
        grpc::Status* status,
        ClientAsync& client
);

ClientAsync::ClientAsync(const std::shared_ptr<Channel>& channel, const uint32_t nbOfThreadsInThreadPool)
        : stub_(marketAccess::Communication::NewStub(channel)),
          clientInternalId_(0),
          internalIdLock_(),
          io_context_(),
          work_guard_(io_context_.get_executor()),
          threadPool_(){
    is_shutting_down_.store(false);

    // Create a threadpool for processing the responses asynchronously
    for (int i = 0; i < nbOfThreadsInThreadPool; ++i) {
        threadPool_.emplace_back([this]() {io_context_.run();});
    }

    // Start a separate thread to process the CompletionQueue
    cq_thread_ = std::thread([this]() { this->AsyncCompleteRpc(); });
}

ClientAsync::~ClientAsync() {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // waits for last answers from server
    is_shutting_down_.store(true);
    io_context_.stop();
    cq_.Shutdown();
    if (cq_thread_.joinable()) {
        cq_thread_.join();
    }

    for (auto& thread : threadPool_) {
        if(thread.joinable()) {
            thread.join();
        }
    }
}

u_int64_t ClientAsync::nextInternalID() {
    std::unique_lock<std::mutex> genLock (internalIdLock_);

    if(clientInternalId_==std::numeric_limits<uint64_t>::max()){
        throw std::overflow_error("Client internal ID overflow");
    }
    auto newID = ++clientInternalId_;

    genLock.unlock();

    return newID;
}

void ClientAsync::AsyncCompleteRpc() {
    void* tag;
    bool ok;
    while (!is_shutting_down_.load()) {
        // Block until the next result is available in the completion queue
        while (cq_.Next(&tag, &ok)) {
            auto* rpcData = static_cast<RequestDataBase*>(tag);
            if (ok) {
                io_context_.post([rpcData]() {
                    rpcData->process();
                    delete rpcData;
                });
            } else {
                throw std::logic_error("RPC error");
            }
        }
    }
}

void ClientAsync::generateDisplayRequestAsync(std::string&& orderBookName,
                                              uint32_t nbOfOrdersToDisplay) {
    // create request
    marketAccess::DisplayParameters request;
    request.set_info(0); // Dispatch requests are tagged with internalID 0
    request.set_nboforderstodisplay(nbOfOrdersToDisplay);

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

void ClientAsync::generateInsertionRequestAsync(std::string&& orderBookName,
                                                std::string userID,
                                                double price,
                                                double volume,
                                                orderDirection buyOrSell,
                                                orderType boType) {

    //Create request
    marketAccess::InsertionParameters request;
    request.set_info(nextInternalID() );
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

void ClientAsync::generateUpdateRequestAsync(std::string&& orderBookName,
                                             std::string userID,
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

void ClientAsync::generateDeleteRequestAsync(std::string&& orderBookName,
                                             std::string userID,
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

void ClientAsync::handleResponse(const marketAccess::OrderBookContent* responseParams) {
    if (responseParams->validation()) {
        std::cout << "Orderbook: " << std::endl << responseParams->orderbook();
    }else {
        std::cout << "Display request failed" << std::endl;
    }
}


template<typename ResponseParametersType>
ClientAsync::RequestData<ResponseParametersType>::RequestData(
        grpc::ClientContext* ctx,
        ResponseParametersType* responseParams,
        grpc::Status* status,
        ClientAsync& client)
        : context_(ctx),
          responseParams_(responseParams),
          status_(status),
          nestingClient_(client){}

template<typename ResponseParametersType>
ClientAsync::RequestData<ResponseParametersType>::~RequestData(){
    delete context_;
    delete responseParams_;
    delete status_;
}

template<typename ResponseParametersType>
void ClientAsync::RequestData<ResponseParametersType>::process(){
    if (status_->ok()) {
        dispatchResponse();
    } else {
        std::cerr << "RPC failed: " << status_->error_message() << std::endl;
    }
}