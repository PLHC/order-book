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
    std::uniform_real_distribution<double> dist(1.0, 100.0);
    std::uniform_int_distribution<int32_t> distBuyOrSell(0, 1);
    std::uniform_int_distribution<int32_t> distOrderType(0, static_cast<int32_t>(orderType::orderTypesCount)-1);

    int32_t i = 15;
    while(i--){
//        std::this_thread::sleep_for(sleepTimerInMilliseconds_);
        market_->addInsertOrderToQueue(1,
                             dist(gen),
                             dist(gen),
                             "Gold",
                             static_cast<orderDirection>(distBuyOrSell(gen)),
                             static_cast<orderType>(distOrderType(gen)));

        market_->addDisplayRequestToQueue("Gold");
    }
}


