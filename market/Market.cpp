#include "Market.h"
#include <iostream>


Market::Market(GeneratorID * genID):
        genID_(genID),
        ProductToOrderBookMap(),
        ProductToOrderBookThreadMap()
    {
    std::cout<<"in market constructor"<<std::endl;
}

Market::~Market(){
    std::cout<<"in market destructor"<<std::endl;
    for(const auto & [product, orderBookPointer] : ProductToOrderBookMap){
        orderBookPointer->setterStopFlagToTrue();
    }
    for(auto & [product, orderBookThread]: ProductToOrderBookThreadMap){
        if(orderBookThread.joinable()) orderBookThread.join();
    }
    for(auto & [product, orderBookPointer] : ProductToOrderBookMap){
        delete orderBookPointer;
    }

    /// join server grpc threads too so they can answer ?
}


void Market::createNewOrderBook(const std::string& product_ID) {
    auto pointerToOrderBook = new OrderBook(product_ID, genID_);
    ProductToOrderBookMap[product_ID] = pointerToOrderBook;
    ProductToOrderBookThreadMap[product_ID] = std::thread(&OrderBook::processRequests, pointerToOrderBook);

}

void Market::deleteOrderBook(const std::string &product_ID) {
    // acquire lock for it first
    // check if orderbook existing or throw error
    delete ProductToOrderBookMap[product_ID];
    ProductToOrderBookMap.erase(product_ID);
}
