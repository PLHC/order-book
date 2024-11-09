#include "OrdersInClientOrderbook.h"

// OrdersMonitoring class
OrdersMonitoring::~OrdersMonitoring(){
    std::cout<<"in OrdersMonitoring destructor"<<std::endl;
    for(const auto & [product, orderbookPtr] : productToOrdersMap_){
        orderbookPtr->deactivateOrderbook();
    }
    std::cout<<"in OrdersMonitoring destructor, done"<<std::endl;
}

bool OrdersMonitoring::insertOrderInLocalMonitoring(std::shared_ptr<OrderClient> & orderToInsert) {
    auto orderbookPtr = getterSharedPointerToOrderbook( orderToInsert->getterProductID() );
    if( !orderbookPtr ) {
        std::cout<<"insertOrderInLocalMonitoring cannot find OB: "<<orderToInsert->getterProductID()<<std::endl;
        return false;
    }
    return orderbookPtr->insertOrder( orderToInsert );
}

bool OrdersMonitoring::deleteOrderInLocalMonitoring(const std::string & internalID, const std::string & product) {
    auto orderbookPtr = getterSharedPointerToOrderbook( product );
    if( !orderbookPtr ) {
        return false;
    }

    std::unique_lock<std::mutex> orderbookLock( orderbookPtr->internalIdToOrderMapMtx_ );
    orderbookPtr->internalIdToOrderMapConditionVariable_.wait(orderbookLock, [](){ return true; });;

    orderbookPtr->deleteOrder( internalID );

    orderbookLock.unlock();
    orderbookPtr->internalIdToOrderMapConditionVariable_.notify_all();

    return true;
}

bool OrdersMonitoring::updateOrderInLocalMonitoring(const std::string &internalID,
                                                    const int64_t boID,
                                                    const double price,
                                                    const double volume,
                                                    const int32_t version,
                                                    const std::string & product) {
    auto orderbookPtr = getterSharedPointerToOrderbook(product);
    if(!orderbookPtr) {
        return false;
    }

    std::unique_lock<std::mutex> orderbookLock(orderbookPtr->internalIdToOrderMapMtx_);
    orderbookPtr->internalIdToOrderMapConditionVariable_.wait(orderbookLock, [](){return true;});

    orderbookPtr->updateOrder(internalID,
                              boID,
                              price,
                              volume,
                              version);

    orderbookLock.unlock();
    orderbookPtr->internalIdToOrderMapConditionVariable_.notify_all();

    return true;
}

std::pair<uint32_t, uint32_t>  OrdersMonitoring::getterBuyAndSellNbOrders(const std::string & product) {
    auto orderbookPtr = getterSharedPointerToOrderbook( product );
    if(!orderbookPtr) return { -1, -1 };

    std::unique_lock<std::mutex> mapLock { orderbookPtr->internalIdToOrderMapMtx_ };
    orderbookPtr->internalIdToOrderMapConditionVariable_.wait(mapLock, [](){ return true; });

    std::pair<uint32_t, uint32_t> nbOrders = { orderbookPtr->getterNbBuyOrders(), orderbookPtr->getterNbSellOrders() };

    mapLock.unlock();
    orderbookPtr->internalIdToOrderMapConditionVariable_.notify_all();

    return nbOrders;
}

void OrdersMonitoring::addTradedProductOrderbook(const std::string &product) {
    std::unique_lock<std::mutex> mapsLock { monitoringMapLock_ };

    if( productToOrdersMap_.count(product)==0 ) { // if non existent
        productToOrdersMap_[product] = std::make_shared<OrdersInOrderbook>( maxNbOrders_ );
    }
    else{ //reactivate it if already existing
        productToOrdersMap_[product]->activateOrderbook();
    }

    mapsLock.unlock();
}

void OrdersMonitoring::removeTradedProductOrderbook(const std::string &product) {
    std::unique_lock<std::mutex> mapsLock (monitoringMapLock_);

    productToOrdersMap_[product]->deactivateOrderbook();
    productToOrdersMap_.erase(product);

    mapsLock.unlock();
}

std::vector<std::string> OrdersMonitoring::extractListOfTradedProducts() {
    std::vector<std::string> productsList;

    std::unique_lock<std::mutex> mapsLock { monitoringMapLock_ };

    productsList.reserve( productToOrdersMap_.size() );
    for( const auto & [ product, orders]: productToOrdersMap_ ){
        productsList.push_back( product );
    }

    mapsLock.unlock();
    return productsList; // compiler should optimize the return by value
}

std::shared_ptr<OrdersMonitoring::OrdersInOrderbook>
        OrdersMonitoring::getterSharedPointerToOrderbook(const std::string &product) {
    std::unique_lock<std::mutex> mapsLock { monitoringMapLock_ };

    auto orderbookIter = productToOrdersMap_.find( product );
    if(orderbookIter==end( productToOrdersMap_ )){
        std::cout<<"did not find the OB: "<<product<<std::endl;
        return nullptr;
    }
    auto ptr = orderbookIter->second;

    mapsLock.unlock();
    return std::move(ptr);
}


// OrdersInOrderBook class
OrdersMonitoring::OrdersInOrderbook::OrdersInOrderbook(uint32_t maxNbOrders){
    pointersToOrders_.resize( 2 * maxNbOrders );
    freeIndexes_.resize( 2 * maxNbOrders );
    std::iota( begin(freeIndexes_), end(freeIndexes_), 0 );
}

void OrdersMonitoring::OrdersInOrderbook::updateOrder(const std::string & internalID,
                                                      const int64_t boID,
                                                      const double price,
                                                      const double volume,
                                                      const int32_t version){
    if(!getterActiveOrNot()){
        return;
    }

    if(volume==0){
        deleteOrder( internalID );
        return;
    }

    std::unique_lock<std::mutex> orderbookLock{ internalIdToOrderMapMtx_ };
    internalIdToOrderMapConditionVariable_.wait( orderbookLock, [](){ return true; } );
    auto orderIter = internalIdToOrderMap_.find( internalID );
    // check if order exists and if version is older than new one
    if( orderIter != end(internalIdToOrderMap_) && pointersToOrders_[orderIter->second]->getterVersion()<version ) {
        pointersToOrders_[orderIter->second]->updatePrice(      price);
        pointersToOrders_[orderIter->second]->updateVolume(     volume);
        pointersToOrders_[orderIter->second]->updateBoID(       boID);
        pointersToOrders_[orderIter->second]->updateVersion(    version);
    }

    orderbookLock.unlock();
    internalIdToOrderMapConditionVariable_.notify_all();
}

bool OrdersMonitoring::OrdersInOrderbook::insertOrder(std::shared_ptr<OrderClient> & orderToInsert) {
    if( !getterActiveOrNot() ){
        orderToInsert = nullptr;
        return false;
    }

    std::unique_lock<std::mutex> orderbookLock{ internalIdToOrderMapMtx_ };
    internalIdToOrderMapConditionVariable_.wait(orderbookLock, [](){return true;});

    if( orderToInsert->getterOrderDirection() == BUY ) {
        nbBuyOrders_++;
    }else{
        nbSellOrders_++;
    }
    internalIdToOrderMap_[orderToInsert->getterInternalID()] = freeIndexes_.back();
    pointersToOrders_[freeIndexes_.back()] = std::move( orderToInsert );
    freeIndexes_.pop_back();

    orderbookLock.unlock();
    internalIdToOrderMapConditionVariable_.notify_all();
    return true;
}

void OrdersMonitoring::OrdersInOrderbook::deleteOrder(const std::string & internalID) {
    if(!getterActiveOrNot()){
        return;
    }
    std::unique_lock<std::mutex> orderbookLock{ internalIdToOrderMapMtx_ };
    internalIdToOrderMapConditionVariable_.wait( orderbookLock, [](){ return true; } );

    auto orderIter = internalIdToOrderMap_.find(internalID);
    if( orderIter != end(internalIdToOrderMap_) ) { // check if order exists

        if( pointersToOrders_[orderIter->second]->getterOrderDirection()==BUY ){
            nbBuyOrders_--;
        }else{
            nbSellOrders_--;
        }
        pointersToOrders_[orderIter->second] = nullptr;
        freeIndexes_.push_back( orderIter->second );
        internalIdToOrderMap_.erase( internalID );
    }

    orderbookLock.unlock();
    internalIdToOrderMapConditionVariable_.notify_all();
}