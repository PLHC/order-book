#include <iostream>
#include <thread>
#include "market/Market.h"
#include "market/Bot.h"
#include "market/MarketServer.h"
#include "market/MarketClient.h"

int main() {
    std::cout<<"in main"<<std::endl;
    Market M{};
    M.createNewOrderBook("Gold");
    M.createNewOrderBook("Silver");
    M.createNewOrderBook("Bronze");
//    Bot B{1ms, &M};
//    B.generateInteractionsWithMarket();
    auto server_thread = std::thread(&Market::RunMarketServer, &M);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    auto gold_client_thread = std::thread(RunMarketClient, "Gold");
    auto silver_client_thread = std::thread(RunMarketClient, "Silver");
    auto bronze_client_thread = std::thread(RunMarketClient, "Bronze");

    server_thread.join();
    gold_client_thread.join();
    silver_client_thread.join();
    bronze_client_thread.join();


    return 0;
}
