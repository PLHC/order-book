#ifndef ORDERBOOK_ORDERSINCLIENTORDERBOOK_H
#define ORDERBOOK_ORDERSINCLIENTORDERBOOK_H

#include <iostream>

#include "../../market/Order.h"
#include <unordered_map>
#include <mutex>
#include <random>


class OrdersInOrderbooks{
    class ProductOrders{
    public:
        std::unordered_map<std::string, std::unique_ptr<OrderClient> > internalIdToBuyOrderMap_; // internal ID to order
        std::unordered_map<std::string, std::unique_ptr<OrderClient> > internalIdToSellOrderMap_;
        int nbBuyOrders_;
        int nbSellOrders_;

        ProductOrders(): nbSellOrders_(0), nbBuyOrders_(0){}

        bool insertOrder(std::unique_ptr<OrderClient> orderToInsert);
        bool deleteOrder(const std::string & internalID);
        bool updateOrder(std::unique_ptr<OrderClient> orderToInsert);

    };

    std::mutex productToOrdersMapLock_;
    std::condition_variable productToOrdersMapLockConditionVariable_;
    std::unordered_map<std::string, ProductOrders*> productToOrdersMap_;
    std::random_device rd;
    std::uniform_real_distribution<int> distributionForRandomSelection;
//    std::uniform_real_distribution<int> distributionForRandomDeletion;
//    enum insertOrDelete {INSERT, DELETE};
protected:
    std::mt19937 gen;

public:
    OrdersInOrderbooks();
    ~OrdersInOrderbooks();

    bool insertOrderInLocalMonitoring(std::unique_ptr<OrderClient> & orderToInsert);
    bool deleteOrderInLocalMonitoring(const std::string & internalID, const std::string & product);
    bool updateOrderInLocalMonitoring(std::unique_ptr<OrderClient> & orderToInsert);
    void addTradedProduct(const std::string & product);
    void removeTradedProduct(const std::string & product);
    std::vector<std::string> extractListOfTradedProducts();

    [[nodiscard]] std::pair< bool, std::pair<int, int> > getterNbOrders(const std::string & product);
    [[nodiscard]] std::pair<bool, std::string> getterRandomOrder(const orderDirection buyOrSell,
                                                                 const std::string & product);
//    return a specific Order
};

#endif //ORDERBOOK_ORDERSINCLIENTORDERBOOK_H
