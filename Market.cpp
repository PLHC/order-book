//
// Created by Paul  on 04/07/2024.
//

#include "Market.h"

Market::Market():
        ProductToOrderBookMap(),
        lastID(0) {} // replace last ID by an ID in database; needs a lock on it

Market::~Market(){
    for(auto & [product, OrderBookPointer] : ProductToOrderBookMap){
        delete OrderBookPointer;
    }
}

void Market::deleteOrder(const std::string &product,
                         uint32_t boID) {
    //check if product and order exist first or throw error
    auto orderBookPointer = ProductToOrderBookMap[product];
    orderBookPointer->deletion(orderBookPointer->getterPointerToOrderFromID(boID));
}

void Market::insertOrder(int32_t userID,
                         int32_t price,
                         uint32_t volume,
                         const std::string& product_ID,
                         orderDirection buyOrSell,
                         orderType boType) {
    //check if product exists first or throw error
    auto orderBookPointer = ProductToOrderBookMap[product_ID];
    auto newOrder = new Order(userID, nextID(), price, volume, product_ID, buyOrSell, boType);
    orderBookPointer->insertion(newOrder);
}

void Market::updateOrder(int32_t userID,
                         int32_t price,
                         uint32_t volume,
                         const std::string &product_ID,
                         orderDirection buyOrSell,
                         orderType boType,
                         uint32_t updatedOrderID) {
    //check if product and order exist first or throw error
    auto orderBookPointer = ProductToOrderBookMap[product_ID];
    auto updatedOrder = orderBookPointer->getterPointerToOrderFromID(updatedOrderID);
    auto newOrder = new Order(userID, nextID(), price, volume, product_ID, buyOrSell, boType);
    orderBookPointer->update(updatedOrder, newOrder);
}

void Market::deleteOrderBook(const std::string &product_ID) {
    // acquire lock for it first
    // check if orderbook existing or throw error
    delete ProductToOrderBookMap[product_ID];
    ProductToOrderBookMap.erase(product_ID);
}