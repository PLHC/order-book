#include "./server_and_client_grpc/Client/RandomOrderGeneratorClientImplementation.h"
#include "./market/Order.h"

int main(int argc, char** argv) {
    // argv:
    // 1: client number
    // 2: orderbook 1
    // 3: orderbook 2
    RandomOrderGeneratorClient randomClient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

    int i = 1;
    while (i < 4) {
        randomClient.generateInsertionRequestAsync(argv[2], std::stoi(argv[1]),
                                                   20, 1, BUY, GOOD_TIL_CANCELLED);
        randomClient.generateDisplayRequestAsync(argv[2], argv[2]);
        i++;
    }

    // Give the client some time to complete all RPCs before shutdown
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}