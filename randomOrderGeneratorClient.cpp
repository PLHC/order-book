#include "server_and_client_grpc/Client/RandomizerClient/RandomizerClient.h"
#include "./market/Order.h"
#include <iostream>

int main(int argc, char** argv) {
    // argv:
    // 1: client number
    // 2: orderbook 1
    // 3: orderbook 2
    std::cout<<argc<<std::endl;
    RandomizerClient randomClient(grpc::CreateChannel("localhost:50051",
                                                      grpc::InsecureChannelCredentials() ),
                                  std::stoi(argv[1]),
                                  42,
                                  9,
                                  80,
                                  argv[2]);

    std::this_thread::sleep_for(std::chrono::seconds(1));

//    for(int i=50000; i>0; i--) {
////        std::cout<<i<<std::endl;
//        randomClient.randomlyInsertOrUpdateOrDelete();
//        std::this_thread::sleep_for(std::chrono::microseconds(10));
//    }
    while(true) {
//        std::cout<<i<<std::endl;
        randomClient.randomlyInsertOrUpdateOrDelete();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout<<"random orders done"<<std::endl;


//    // Give the client some time to complete all RPCs before shutdown
    std::this_thread::sleep_for(std::chrono::seconds(1));
//
    randomClient.generateDisplayRequestAsync("abc", argv[2]);

    std::this_thread::sleep_for(std::chrono::seconds(1));


    return 0;
}