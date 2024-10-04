#include "Market.h"
#include <iostream>


Market::Market(GeneratorId * genID):
        genId_(genID),
        productToOrderBookMap_(),
        productToOrderBookThreadMap_(),
        stopFlag_(false){}

Market::~Market(){
    std::cout<<"Market destructor begins"<<std::endl;
    for(auto & [product, orderBookPointer] : productToOrderBookMap_){
        delete orderBookPointer;
    }
    std::cout<<"Market destructor ends"<<std::endl;
}


void Market::createNewOrderBook(const std::string& product_ID) {
    auto pointerToOrderBook = new OrderBook(product_ID, genId_);
    productToOrderBookMap_[product_ID] = pointerToOrderBook;
//    productToOrderBookThreadMap_[product_ID] = std::thread(&OrderBook::processRequests, pointerToOrderBook);

}

void Market::deleteOrderBook(const std::string &product_ID) {
    // acquire lock for it first
    // check if orderbook existing or throw error
    delete productToOrderBookMap_[product_ID];
    productToOrderBookMap_.erase(product_ID);

    //missing shutting down its thread
}

void Market::setterStopFlagToTrue() {
    stopFlag_.store(true);
//    for(const auto & [product, orderbookPtr]: productToOrderBookMap_){
//        orderbookPtr->setterStopFlagToTrue();
//    }
}
