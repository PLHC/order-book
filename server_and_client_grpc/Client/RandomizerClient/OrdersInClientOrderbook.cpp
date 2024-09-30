#include "OrdersInClientOrderbook.h"
OrdersMonitoring::OrdersMonitoring()
        : monitoringMapLock_(),
          monitoringMapLockConditionVariable_(),
          productToOrdersMap_(),
          rd_(),
          mtGen_(rd_()){}

OrdersMonitoring::~OrdersMonitoring(){
    for(const auto & [product, orderbookPtr] : productToOrdersMap_){
        orderbookPtr->deactivateOrderbook();
    }
}

bool OrdersMonitoring::insertOrderInLocalMonitoring(std::shared_ptr<OrderClient> & orderToInsert) {
    auto orderbookPtr = getterSharedPointerToOrderbook(orderToInsert->getterProductID());
    if(!orderbookPtr) {
        return false;
    }
    return orderbookPtr->insertOrder(orderToInsert);
}

bool OrdersMonitoring::deleteOrderInLocalMonitoring(const std::string & internalID, const std::string & product) {
    auto orderbookPtr = getterSharedPointerToOrderbook(product);
    if(!orderbookPtr) {
        return false;
    }

    std::unique_lock<std::mutex> orderbookLock(orderbookPtr->internalIdToOrderMapMtx_);
    orderbookPtr->internalIdToOrderMapConditionVariable_.wait(orderbookLock, [](){return true;});;

    orderbookPtr->deleteOrder(internalID);
    auto success = true; ///// NOT sure?

    orderbookLock.unlock();
    orderbookPtr->internalIdToOrderMapConditionVariable_.notify_all();

    return success;
}

bool OrdersMonitoring::updateOrderInLocalMonitoring(const std::string &internalID,
                                                    const uint64_t boID,
                                                    const double price,
                                                    const double volume,
                                                    const uint32_t version,
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

    auto success = true;

    orderbookLock.unlock();
    orderbookPtr->internalIdToOrderMapConditionVariable_.notify_all();

    return success;
}






//std::pair<bool, std::pair<std::string, uint64_t> > OrdersMonitoring::getterRandomOrder(const std::string &product) {
//    auto distribution = std::uniform_int_distribution<uint32_t> (0, 10000);
//    auto randomOrderNumber = distribution(mtGen_);
//    std::string orderInternalID;
//    uint64_t boID;
//
//    std::unique_lock<std::mutex> mapsLock (monitoringMapLock_);
//    monitoringMapLockConditionVariable_.wait(mapsLock, [](){return true;});
//    auto productIter = productToOrdersMap_.find(product);
//    if(productIter == end(productToOrdersMap_)){
//        return {false, {"", 0}};
//    }
//    auto orderbook = productIter->second;
//
//    std::unique_lock<std::mutex> orderbookLock (orderbook->internalIdToOrderMapMtx_);
//    orderbook->internalIdToOrderMapConditionVariable_.wait(mapsLock, [](){return true;});
//
//    mapsLock.unlock();
//    monitoringMapLockConditionVariable_.notify_all();
//
//    if(productIter != end(productToOrdersMap_)){
//        randomOrderNumber %= productIter->second->getterNbBuyOrders() + productIter->second->getterNbSellOrders();
//
//        for(const auto & [ID, orderPtr] : productIter->second->internalIdToOrderMap_){
//            if(!randomOrderNumber--){
//                orderInternalID = ID;
//                boID = orderPtr->getterBoID();
//            };
//        }
//    }
//
//    orderbookLock.unlock();
//    orderbook->internalIdToOrderMapConditionVariable_.notify_all();
//
//    return {true, {orderInternalID, boID} };
//}







void OrdersMonitoring::OrdersInOrderbook::updateOrder(const std::string &internalID,
                                                      const uint64_t boID,
                                                      const double price,
                                                      const double volume,
                                                      const uint32_t version) {
    if(!getterActiveOrNot()){
        return;
    }

    if(!volume){
        deleteOrder(internalID);
        return;
    }

    std::unique_lock<std::mutex> orderbookLock(internalIdToOrderMapMtx_);
    internalIdToOrderMapConditionVariable_.wait(orderbookLock, [](){return true;});

    auto orderIter = internalIdToOrderMap_.find(internalID);
    if(orderIter != end(internalIdToOrderMap_) && orderIter->second->getterVersion()<version) { // check if order exists
        orderIter->second->updatePrice(price);
        orderIter->second->updateVolume(volume);
        orderIter->second->updateBoID(boID);
        orderIter->second->updateVersion(version);
    }

    orderbookLock.unlock();
    internalIdToOrderMapConditionVariable_.notify_all();
}

bool OrdersMonitoring::OrdersInOrderbook::insertOrder(std::shared_ptr<OrderClient> & orderToInsert) {
    if(!getterActiveOrNot()){
        orderToInsert = nullptr;
        return false;
    }

    std::unique_lock<std::mutex> orderbookLock(internalIdToOrderMapMtx_);
    internalIdToOrderMapConditionVariable_.wait(orderbookLock, [](){return true;});

    if(orderToInsert->getterOrderDirection() == BUY) {
        nbBuyOrders_++;
    }else{
        nbSellOrders_++;
    }
    internalIdToOrderMap_[orderToInsert->getterInternalID()] = std::move(orderToInsert);

    orderbookLock.unlock();
    internalIdToOrderMapConditionVariable_.notify_all();
    return true;
}

void OrdersMonitoring::OrdersInOrderbook::deleteOrder(const std::string &internalID) {
    if(!getterActiveOrNot()){
        return;
    }

    std::unique_lock<std::mutex> orderbookLock(internalIdToOrderMapMtx_);
    internalIdToOrderMapConditionVariable_.wait(orderbookLock, [](){return true;});

    auto orderIter = internalIdToOrderMap_.find(internalID);
    if(orderIter != end(internalIdToOrderMap_)) { // check if order exists
        if(orderIter->second->getterOrderDirection()==BUY){
            nbBuyOrders_--;
        }else{
            nbSellOrders_--;
        }
        internalIdToOrderMap_.erase(internalID);
    }

    orderbookLock.unlock();
    internalIdToOrderMapConditionVariable_.notify_all();
}



uint32_t OrdersMonitoring::getterNbOrders(const std::string &product) {
    auto orderbookPtr = getterSharedPointerToOrderbook(product);
    if(!orderbookPtr) return 0;

    std::unique_lock<std::mutex> mapLock (orderbookPtr->internalIdToOrderMapMtx_);
    orderbookPtr->internalIdToOrderMapConditionVariable_.wait(mapLock, [](){return true;});

    auto nbOrders = orderbookPtr->getterNbSellOrders() + orderbookPtr->getterNbBuyOrders();

    mapLock.unlock();
    orderbookPtr->internalIdToOrderMapConditionVariable_.notify_all();

    return nbOrders;
}

void OrdersMonitoring::addTradedProductOrderbook(const std::string &product) {
    std::unique_lock<std::mutex> mapsLock (monitoringMapLock_);
    monitoringMapLockConditionVariable_.wait(mapsLock, [](){return true;});

    if(productToOrdersMap_.count(product)==0) { // if non existent
        productToOrdersMap_[product] = std::make_shared<OrdersInOrderbook>();
    }
    else{ //reactivate it if already existing
        productToOrdersMap_[product]->activateOrderbook();
    }

    mapsLock.unlock();
    monitoringMapLockConditionVariable_.notify_all();
}

void OrdersMonitoring::removeTradedProductOrderbook(const std::string &product) {
    std::unique_lock<std::mutex> mapsLock (monitoringMapLock_);
    monitoringMapLockConditionVariable_.wait(mapsLock, [](){return true;});

    productToOrdersMap_[product]->deactivateOrderbook();
    productToOrdersMap_.erase(product);

    mapsLock.unlock();
    monitoringMapLockConditionVariable_.notify_all();
}

std::vector<std::string> OrdersMonitoring::extractListOfTradedProducts() {
    std::vector<std::string> productsList;
    std::unique_lock<std::mutex> mapsLock (monitoringMapLock_);
    monitoringMapLockConditionVariable_.wait(mapsLock, [](){return true;});

    for(const auto & [product, orders]: productToOrdersMap_){
        productsList.push_back(product);
    }

    mapsLock.unlock();
    monitoringMapLockConditionVariable_.notify_all();
    return productsList;
}

std::shared_ptr<OrdersMonitoring::OrdersInOrderbook> OrdersMonitoring::getterSharedPointerToOrderbook(
        const std::string &product) {
    std::unique_lock<std::mutex> mapsLock (monitoringMapLock_);
    monitoringMapLockConditionVariable_.wait(mapsLock, [](){return true;});

    auto orderbookIter = productToOrdersMap_.find(product);
    if(orderbookIter==end(productToOrdersMap_)){
        return nullptr;
    }
    auto ptr = orderbookIter->second;

    mapsLock.unlock();
    monitoringMapLockConditionVariable_.notify_all();
    return std::move(ptr);
}
