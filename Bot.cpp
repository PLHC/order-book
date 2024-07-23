//
// Created by Paul  on 06/07/2024.
//

#include "Bot.h"

Bot::Bot(std::chrono::milliseconds sleepTimerInMilliseconds, Market *M):
    sleepTimerInMilliseconds_(sleepTimerInMilliseconds),
    market_(M){}

void Bot::generateInteractionsWithMarket() {
    auto orderBook = market_->getterOrderBookPointer("Gold");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(1, 100);
    std::uniform_int_distribution<int32_t> distBuyOrSell(0, 1);
    std::uniform_int_distribution<int32_t> distOrderType(0, static_cast<int32_t>(orderType::orderTypesCount)-1);

    int32_t i = 1000;
    while(i--){
        std::this_thread::sleep_for(sleepTimerInMilliseconds_);
        market_->insertOrder(1,
                             dist(gen),
                             dist(gen),
                             "Gold",
                             static_cast<orderDirection>(distBuyOrSell(gen)),
                             static_cast<orderType>(distOrderType(gen)));

        orderBook->displayOrderBook();
    }
}


