#include "server_and_client_grpc/Client/RandomizerClient/RandomizerClient.h"
#include "./market/Order.h"
#include <iostream>
#include <csignal>

bool stopFlag = false;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        stopFlag = true;
    }
}

int main(int argc, char** argv) {
    // argv:
    // 1: client number
    // 2: orderbook 1
    // 3: price forecast for orderbook 1
    // 4: orderbook 2
    // 5: price forecast for orderbook 2
    // ......

    std::signal(SIGINT, signalHandler);

    std::vector<std::string> tradedProducts;
    std::vector<int> priceForecasts;

    for(int i = 2; i<argc; i += 2){
        tradedProducts.push_back(argv[i]);
        priceForecasts.push_back(std::stoi(argv[i+1]));
    }

    RandomizerClient randomClient(grpc::CreateChannel("localhost:50051",
                                                      grpc::InsecureChannelCredentials() ),
                                  std::stoi(argv[1]),
                                  500,
                                  9,
                                  priceForecasts,
                                  tradedProducts);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    while(!stopFlag) {
        randomClient.randomlyInsertOrUpdateOrDelete();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
//    int i = 250;
//    while(i--) {
//        randomClient.randomlyInsertOrUpdateOrDelete();
//        std::this_thread::sleep_for(std::chrono::milliseconds(1));
//    }


    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout<<" client is done "<<std::endl;
}