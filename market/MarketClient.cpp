//
// Created by Paul  on 13/08/2024.
//

#include "MarketClient.h"

void DisplayRequest(std::shared_ptr<grpc::Channel> channel, const std::string & product, const int i) {
    marketAccess::Communication::Stub stub(channel);

    marketAccess::DisplayParameters request;
    request.set_product_id(product);
    request.set_requestnumber("nb"+std::to_string(i));

    marketAccess::OrderBookContent reply;
    grpc::ClientContext context;

    grpc::Status status = stub.DisplayRequest(&context, request, &reply);

    if (status.ok()) {
        std::cout << "Response received: " << reply.orderbook() << std::endl;
    } else {
        std::cout << "RPC failed" << std::endl;
    }
}

void RunMarketClient(const std::string product){
    std::string target_str = "localhost:50051";
    auto channel = grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials());
    for(int i = 1; i<10; ++i) DisplayRequest(channel, product, i);
}