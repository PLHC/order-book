#include <iostream>
#include <thread>
#include "market/Market.h"
#include "server_and_client_grpc/running_server_function/RunningServer.h"

int main(int argc, char *argv[]) {
    GeneratorId genID(0);
    Market tradingPlatform(&genID);
    for (int i = 1; i < argc; ++i) {
        tradingPlatform.createNewOrderBook(argv[i]);
    }
    // Run server for the trading platform
    auto server_thread = std::thread(RunServer, &tradingPlatform, "localhost:50051");

    if (server_thread.joinable())
        server_thread.join();

    return 0;
}
