#include "Market.h"


Market::~Market(){
    std::cout<<"Market destructor begins"<<std::endl;
    for(auto & [product, orderBookPointer] : productToOrderBookMap_){
        delete orderBookPointer;
    }
    delete genId_;
    std::cout<<"Market destructor ends"<<std::endl;
}


void Market::createNewOrderBook(const std::string& product_ID) {
    std::unique_lock<std::mutex> mapLock(orderbookMapMtx_);
    if(productToOrderBookMap_.count(product_ID)){ // orderbook already exists
        return;
    }
    mapLock.unlock();
    auto pointerToOrderBook = new OrderBook(product_ID);

    mapLock.lock();
    productToOrderBookMap_[product_ID] = pointerToOrderBook;
}

void Market::deleteOrderBook(const std::string& product_ID) {
    std::unique_lock<std::mutex> mapLock(orderbookMapMtx_);
    if(!productToOrderBookMap_.count(product_ID)){ // orderbook does not exist
        return;
    }
    auto orderBookPtr = productToOrderBookMap_[product_ID];;
    productToOrderBookMap_.erase(product_ID);
    mapLock.unlock();

    delete orderBookPtr;
}