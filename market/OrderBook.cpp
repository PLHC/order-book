#include "OrderBook.h"
#include <vector>

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
    if(deletedOrder->getterNextBO() != nullptr) {
        deletedOrder->getterNextBO()->updatePrevBO(deletedOrder->getterPrevBO());
    }
    deletedOrder->getterPrevBO()->updateNextBO(deletedOrder->getterNextBO());
    auto LinkedList = (deletedOrder->getterOrderDirection() == BUY) ? &bids_ : &offers_;
    if(LinkedList->getterHead() == deletedOrder) LinkedList->updateHead(deletedOrder->getterPrevBO());
    idToPointerMap_.erase(deletedOrder->getterBoID());
    delete deletedOrder;
    // update database about deleted order
}

bool OrderBook::insertion(Order* &newOrder){
    std::vector<double> bos;
    auto d = newOrder->getterOrderDirection()==BUY? "buy":"sell";
    std::cout<<"inserting: "<<newOrder->getterBoID()<<" "<<newOrder->getterPrice()<<" "<<d<<std::endl; /////


    auto execution = checkExecution(newOrder);
    newOrder->incrementAndReturnVersion();

    if(newOrder->getterOrderType() == FILL_OR_KILL && execution != FULL_EXECUTION){
        //record order as deleted and never filled in memory
        return false;
    }
    auto LinkedList = (newOrder->getterOrderDirection() == BUY)? &bids_ : &offers_;
    auto nextBO = LinkedList->getterTail();
    auto priceInCents = newOrder->getterPriceInCents();
    while(nextBO &&
        (newOrder->getterOrderDirection() == BUY ? priceInCents<=nextBO->getterPriceInCents() : priceInCents>=nextBO->getterPriceInCents())){


        bos.push_back(nextBO->getterPrice()); ////


        nextBO = nextBO->getterNextBO();
    }



    std::cout<<"tail list is: ";
    for(const auto bo : bos) std::cout<<bo<<" "; //////
    std::cout<<std::endl;




    if(nextBO==nullptr){
        std::cout<<"insert at the end"<<std::endl;
        std::cout<<"prevBO has a price of: "<<LinkedList->getterHead()->getterPrice()<<std::endl;
        newOrder->updatePrevBO(LinkedList->getterHead());
        LinkedList->getterHead()->updateNextBO(newOrder);
        LinkedList->updateHead(newOrder);

    }
    else{
        std::cout<<"insert in the middle"<<std::endl;
        auto prevBO = nextBO->getterPrevBO();
        newOrder->updatePrevBO(prevBO);
        newOrder->updateNextBO(nextBO);
        nextBO->updatePrevBO(newOrder);
        prevBO->updateNextBO(newOrder);
        std::cout<<"prevBO has a price of: "<<prevBO->getterPrice()<<std::endl;

    }
    std::cout<<"head price: "<<LinkedList->getterHead()->getterPrice()<< " volume: "<<LinkedList->getterHead()->getterVolume()<<std::endl;
    idToPointerMap_[newOrder->getterBoID()] = newOrder;
    //record order in memory

    std::cout<<displayOrderBook()<<std::endl;

    if(execution != NO_EXECUTION) performExecution(newOrder);


    return true;
}

void OrderBook::performExecution(Order* & executingOrder) {
    std::cout<<"executing order "<<executingOrder->getterBoID()<<"  "<<executingOrder->getterPrice()<<std::endl;
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
    executingOrder->updateVolume(volumeInHundredths/100.0);
    if(volumeInHundredths==0){
        // order needs to be removed from the orderbook but is required to answer gRPC request, so a copy is made
        // update database for executing order
        auto copyOfExecutingOrder = new Order(executingOrder);
        deletion(executingOrder);
        executingOrder = copyOfExecutingOrder;
    }
    std::cout<<displayOrderBook()<<std::endl;
}

bool OrderBook::update(Order* updatedOrder, Order* &newOrder){
    if(!updatedOrder->checkIfItHasAnOlderVersionThan(newOrder)){
        std::cout<<"older version detected, no update done"<<std::endl;
        return false;
    }


    auto d = updatedOrder->getterOrderDirection()==BUY? "buy":"sell";
    std::cout<<"updating: "<<updatedOrder->getterBoID()<<" p: "<<updatedOrder->getterPrice()<<"  v: "<<updatedOrder->getterVolume()<<" "<<d<<"    ";
    d = newOrder->getterOrderDirection()==BUY? "buy":"sell";
    std::cout<<"by: "<<newOrder->getterBoID()<<" p: "<<newOrder->getterPrice()<<"  v: "<<newOrder->getterVolume()<<" "<<d<<std::endl;



    if(updatedOrder->checkIfOnlyVolumeUpdatedAndDown(newOrder)){
        // updated order does not lose its spot in the orders with same price, as new orders are always push last
        // update database with newOrder and update updatedOrder to say replaced by newOrder
        newOrder->updateNextBO(updatedOrder->getterNextBO());
        newOrder->updatePrevBO(updatedOrder->getterPrevBO());
//        std::cout<<(newOrder->getterNextBO()==nullptr)<<std::endl;
//        std::cout<<(newOrder->getterPrevBO()==nullptr)<<std::endl;
        if(newOrder->getterNextBO()!=nullptr) {
            newOrder->getterNextBO()->updatePrevBO(newOrder);
        }
        newOrder->getterPrevBO()->updateNextBO(newOrder);
        newOrder->incrementAndReturnVersion();
        idToPointerMap_[newOrder->getterBoID()] = newOrder;
        idToPointerMap_.erase(updatedOrder->getterBoID());
        if(updatedOrder==bids_.getterHead() ) {
            bids_.updateHead(newOrder);
        } else if (updatedOrder==offers_.getterHead() ){
            offers_.updateHead(newOrder);
        }
        delete updatedOrder;
        std::cout<<displayOrderBook()<<std::endl;

    }else { // update database: newOrder is replacing updatedOrder
        deletion(updatedOrder);
        insertion(newOrder);
    }
    return true;
}

std::string OrderBook::displayOrderBook() {
    auto bidNode = bids_.getterTail();
    auto offerNode = offers_.getterTail();

    std::cout << std::fixed << std::setprecision(2);
    std::ostringstream oss;
//    system("clear");
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

Order *OrderBook::getterPointerToOrderFromID(uint64_t boID)  {
    if(idToPointerMap_.count(boID)) {
        return idToPointerMap_[boID];
    }
    return nullptr;
}
