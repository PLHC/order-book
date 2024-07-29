#include <iostream>
#include "Market.h"
#include "Bot.h"

int main() {
    Market M{};
    M.createNewOrderBook("Gold");
    Bot B{1ms, &M};
    B.generateInteractionsWithMarket();
    return 0;
}
