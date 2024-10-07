#include <iostream>
#include <csignal>

#include "server_and_client_grpc/Client/RandomizerClient/RandomizerClient.h"


std::atomic<bool> stopFlag = false;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        stopFlag.store(true);
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
        tradedProducts.emplace_back(argv[i]);
        priceForecasts.push_back(std::stoi(argv[i+1]));
    }

    RandomizerClient randomClient(grpc::CreateChannel("localhost:50051",
                                                      grpc::InsecureChannelCredentials() ),
                                  argv[1],
                                  500,
                                  9,
                                  priceForecasts,
                                  tradedProducts,
                                  3); // 2 threads enough for sending 3 random requests every 1ms

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    while(!stopFlag.load()) {
        // followed by a sleep_for of 1ms, randomlyInsertOrUpdateOrDelete on 3 OBs was clocked between 100 and 700us
        randomClient.randomlyInsertOrUpdateOrDelete();
        std::this_thread::sleep_for(std::chrono::microseconds (1000));
    }


    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout<<" client is done "<<std::endl;
}