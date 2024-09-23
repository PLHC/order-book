#include "OrderBook.h"

OrderBook::OrderBook(std::string productID, GeneratorId * genID):
        productId_(std::move(productID)),
        genId_(genID),
        bids_(BUY),
        offers_(SELL),
        idToPointerMap_(),
        stopFlag_(false),
        requestQueue_(){}

orderExecution OrderBook::checkExecution(Order* orderToBeChecked){
    auto volumeInHundredths = static_cast<int32_t>(orderToBeChecked->getterVolumeInHundredth());
    auto priceInCents = orderToBeChecked->getterPriceInCents();
    auto& LinkedList = orderToBeChecked->getterOrderDirection() == BUY ? offers_ : bids_;
    auto nextOrder = LinkedList.getterTail();

    while(nextOrder && volumeInHundredths > 0 &&
          (orderToBeChecked->getterOrderDirection() == BUY ?
           nextOrder->getterPriceInCents() <= priceInCents : nextOrder->getterPriceInCents() >= priceInCents)){
        volumeInHundredths -= static_cast<int32_t>(nextOrder->getterVolumeInHundredth());
        nextOrder = nextOrder->getterNextBO();
    }
    if(volumeInHundredths<=0) return FULL_EXECUTION;
    else if(volumeInHundredths<orderToBeChecked->getterVolumeInHundredth()) return PARTIAL_EXECUTION;
    else return NO_EXECUTION;
}

void OrderBook::deletion(Order* deletedOrder) {
    if(deletedOrder->getterNextBO()!= nullptr) {
        deletedOrder->getterNextBO()->updatePrevBO(deletedOrder->getterPrevBO());
    }
    deletedOrder->getterPrevBO()->updateNextBO(deletedOrder->getterNextBO());
    auto& LinkedList = deletedOrder->getterOrderDirection() == BUY ? bids_ : offers_;
    if(LinkedList.getterHead() == deletedOrder) LinkedList.updateTail(deletedOrder->getterPrevBO());
    idToPointerMap_.erase(deletedOrder->getterBoID());
    delete deletedOrder;
    // update database about deleted order
}

void OrderBook::insertion(Order* newOrder){
    auto execution = checkExecution(newOrder);

    if(newOrder->getterOrderType() == FILL_OR_KILL && execution != FULL_EXECUTION){
        //record order as deleted and never filled in memory
        return;
    }

    auto& LinkedList = newOrder->getterOrderDirection() == BUY ? bids_ : offers_;
    auto nextBO = LinkedList.getterTail();
    auto priceInCents = newOrder->getterPriceInCents();
    while(nextBO &&
          (newOrder->getterOrderDirection() == BUY ?
           priceInCents<=nextBO->getterPriceInCents() : priceInCents>=nextBO->getterPriceInCents())){
        nextBO = nextBO->getterNextBO();
    }
    if(nextBO==nullptr){
        newOrder->updatePrevBO(LinkedList.getterHead());
        LinkedList.getterHead()->updateNextBO(newOrder);
        LinkedList.updateTail(LinkedList.getterHead()->getterNextBO());
    }
    else{
        auto prevBO = nextBO->getterPrevBO();
        newOrder->updatePrevBO(prevBO);
        newOrder->updateNextBO(nextBO);
        nextBO->updatePrevBO(newOrder);
        prevBO->updateNextBO(newOrder);
    }
    idToPointerMap_[newOrder->getterBoID()] = newOrder;
    //record order in memory

    if(execution != NO_EXECUTION) performExecution(newOrder);
}

void OrderBook::performExecution(Order* executingOrder) {
    auto& LinkedList = executingOrder->getterOrderDirection() == BUY ? offers_ : bids_;
    auto orderToBeUpdated = LinkedList.getterTail();
    Order* nextOrder;
    auto volumeInHundredths = executingOrder->getterVolumeInHundredth();
    auto priceInCents = executingOrder->getterPriceInCents();

    while(orderToBeUpdated && volumeInHundredths > 0 && (executingOrder->getterOrderDirection() == BUY ?
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

void OrderBook::update(Order* updatedOrder, Order* newOrder){
    if(updatedOrder->checkIfOnlyVolumeUpdatedAndDown(newOrder)){
        newOrder->updateNextBO(updatedOrder->getterNextBO());
        newOrder->updatePrevBO(updatedOrder->getterPrevBO());
        newOrder->getterNextBO()->updatePrevBO(newOrder);
        newOrder->getterPrevBO()->updateNextBO(newOrder);
        idToPointerMap_[newOrder->getterBoID()] = newOrder;
        idToPointerMap_.erase(updatedOrder->getterBoID());
        //update database with newOrder and update updatedOrder to say replaced by newOrder
        delete updatedOrder;
        return;
    }

    //update database newOrder is replacing updatedOrder
    deletion(updatedOrder);
    insertion(newOrder);
}

std::string OrderBook::displayOrderBook() {
    auto bidNode = bids_.getterTail();
    auto offerNode = offers_.getterTail();

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
    return oss.str();
}

void OrderBook::processRequests(){
    while(!stopFlag_){
        requestQueue_.runNextRequest();
    }
}