#include "OrdersInClientOrderbook.h"

bool OrdersInOrderbooks::ProductOrders::insertOrder(std::unique_ptr<OrderClient> orderToInsert) {
    if(orderToInsert->getterOrderDirection() == BUY) {
        if (internalIdToBuyOrderMap_.count(orderToInsert->getterInternalID())) {
            return false;
        }
        internalIdToBuyOrderMap_[orderToInsert->getterInternalID()] = std::move(orderToInsert);
        nbBuyOrders_++;
    }else{
        if (internalIdToSellOrderMap_.count(orderToInsert->getterInternalID())) {
            return false;
        }
        internalIdToSellOrderMap_[orderToInsert->getterInternalID()] = std::move(orderToInsert);
        nbSellOrders_++;
    }
    return true;
}

bool OrdersInOrderbooks::ProductOrders::deleteOrder(const std::string &internalID) {
    if(internalIdToBuyOrderMap_.count(internalID)) { // check if order exists
        internalIdToBuyOrderMap_.erase(internalID);
        nbBuyOrders_--;
        return true;
    }else if (internalIdToSellOrderMap_.count(internalID)){ // check if order exists
        internalIdToSellOrderMap_.erase(internalID);
        nbSellOrders_--;
        return true;
    }
    return false;
}

bool OrdersInOrderbooks::ProductOrders::updateOrder(std::unique_ptr<OrderClient> orderToInsert) {
    if(orderToInsert->getterOrderDirection() == BUY) {
        if (internalIdToBuyOrderMap_.count(orderToInsert->getterInternalID())) {
            return false;
        }
        internalIdToBuyOrderMap_[orderToInsert->getterInternalID()] = std::move(orderToInsert);
    }else{
        if (internalIdToSellOrderMap_.count(orderToInsert->getterInternalID())) {
            return false;
        }
        internalIdToSellOrderMap_[orderToInsert->getterInternalID()] = std::move(orderToInsert);
    }
    return true;
}



OrdersInOrderbooks::OrdersInOrderbooks()
    : productToOrdersMapLock_(),
      productToOrdersMapLockConditionVariable_(),
      productToOrdersMap_(),
      rd(),
      gen(rd()),
      distributionForRandomSelection(0, 10000){}

OrdersInOrderbooks::~OrdersInOrderbooks() {
    for(const auto & [internalID, ptr]: productToOrdersMap_){
        delete ptr; // delete ProductOrders object before this hashmap is destructed
    }
    productToOrdersMap_.clear();
}

bool OrdersInOrderbooks::insertOrderInLocalMonitoring(std::unique_ptr<OrderClient> & orderToInsert) {
    bool success = false;

    std::unique_lock<std::mutex> mapsLock (productToOrdersMapLock_);
    productToOrdersMapLockConditionVariable_.wait(mapsLock, [](){return true;});
    auto productIter = productToOrdersMap_.find(orderToInsert->getterProductID());
    if(productIter != end(productToOrdersMap_)) {
        productIter->second->insertOrder(std::move(orderToInsert));
        success = true;
    }

    mapsLock.unlock();
    productToOrdersMapLockConditionVariable_.notify_all();
    return success;
}

bool OrdersInOrderbooks::deleteOrderInLocalMonitoring(const std::string & internalID, const std::string & product) {
    bool success = false;

    std::unique_lock<std::mutex> mapsLock (productToOrdersMapLock_);
    productToOrdersMapLockConditionVariable_.wait(mapsLock, [](){return true;});

    auto productIter = productToOrdersMap_.find(product);
    if(productIter != end(productToOrdersMap_)) {
        productIter->second->deleteOrder(internalID);
        success = true;
    }

    mapsLock.unlock();
    productToOrdersMapLockConditionVariable_.notify_all();
    return success;
}

bool OrdersInOrderbooks::updateOrderInLocalMonitoring(std::unique_ptr<OrderClient> & orderToInsert) {
    bool success = false;

    std::unique_lock<std::mutex> mapsLock (productToOrdersMapLock_);
    productToOrdersMapLockConditionVariable_.wait(mapsLock, [](){return true;});

    auto productIter = productToOrdersMap_.find(orderToInsert->getterProductID());
    if(productIter != end(productToOrdersMap_)) {
        productIter->second->updateOrder(std::move(orderToInsert));
        success = true;
    }

    mapsLock.unlock();
    productToOrdersMapLockConditionVariable_.notify_all();
    return success;
}

std::pair< bool, std::pair<int, int> > OrdersInOrderbooks::getterNbOrders(const std::string &product) {
    std::pair<int, int> values = {0, 0};
    bool success= false;

    std::unique_lock<std::mutex> mapsLock (productToOrdersMapLock_);
    productToOrdersMapLockConditionVariable_.wait(mapsLock, [](){return true;});

    auto productIter = productToOrdersMap_.find(product);
    if(productIter != end(productToOrdersMap_) ) {
        values ={productIter->second->nbBuyOrders_, productIter->second->nbSellOrders_};
        success = true;
    }

    mapsLock.unlock();
    productToOrdersMapLockConditionVariable_.notify_all();
    return {success, values};
}

std::pair<bool, std::string> OrdersInOrderbooks::getterRandomOrder(const orderDirection buyOrSell, const std::string &product) {
    auto randomOrderNumber = distributionForRandomSelection(gen);
    bool success = false;
    std::string orderInternalID;

    std::unique_lock<std::mutex> mapsLock (productToOrdersMapLock_);
    productToOrdersMapLockConditionVariable_.wait(mapsLock, [](){return true;});
    auto productIter = productToOrdersMap_.find(product);
    if(productIter != end(productToOrdersMap_)){
        success = true;

        if(buyOrSell==BUY){
            randomOrderNumber %= productIter->second->nbBuyOrders_;
            for(const auto & [ID, orderPtr] : productIter->second->internalIdToBuyOrderMap_){
                if(!randomOrderNumber--){
                    orderInternalID = ID;
                };
            }
        }else{
            randomOrderNumber %= productIter->second->nbSellOrders_;
            for(const auto & [ID, orderPtr] : productIter->second->internalIdToSellOrderMap_){
                if(!randomOrderNumber--){
                    orderInternalID = ID;
                };
            }
        }
    }

    mapsLock.unlock();
    productToOrdersMapLockConditionVariable_.notify_all();
    return {success, orderInternalID};
}

void OrdersInOrderbooks::addTradedProduct(const std::string &product) {
    std::unique_lock<std::mutex> mapsLock (productToOrdersMapLock_);
    productToOrdersMapLockConditionVariable_.wait(mapsLock, [](){return true;});

    productToOrdersMap_[product] = new ProductOrders();

    mapsLock.unlock();
    productToOrdersMapLockConditionVariable_.notify_all();
}

void OrdersInOrderbooks::removeTradedProduct(const std::string &product) {
    std::unique_lock<std::mutex> mapsLock (productToOrdersMapLock_);
    productToOrdersMapLockConditionVariable_.wait(mapsLock, [](){return true;});

    productToOrdersMap_.erase(product);

    mapsLock.unlock();
    productToOrdersMapLockConditionVariable_.notify_all();
}

std::vector<std::string> OrdersInOrderbooks::extractListOfTradedProducts() {
    std::vector<std::string> productsList;
    std::unique_lock<std::mutex> mapsLock (productToOrdersMapLock_);
    productToOrdersMapLockConditionVariable_.wait(mapsLock, [](){return true;});

    for(const auto & [product, orders]: productToOrdersMap_){
        productsList.push_back(product);
    }

    mapsLock.unlock();
    productToOrdersMapLockConditionVariable_.notify_all();
    return productsList;
}
