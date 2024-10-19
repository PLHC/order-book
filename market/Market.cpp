#include "Market.h"


Market::~Market(){
    std::cout<<"Market destructor begins"<<std::endl;
    for(auto & [product, orderBookPointer] : productToOrderBookMap_){
        delete orderBookPointer;
    }
    std::cout<<"Market destructor ends"<<std::endl;
}


void Market::createNewOrderBook(const std::string& product_ID) {
    if(productToOrderBookMap_.count(product_ID)){ // orderbook already exist
        return;
    }
    auto pointerToOrderBook = new OrderBook(product_ID);
    std::unique_lock<std::mutex> mapLock(orderbookMapMtx_);
    productToOrderBookMap_[product_ID] = pointerToOrderBook;
}

void Market::deleteOrderBook(const std::string &product_ID) {
    std::unique_lock<std::mutex> mapLock(orderbookMapMtx_);
    if(!productToOrderBookMap_.count(product_ID)){ // orderbook does not exist
        return;
    }
    delete productToOrderBookMap_[product_ID];
    productToOrderBookMap_.erase(product_ID);
}