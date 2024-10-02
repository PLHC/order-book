#include "server_and_client_grpc/Client/RandomizerClient/RandomizerClient.h"
#include "./market/Order.h"

int main(int argc, char** argv) {
    // argv:
    // 1: client number
    // 2: orderbook 1
    // 3: orderbook 2
    RandomizerClient randomClient(grpc::CreateChannel("localhost:50051",
                                                      grpc::InsecureChannelCredentials() ),
                                  std::stoi(argv[1]),
                                  9,
                                  20,
                                  80,
                                  argv[2]);

    std::this_thread::sleep_for(std::chrono::seconds(1));

//    randomClient.generateDisplayRequestAsync("abc", argv[2]);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    for(int i=250; i>0; i--) {
//        randomClient.generateDisplayRequestAsync("abc", argv[2]);
        randomClient.randomlyInsertOrUpdateOrDelete();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
//        randomClient.generateDisplayRequestAsync("abc", argv[2]);
    }


    // Give the client some time to complete all RPCs before shutdown
    std::this_thread::sleep_for(std::chrono::seconds(1));

    randomClient.generateDisplayRequestAsync("abc", argv[2]);

    std::this_thread::sleep_for(std::chrono::seconds(1));


    return 0;
}