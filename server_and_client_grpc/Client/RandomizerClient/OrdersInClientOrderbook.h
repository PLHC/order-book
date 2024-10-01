#ifndef ORDERBOOK_ORDERSINCLIENTORDERBOOK_H
#define ORDERBOOK_ORDERSINCLIENTORDERBOOK_H

#include <iostream>

#include "OrderClient.h"
#include <unordered_map>
#include <mutex>
#include <random>


class OrdersMonitoring{
    std::random_device rd_;

protected:
    class OrdersInOrderbook{
        uint32_t nbBuyOrders_;
        uint32_t nbSellOrders_;
        bool active_;

    public:
        std::unordered_map<std::string, std::shared_ptr<OrderClient> > internalIdToOrderMap_;
        std::mutex internalIdToOrderMapMtx_;
        std::condition_variable internalIdToOrderMapConditionVariable_;

        OrdersInOrderbook(): nbSellOrders_(0), nbBuyOrders_(0), active_(true){} ////

        bool insertOrder(std::shared_ptr<OrderClient> & orderToInsert); ////
        void deleteOrder(const std::string & internalID); ////
        void updateOrder(const std::string &internalID,  ////
                         const uint64_t boID,
                         const double price,
                         const double volume,
                         const uint32_t version);

        [[nodiscard]] inline uint32_t getterNbBuyOrders() const {return nbBuyOrders_;}; ////
        [[nodiscard]] inline uint32_t getterNbSellOrders() const {return nbSellOrders_;}; ////

        [[nodiscard]] inline bool getterActiveOrNot() const {return active_;}; ////
        inline void deactivateOrderbook(){active_ = false;}; ////
        inline void activateOrderbook(){active_ = true;}; ////
    };

    std::mt19937 mtGen_;
    std::mutex monitoringMapLock_;
    std::condition_variable monitoringMapLockConditionVariable_;
    std::unordered_map<std::string, std::shared_ptr<OrdersInOrderbook>> productToOrdersMap_;

public:
    OrdersMonitoring(); ////
    ~OrdersMonitoring(); ////

    bool insertOrderInLocalMonitoring(std::shared_ptr<OrderClient> & orderToInsert); ////
    bool deleteOrderInLocalMonitoring(const std::string & internalID, ////
                                      const std::string & product);
    bool updateOrderInLocalMonitoring(const std::string &internalID, ////
                                      const uint64_t boID,
                                      const double price,
                                      const double volume,
                                      const uint32_t version,
                                      const std::string & product);

    void addTradedProductOrderbook(const std::string & product); ////
    void removeTradedProductOrderbook(const std::string & product); ////

    [[nodiscard]] std::vector<std::string> extractListOfTradedProducts(); ////
    [[nodiscard]] std::shared_ptr<OrdersInOrderbook> getterSharedPointerToOrderbook(const std::string &product); ////
    [[nodiscard]] std::pair<int64_t, int64_t> getterBuyAndSellNbOrders(const std::string & product); ////
};

#endif //ORDERBOOK_ORDERSINCLIENTORDERBOOK_H
