#include "Market.h"
#include <iostream>


Market::Market(GeneratorId * genID):
        genId_(genID),
        productToOrderBookMap_(),
        productToOrderBookThreadMap_()
    {}

Market::~Market(){
    for(const auto & [product, orderBookPointer] : productToOrderBookMap_){
        orderBookPointer->setterStopFlagToTrue();
    }
    for(auto & [product, orderBookThread]: productToOrderBookThreadMap_){
        if(orderBookThread.joinable()) orderBookThread.join();
    }
    for(auto & [product, orderBookPointer] : productToOrderBookMap_){
        delete orderBookPointer;
    }

    /// join server grpc threads too so they can answer ?
}


void Market::createNewOrderBook(const std::string& product_ID) {
    auto pointerToOrderBook = new OrderBook(product_ID, genId_);
    productToOrderBookMap_[product_ID] = pointerToOrderBook;
    productToOrderBookThreadMap_[product_ID] = std::thread(&OrderBook::processRequests, pointerToOrderBook);

}

void Market::deleteOrderBook(const std::string &product_ID) {
    // acquire lock for it first
    // check if orderbook existing or throw error
    delete productToOrderBookMap_[product_ID];
    productToOrderBookMap_.erase(product_ID);
}
