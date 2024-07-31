//
// Created by Paul  on 04/07/2024.
//

#include "OrderBook.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>

OrderBook::OrderBook(CustomerRequestQueue * requestQueue):
        bids_(buy),
        offers_(sell),
        IDtoPointerMap(),
        requestQueue_(requestQueue){}

void OrderBook::deleteOrder(DeleteRequestNode* node) {
    //check if product and order exist first or throw error
    deletion(getterPointerToOrderFromID(node->getterBoID()));
}

void OrderBook::insertOrder(InsertRequestNode* node) {
    //check if product exists first or throw error
    auto newOrder = new Order(node->getterUserID(),
                              node->getterBoID(),
                              node->getterPrice(),
                              node->getterVolume(),
                              node->getterProductID(),
                              node->getterOrderDirection(),
                              node->getterOrderType());
    insertion(newOrder);
}

void OrderBook::updateOrder(UpdateRequestNode* node) {
    //check if product and order exist first or throw error
    auto updatedOrder = getterPointerToOrderFromID(node->getterUpdatedOrderID());
    auto newOrder = new Order(node->getterUserID(),
                              node->getterBoID(),
                              node->getterPrice(),
                              node->getterVolume(),
                              node->getterProductID(),
                              node->getterOrderDirection(),
                              node->getterOrderType());
    update(updatedOrder, newOrder);
}

orderExecution OrderBook::checkExecution(Order* orderToBeChecked){
    auto volumeInHundredths = static_cast<int32_t>(orderToBeChecked->getterVolumeInHundredth());
    auto priceInCents = orderToBeChecked->getterPriceInCents();
    auto& LinkedList = orderToBeChecked->getterOrderDirection()==buy? offers_ : bids_;
    auto nextOrder = LinkedList.getterHead();

    while(nextOrder && volumeInHundredths > 0 &&
          (orderToBeChecked->getterOrderDirection() == buy ?
           nextOrder->getterPriceInCents() <= priceInCents : nextOrder->getterPriceInCents() >= priceInCents)){
        volumeInHundredths -= static_cast<int32_t>(nextOrder->getterVolumeInHundredth());
        nextOrder = nextOrder->getterNextBO();
    }
    if(volumeInHundredths<=0) return fullExecution;
    else if(volumeInHundredths<orderToBeChecked->getterVolumeInHundredth()) return partialExecution;
    else return noExecution;
}

void OrderBook::deletion(Order* deletedOrder) {
//    if(IDtoPointerMap.find(boID)==end(IDtoPointerMap)) throw std::invalid_argument("Order "+std::to_string(boID)+" non existing in this financial product");
    if(deletedOrder->getterNextBO()!= nullptr) {
        deletedOrder->getterNextBO()->updatePrevBO(deletedOrder->getterPrevBO());
    }
    deletedOrder->getterPrevBO()->updateNextBO(deletedOrder->getterNextBO());
    auto& LinkedList = deletedOrder->getterOrderDirection()==buy? bids_ : offers_;
    if(LinkedList.getterTail()==deletedOrder) LinkedList.updateTail(deletedOrder->getterPrevBO());
    IDtoPointerMap.erase(deletedOrder->getterBoID());
    delete deletedOrder;
    // update database about deleted order
}

void OrderBook::insertion(Order* newOrder){
    auto execution = checkExecution(newOrder);

    if(newOrder->getterOrderType()==FillOrKill && execution!=fullExecution){
        //record order as deleted and never filled in memory
        return;
    }

    auto& LinkedList = newOrder->getterOrderDirection()==buy? bids_ : offers_;
    auto nextBO = LinkedList.getterHead();
    auto priceInCents = newOrder->getterPriceInCents();
    while(nextBO &&
          (newOrder->getterOrderDirection()==buy?
           priceInCents<=nextBO->getterPriceInCents() : priceInCents>=nextBO->getterPriceInCents())){
        nextBO = nextBO->getterNextBO();
    }
    if(nextBO==nullptr){
        newOrder->updatePrevBO(LinkedList.getterTail());
        LinkedList.getterTail()->updateNextBO(newOrder);
        LinkedList.updateTail(LinkedList.getterTail()->getterNextBO());
    }
    else{
        auto prevBO = nextBO->getterPrevBO();
        newOrder->updatePrevBO(prevBO);
        newOrder->updateNextBO(nextBO);
        nextBO->updatePrevBO(newOrder);
        prevBO->updateNextBO(newOrder);
    }
    IDtoPointerMap[newOrder->getterBoID()] = newOrder;
    //record order in memory

    if(execution!=noExecution) performExecution(newOrder);
}

void OrderBook::performExecution(Order* executingOrder) {
    auto& LinkedList = executingOrder->getterOrderDirection()==buy? offers_ : bids_;
    auto orderToBeUpdated = LinkedList.getterHead();
    Order* nextOrder;
    auto volumeInHundredths = executingOrder->getterVolumeInHundredth();
    auto priceInCents = executingOrder->getterPriceInCents();

    while(orderToBeUpdated && volumeInHundredths > 0 && (executingOrder->getterOrderDirection() == buy ?
                orderToBeUpdated->getterPriceInCents() <= priceInCents :
                orderToBeUpdated->getterPriceInCents() >= priceInCents)){
        nextOrder = orderToBeUpdated->getterNextBO();
        if(volumeInHundredths >= orderToBeUpdated->getterVolumeInHundredth()) {
            volumeInHundredths -= orderToBeUpdated->getterVolumeInHundredth();
            orderToBeUpdated->updateVolume(0);
            //orderToBeUpdated: record changes in memory
            deletion(orderToBeUpdated);
        }
        else{
            orderToBeUpdated->updateVolume(
                    (orderToBeUpdated->getterVolumeInHundredth() - volumeInHundredths)/100.0);
            volumeInHundredths=0;
            //record in database changes done on orderToBeUpdated
        }
        orderToBeUpdated = nextOrder;
    }

    if(volumeInHundredths==0){
        //update database for executing order
        deletion(executingOrder);
    }
    else{
        executingOrder->updateVolume(volumeInHundredths/100.0);
        //update database for executing order
    }
}

void OrderBook::update(Order* updatedOrder,
                       Order* newOrder){
    if(updatedOrder->checkIfOnlyVolumeUpdatedAndDown(newOrder)){
        newOrder->updateNextBO(updatedOrder->getterNextBO());
        newOrder->updatePrevBO(updatedOrder->getterPrevBO());
        newOrder->getterNextBO()->updatePrevBO(newOrder);
        newOrder->getterPrevBO()->updateNextBO(newOrder);
        IDtoPointerMap[newOrder->getterBoID()] = newOrder;
        IDtoPointerMap.erase(updatedOrder->getterBoID());
        //update database with newOrder and update updatedOrder to say replaced by newOrder
        delete updatedOrder;
        return;
    }

    //update database newOrder is replacing updatedOrder
    deletion(updatedOrder);
    insertion(newOrder);
}

void OrderBook::displayOrderBook() {
    auto bidNode = bids_.getterHead();
    auto offerNode = offers_.getterHead();
    std::cout << std::fixed << std::setprecision(2);
    std::ostringstream oss;
    system("clear");
    oss<<std::left<<std::setw(32)<<"Bids"<<"|"<<std::setw(30)<<"Offers"<<std::endl;
    oss<<std::left<<std::setw(10)<<"ID"<<"|"<<std::setw(10)<<"Vol"<<"|"<<std::setw(10)<<"Price";
    oss<<"|"<<std::setw(10)<<"Price"<<"|"<<std::setw(10)<<"Vol"<<"|"<<std::setw(10)<<"ID"<<std::endl;
    while(bidNode || offerNode){
        if(bidNode){
            oss<<std::left<<std::setw(10)<<bidNode->getterBoID()<<"|"<<std::setw(10)<<bidNode->getterVolume()<<
                "|"<<std::setw(10)<<bidNode->getterPrice()<<"|";
            bidNode = bidNode->getterNextBO();
        }
        else{
            oss<<std::left<<std::setw(10)<<""<<"|"<<std::setw(10)<<""<<"|"<<std::setw(10)<<""<<"|";
        }
        if(offerNode){
            oss<<std::setw(10)<<offerNode->getterPrice()<<"|"<<std::setw(10)<<offerNode->getterVolume()<<"|"
                    <<std::setw(10)<<offerNode->getterBoID()<<std::endl;
            offerNode = offerNode->getterNextBO();
        }
        else{
            oss<<std::setw(10)<<""<<"|"<<std::setw(10)<<""<<"|"<<std::setw(10)<<""<<std::endl;
        }
    }
    oss<<std::endl;
    auto table = oss.str();
    std::cout<<table;
}

void OrderBook::startListeningToRequests() {

}
