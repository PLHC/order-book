#include <iostream>
#include "Market.h"
#include "Bot.h"

#include <iostream>
int main() {
    std::cout<<"in main"<<std::endl;
    Market M{};
    M.createNewOrderBook("Gold");
    Bot B{1ms, &M};
    B.generateInteractionsWithMarket();
    return 0;
}
