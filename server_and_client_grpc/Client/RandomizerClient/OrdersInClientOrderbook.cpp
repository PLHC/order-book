#include "OrdersInClientOrderbook.h"

// OrdersMonitoring class
OrdersMonitoring::OrdersMonitoring()
        : monitoringMapLock_(),
          productToOrdersMap_(),
          rd_(),
          mtGen_(rd_()){}

OrdersMonitoring::~OrdersMonitoring(){
    std::cout<<"in OrdersMonitoring destructor"<<std::endl;
    for(const auto & [product, orderbookPtr] : productToOrdersMap_){
        orderbookPtr->deactivateOrderbook();
    }
    std::cout<<"in OrdersMonitoring destructor, done"<<std::endl;
}

bool OrdersMonitoring::insertOrderInLocalMonitoring(std::shared_ptr<OrderClient> & orderToInsert) {
    auto orderbookPtr = getterSharedPointerToOrderbook(orderToInsert->getterProductID());
    if(!orderbookPtr) {
        std::cout<<"insertOrderInLocalMonitoring cannot find OB: "<<orderToInsert->getterProductID()<<std::endl;
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

std::pair<uint32_t, uint32_t>  OrdersMonitoring::getterBuyAndSellNbOrders(const std::string & product) {
    auto orderbookPtr = getterSharedPointerToOrderbook(product);
    if(!orderbookPtr) return {-1, -1};

    std::unique_lock<std::mutex> mapLock (orderbookPtr->internalIdToOrderMapMtx_);
    orderbookPtr->internalIdToOrderMapConditionVariable_.wait(mapLock, [](){return true;});

    std::pair<uint32_t, uint32_t> nbOrders = {orderbookPtr->getterNbBuyOrders(), orderbookPtr->getterNbSellOrders()};

    mapLock.unlock();
    orderbookPtr->internalIdToOrderMapConditionVariable_.notify_all();

    return nbOrders;
}

void OrdersMonitoring::addTradedProductOrderbook(const std::string &product) {
    std::unique_lock<std::mutex> mapsLock (monitoringMapLock_);

    if(productToOrdersMap_.count(product)==0) { // if non existent
        productToOrdersMap_[product] = std::make_shared<OrdersInOrderbook>();
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
    std::unique_lock<std::mutex> mapsLock (monitoringMapLock_);

    for(const auto & [product, orders]: productToOrdersMap_){
        productsList.push_back(product);
    }

    mapsLock.unlock();
    return productsList;
}

std::shared_ptr<OrdersMonitoring::OrdersInOrderbook> OrdersMonitoring::getterSharedPointerToOrderbook(
        const std::string &product) {
    std::unique_lock<std::mutex> mapsLock (monitoringMapLock_);

    auto orderbookIter = productToOrdersMap_.find(product);
    if(orderbookIter==end(productToOrdersMap_)){
        std::cout<<"did not find the OB: "<<product<<std::endl;
        return nullptr;
    }
    auto ptr = orderbookIter->second;

    mapsLock.unlock();
    return std::move(ptr);
}


// OrdersInOrderBook class
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
    // check if order exists and if version is older than new one
    if(orderIter != end(internalIdToOrderMap_) && orderIter->second->getterVersion()<version) {
//        std::cout<<"updating "<<boID<<std::endl;
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