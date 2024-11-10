#include "OrderBook.h"

OrderBook::~OrderBook() {
    std::cout<<"OB destructor begins"<<std::endl;
    // need to update all elements of CRQ in order to terminate processingThread_
    // ignoring lock as no more node should be inserted at that point
    auto tail = requestQueue_.dummyTail_;
    while(tail){
        tail->status_ = CANCELLED;
        tail->statusConditionVariable_.notify_all();
        tail = tail->next_;
    }

    setterStopFlagToTrue();
    requestQueue_.prevLock_.unlock();

    if(processingThread_.joinable()){
        processingThread_.join();
    }
    std::cout<<"OB destructor ends"<<std::endl;
}

orderExecution OrderBook::checkExecution(Order* orderToBeChecked){
    auto volumeInHundredths = static_cast<int32_t>( orderToBeChecked->getterVolumeInHundredth() );
    auto priceInCents = orderToBeChecked->getterPriceInCents();
    auto& LinkedList = orderToBeChecked->getterOrderDirection() == BUY ? offers_ : bids_;
    auto nextOrder = LinkedList.getterTail();

    while(nextOrder && volumeInHundredths > 0 &&
          (orderToBeChecked->getterOrderDirection() == BUY ?
           nextOrder->getterPriceInCents() <= priceInCents : nextOrder->getterPriceInCents() >= priceInCents)){
        volumeInHundredths -= static_cast<int32_t>( nextOrder->getterVolumeInHundredth() );
        nextOrder = nextOrder->getterNextBO();
    }
    if(volumeInHundredths<=0) return FULL_EXECUTION;
    else if(volumeInHundredths<orderToBeChecked->getterVolumeInHundredth()) return PARTIAL_EXECUTION;
    else return NO_EXECUTION;
}

void OrderBook::deletion(Order* deletedOrder, communicate communicated) {
    if(deletedOrder->getterNextBO() != nullptr) {
        deletedOrder->getterNextBO()->updatePrevBO( deletedOrder->getterPrevBO() );
    }
    deletedOrder->getterPrevBO()->updateNextBO(deletedOrder->getterNextBO());
    auto& LinkedList = ( deletedOrder->getterOrderDirection() == BUY ) ? bids_ : offers_;
    if(LinkedList.getterHead() == deletedOrder) LinkedList.updateHead(deletedOrder->getterPrevBO());

    idToPointerMap_.erase(deletedOrder->getterBoID());
    if(communicated == COMMUNICATED){
        db_->pushNewDbInputOnQueue(DELETION, *deletedOrder);
    }
    delete deletedOrder;
}

bool OrderBook::insertion(Order* &newOrder, communicate communicated){
    auto execution = checkExecution(newOrder);
    newOrder->incrementAndReturnVersion();

    if(newOrder->getterOrderType() == FILL_OR_KILL && execution != FULL_EXECUTION){
        //record order as deleted and never filled in memory
        return false;
    }
    auto& LinkedList = ( newOrder->getterOrderDirection() == BUY )? bids_ : offers_;
    auto nextBO = LinkedList.getterTail();
    auto priceInCents = newOrder->getterPriceInCents();
    while(nextBO &&
        ( newOrder->getterOrderDirection() == BUY ?
                priceInCents<=nextBO->getterPriceInCents() : priceInCents>=nextBO->getterPriceInCents()))
    {
        nextBO = nextBO->getterNextBO();
    }

    if(nextBO==nullptr){
        newOrder->updatePrevBO( LinkedList.getterHead() );
        LinkedList.getterHead()->updateNextBO( newOrder );
        LinkedList.updateHead( newOrder );
    }
    else{
        auto prevBO = nextBO->getterPrevBO();
        newOrder->updatePrevBO( prevBO );
        newOrder->updateNextBO( nextBO );
        nextBO->updatePrevBO(   newOrder );
        prevBO->updateNextBO(   newOrder );
    }
    idToPointerMap_[newOrder->getterBoID()] = newOrder;
    if(communicated == COMMUNICATED) {
        db_->pushNewDbInputOnQueue(INSERTION, *newOrder);
    }
    if(execution != NO_EXECUTION) performExecution(newOrder);
    return true;
}

void OrderBook::performExecution(Order* & executingOrder) {
    auto& LinkedList = executingOrder->getterOrderDirection() == BUY ? offers_ : bids_;
    auto orderToBeUpdated = LinkedList.getterTail();
    Order* nextOrder;
    auto volumeInHundredths = executingOrder->getterVolumeInHundredth();
    auto priceInCents = executingOrder->getterPriceInCents();
    int32_t executedPriceInCents = 0;
    uint32_t executedVolumeInHundredth = 0;

    while(orderToBeUpdated && volumeInHundredths > 0 && (executingOrder->getterOrderDirection() == BUY ?
                orderToBeUpdated->getterPriceInCents() <= priceInCents :
                orderToBeUpdated->getterPriceInCents() >= priceInCents))
    {
        nextOrder = orderToBeUpdated->getterNextBO();
        if(volumeInHundredths >= orderToBeUpdated->getterVolumeInHundredth()) {
            executedVolumeInHundredth = orderToBeUpdated->getterVolumeInHundredth();
            volumeInHundredths -= executedVolumeInHundredth;
            executedPriceInCents = orderToBeUpdated->getterPriceInCents();

            orderToBeUpdated->incrementAndReturnVersion();
            orderToBeUpdated->updateVolume(0);
            db_->pushNewDbInputOnQueue(EXECUTION, *orderToBeUpdated, executedVolumeInHundredth, executedPriceInCents);

            deletion(orderToBeUpdated, NON_COMMUNICATED);
        }
        else{
            orderToBeUpdated->updateVolume(
                    ( orderToBeUpdated->getterVolumeInHundredth() - volumeInHundredths)/100.0 );
            executedVolumeInHundredth = volumeInHundredths;
            volumeInHundredths=0;

            orderToBeUpdated->incrementAndReturnVersion();
            executedPriceInCents = orderToBeUpdated->getterPriceInCents();
            db_->pushNewDbInputOnQueue( EXECUTION,
                                        *orderToBeUpdated,
                                        executedVolumeInHundredth,
                                        executedPriceInCents);
        }
        orderToBeUpdated = nextOrder;
    }
    executingOrder->updateVolume(volumeInHundredths/100.0);
    executingOrder->incrementAndReturnVersion();
    db_->pushNewDbInputOnQueue( EXECUTION,
                                *executingOrder,
                                executedVolumeInHundredth,
                                executedPriceInCents);
    if(volumeInHundredths==0){
        // order needs to be removed from the orderbook but is required to answer gRPC request, so a copy is made
        // update database for executing order
        auto copyOfExecutingOrder = new Order( executingOrder );
        deletion( executingOrder, NON_COMMUNICATED );
        executingOrder = copyOfExecutingOrder;
    }
}

bool OrderBook::update(Order* updatedOrder, Order* &newOrder){
    if( !updatedOrder->checkIfItHasAnOlderVersionThan( newOrder ) ){
        // older version detected, no update done
        return false;
    }
    newOrder->incrementAndReturnVersion();
    if(updatedOrder->checkIfOnlyVolumeUpdatedAndDownwards(newOrder)){
        // updated order does not lose its spot in the orders with same price, as new orders are always push last
        newOrder->updateNextBO( updatedOrder->getterNextBO() );
        newOrder->updatePrevBO( updatedOrder->getterPrevBO() );
        if(newOrder->getterNextBO()!=nullptr) {
            newOrder->getterNextBO()->updatePrevBO( newOrder );
        }
        newOrder->getterPrevBO()->updateNextBO(newOrder);
        idToPointerMap_[ newOrder->getterBoID() ] = newOrder;
        if( updatedOrder==bids_.getterHead() ) {
            bids_.updateHead( newOrder );
        } else if (updatedOrder==offers_.getterHead() ){
            offers_.updateHead( newOrder );
        }
        db_->pushNewDbInputOnQueue( UPDATE, *newOrder );
        delete updatedOrder;
    }else { // update database: newOrder is replacing updatedOrder
        db_->pushNewDbInputOnQueue( UPDATE, *newOrder );
        deletion( updatedOrder, NON_COMMUNICATED );
        insertion( newOrder, NON_COMMUNICATED );
    }
    return true;
}

std::string OrderBook::displayOrderBook(uint32_t nbOfOrdersToDisplay) {
    auto bidNode = bids_.getterTail();
    auto offerNode = offers_.getterTail();

    std::cout << std::fixed << std::setprecision(2);
    std::ostringstream oss;
    oss<<std::left<<std::setw(43)<<"Bids"<<"|"
        <<std::setw(40)<<"Offers"<<std::endl;
    oss<<std::left<<std::setw(10)<<"ID"<<"|"
        <<std::setw(10)<<"User"<<"|"
        <<std::setw(10)<<"Vol"<<"|"
        <<std::setw(10)<<"Price";
    oss<<"|"<<std::setw(10)<<"Price"<<"|"
        <<std::setw(10)<<"Vol"<<"|"
        <<std::setw(10)<<"User"<<"|"
        <<std::setw(10)<<"ID"<<std::endl;

    while((bidNode || offerNode) && nbOfOrdersToDisplay--){
        if(bidNode){
            oss<<std::left<<std::setw(10)<<bidNode->getterBoID()<<"|"
                    <<std::setw(10)<<bidNode->getterUserID()<<"|"
                    <<std::setw(10)<<bidNode->getterVolume()<<"|"
                    <<std::setw(10)<<bidNode->getterPrice()<<"|";
            bidNode = bidNode->getterNextBO();
        }
        else{
            oss<<std::left<<std::setw(10)<<""<<"|"
                <<std::setw(10)<<""<<"|"
                <<std::setw(10)<<""<<"|"
                <<std::setw(10)<<""<<"|";
        }
        if(offerNode){
            oss<<std::setw(10)<<offerNode->getterPrice()<<"|"
                    <<std::setw(10)<<offerNode->getterVolume()<<"|"
                    <<std::setw(10)<<offerNode->getterUserID()<<"|"
                    <<std::setw(10)<<offerNode->getterBoID()<<std::endl;
            offerNode = offerNode->getterNextBO();
        }
        else{
            oss<<std::setw(10)<<""<<"|"
                <<std::setw(10)<<""<<"|"
                <<std::setw(10)<<""<<"|"
                <<std::setw(10)<<""<<std::endl;
        }
    }
    oss<<std::endl;
    return std::move(oss).str(); // move the content of stream in the returned string
}

void OrderBook::processRequests(){
    while( !stopFlagOB_ ){
        requestQueue_.runNextRequest();
    }
}

Order *OrderBook::getterPointerToOrderFromID(uint64_t boID)  {
    if(idToPointerMap_.count( boID )) {
        return idToPointerMap_[ boID ];
    }
    return nullptr;
}


