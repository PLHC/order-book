#include <iostream>
#include <csignal>
#include <chrono>

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

    constexpr uint32_t nbOfOrdersOnEachSIde {500};
    constexpr uint32_t spread {9};
    constexpr uint32_t nbOfThreadsInThreadPool {2}; // 2 threads enough for sending 3 random requests every 200µs

    RandomizerClient randomClient(grpc::CreateChannel("localhost:50051",
                                                      grpc::InsecureChannelCredentials() ),
                                  argv[1],
                                  nbOfOrdersOnEachSIde,
                                  spread,
                                  priceForecasts,
                                  tradedProducts,
                                  nbOfThreadsInThreadPool);


    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // wait before starting

    while(!stopFlag.load()) {
        // Release: randomlyInsertOrUpdateOrDelete on 3 OBs was clocked between 15 and 250µs and mostly below 100µs
        randomClient.randomlyInsertOrUpdateOrDelete();
        std::this_thread::sleep_for(std::chrono::microseconds (100)); // adding 100µs makes the loop lasting on average 200µs
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout<<" client is done "<<std::endl;
}