#include "ClientAsync.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using marketAccess::Communication;
using marketAccess::OrderBookContent;

Client::Client(const std::shared_ptr<Channel>& channel)
        : stub_(Communication::NewStub(channel)), is_shutting_down_(false) {
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
Client::RpcData<RequestResponseType>::RpcData(grpc::ClientContext* c,
                                              RequestResponseType* r,
                                              grpc::Status* s):
        context_(c),
        response_(r),
        status_(s){}

template<typename RequestResponseType>
Client::RpcData<RequestResponseType>::~RpcData(){
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
            auto* rpcData = static_cast<RpcDataBase*>(tag);

            if (ok) {
               rpcData->process();
            } else {
                std::cerr << "RPC encountered an error!" << std::endl;
            }
            delete rpcData;
        }
    }
}

std::string Client::HandleDisplayRequestAsync(std::string&& message, std::string&& orderBookName) {
    marketAccess::DisplayParameters request;
    request.set_requestnumber(message);

    // Create a new context_
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", orderBookName);

    // Create an async response_ reader
    auto response = new marketAccess::OrderBookContent;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<marketAccess::OrderBookContent>> rpc(
            stub_->AsyncDisplayRequest(context, request, &cq_));
    
    // Request the async call to finish
    rpc->Finish(response, status, (void*)new RpcData{context, response, status});
    return response->orderbook();
}

std::pair<bool, u_int64_t> Client::HandleInsertionRequestAsync(std::string&& orderBookName,
                                         int userID,
                                         double price,
                                         double volume,
                                         orderDirection buyOrSell,
                                         orderType boType) {
    marketAccess::InsertionParameters request;
    request.set_userid(userID);
    request.set_price(price);
    request.set_volume(volume);
    request.set_buyorsell(static_cast<marketAccess::orderDirection>(buyOrSell));
    request.set_botype(static_cast<marketAccess::orderType>(boType));

    // Create a new context_
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", orderBookName);

    // Create an async response_ reader
    auto response = new marketAccess::Confirmation;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<marketAccess::Confirmation>> rpc(
            stub_->AsyncInsertionRequest(context, request, &cq_));
    // Request the async call to finish
    rpc->Finish(response, status, (void*)new RpcData{context, response, status});

    if(response->has_boid())
        return {response->validation(), response->boid()};
    else
        return {response->validation(), 0};
}

template<typename RequestResponseType>
void Client::RpcData<RequestResponseType>::process(){
    if (status_->ok()) {
        std::cout << "Response from Type B: " << response_->comment() << std::endl;
    } else {
        std::cerr << "RPC failed: " << status_->error_message() << std::endl;
    }

}



int main(int argc, char** argv) {
    // argv:
    // 1: client number
    // 2: orderbook 1
    // 3: orderbook 2
    Client client1(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    int i = 1;
    while (i < 4) {
//        std::cout << argv[1] << " sending " << std::to_string(i) << " to " << argv[3] << std::endl;
        auto insertion = client1.HandleInsertionRequestAsync(argv[2], std::stoi(argv[1]), 20, 1, buy, GoodTilCancelled);
        std::cout<<std::endl<<insertion.first<<" "<<insertion.second<<std::endl;

//        std::cout << argv[1] << " sending " << std::to_string(i) << " to " << argv[2] << std::endl;
        std::cout<<client1.HandleDisplayRequestAsync(std::to_string(i), argv[2])<<std::endl;
        i++;

    }

//    i = 1;
//    while (i < 4) {
//        std::cout << argv[1] << " sending " << std::to_string(i) << " to " << argv[3] << std::endl;
//        client1.HandleDisplayRequestAsync(std::to_string(i++), argv[3]);
//    }

    // Give the client some time to complete all RPCs before shutdown
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}