#include <iostream>
#include <thread>
#include "market/Market.h"
#include "server_and_client_grpc/Server/Server.h"

int main(int argc, char* argv[]) {
    std::cout<<"in main"<<std::endl;
    Market M{};
    for(int i=1; i<argc; ++i){
        M.createNewOrderBook(argv[i]);
    }
    // Run server for market platform M
    auto server_thread = std::thread(RunServer, &M, "localhost:50051");

//    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//    auto gold_client_thread = std::thread(RunMarketClient, "Gold");
//    auto silver_client_thread = std::thread(RunMarketClient, "Silver");
//    auto bronze_client_thread = std::thread(RunMarketClient, "Bronze");



    if(server_thread.joinable())
        server_thread.join();

    return 0;
}
