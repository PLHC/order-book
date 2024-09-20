#include "ClientAsync.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using marketAccess::Communication;
using marketAccess::DisplayParameters;
using marketAccess::OrderBookContent;


Client::Client(std::shared_ptr<Channel> channel)
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

void Client::HandleDisplayRequestAsync(std::string&& message, std::string&& orderBookName) {
    DisplayParameters request;
    request.set_requestnumber(message);

    // Create a new context
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", orderBookName);

    // Create an async response reader
    auto response = new OrderBookContent;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<OrderBookContent>> rpc(
            stub_->AsyncDisplayRequest(context, request, &cq_));
    // Request the async call to finish
    rpc->Finish(response, status, (void*)new RpcData{context, response, status});
}


void Client::AsyncCompleteRpc() {
    void* tag;
    bool ok;
    while (!is_shutting_down_) {
        // Block until the next result is available in the completion queue
        while (cq_.Next(&tag, &ok)) {
            RpcData* rpcData = static_cast<RpcData*>(tag);

            if (ok) {
                if (rpcData->status->ok()) {
                    std::cout << "Response from Type B: " << rpcData->response->orderbook() << std::endl;
                } else {
                    std::cerr << "RPC failed: " << rpcData->status->error_message() << std::endl;
                }
            } else {
                std::cerr << "RPC encountered an error!" << std::endl;
            }

            // Clean up
            delete rpcData->context;
            delete rpcData->response;
            delete rpcData->status;
            delete rpcData;
        }
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
        std::cout << argv[1] << " sending " << std::to_string(i) << " to " << argv[2] << std::endl;
        client1.HandleDisplayRequestAsync(std::to_string(i++), argv[2]);
    }

    i = 1;
    while (i < 4) {
        std::cout << argv[1] << " sending " << std::to_string(i) << " to " << argv[3] << std::endl;
        client1.HandleDisplayRequestAsync(std::to_string(i++), argv[3]);
    }

    // Give the client some time to complete all RPCs before shutdown
    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}