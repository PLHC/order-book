//
// Created by Paul  on 06/07/2024.
//

#ifndef ORDERBOOK_BOT_H
#define ORDERBOOK_BOT_H

#include <chrono>
#include <thread>
#include "Market.h"
#include <random>


using namespace std::chrono_literals;

class Bot {
    std::chrono::milliseconds sleepTimerInMilliseconds_;
    Market* market_;
public:
    Bot(std::chrono::milliseconds sleepTimerInMilliseconds, Market* M);
    void generateInteractionsWithMarket();
};

#endif //ORDERBOOK_BOT_H
