#include "OrderBook.h"

OrderBook::OrderBook():
        bids_(buy),
        offers_(sell),
        IDtoPointerMap(),
        stopFlag(false),
        requestQueue_(){
    std::cout<<"in OB constructor"<<std::endl;
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

std::string OrderBook::displayOrderBook() {
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
    return oss.str();
}

void OrderBook::processRequests(){
    while(!stopFlag){
        requestQueue_.runNextRequest();
    }
}

//void OrderBook::processRequests() {
//    while (true) {
//        std::unique_lock<std::mutex> lock(requestQueue_.queueMutex_);
//        requestQueue_.queueConditionVariable_.wait(lock,
//                                                    [this](){return !requestQueue_.CRQueue_.empty() || stopFlag;});
//        while (!requestQueue_.CRQueue_.empty() && !stopFlag) {
//            std::cout<<"OB processing"<<std::endl;
//            auto item = requestQueue_.CRQueue_.front();
//            switch(item.getterNodeType()){
//                case insertionCR:
////                    check if insertion possible
//                    insertion(new Order(item.getterUserID(),
//                                        item.getterBoID(),
//                                        item.getterPrice(),
//                                        item.getterVolume(),
//                                        item.getterProductID(),
//                                        item.getterOrderDirection(),
//                                        item.getterOrderType()));
//                    break;
//                case deletionCR:
//                    //check if existing
//                    deletion(getterPointerToOrderFromID(item.getterBoID()));
//                    break;
//                case updateCR:
//                    // check if existing
//                    update(getterPointerToOrderFromID(item.getterBoID()),
//                            new Order(item.getterUserID(),
//                                     item.getterBoID(),
//                                     item.getterPrice(),
//                                     item.getterVolume(),
//                                     item.getterProductID(),
//                                     item.getterOrderDirection(),
//                                     item.getterOrderType()));
//                    break;
//                    //default throw error
//                case displayOrderBookCR:
//                    displayOrderBook();
//                    break;
//            }
//            requestQueue_.CRQueue_.pop();
//        }
//        if(stopFlag) break;
//    }
//}
