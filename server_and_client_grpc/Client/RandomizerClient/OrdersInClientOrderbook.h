#ifndef ORDERBOOK_ORDERSINCLIENTORDERBOOK_H
#define ORDERBOOK_ORDERSINCLIENTORDERBOOK_H

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <random>

#include "../../../market/order/OrderClient.h"


class OrdersMonitoring{
    std::random_device rd_{};
    uint32_t maxNbOrders_;

protected:
    class OrdersInOrderbook{
        uint32_t nbBuyOrders_{0};
        uint32_t nbSellOrders_{0};
        std::atomic<bool> active_{true};

    public:
        std::unordered_map<std::string, int> internalIdToOrderMap_;
        std::vector<std::shared_ptr<OrderClient>> pointersToOrders_;
        std::vector<int> freeIndexes_;

        std::mutex internalIdToOrderMapMtx_{};
        std::condition_variable internalIdToOrderMapConditionVariable_{};

        explicit OrdersInOrderbook(uint32_t maxNbOrders);

        bool insertOrder(std::shared_ptr<OrderClient> & orderToInsert); 
        void deleteOrder(const std::string & internalID); 
        void updateOrder(const std::string & internalID,
                         const int64_t boID,
                         const double price,
                         const double volume,
                         const int32_t version);

        [[nodiscard]] uint32_t getterNbBuyOrders() const { return nbBuyOrders_; }
        [[nodiscard]] uint32_t getterNbSellOrders() const { return nbSellOrders_; }
        [[nodiscard]] bool getterActiveOrNot() const { return active_.load(); }

        void deactivateOrderbook(){ active_.store(false); }
        void activateOrderbook(){ active_.store(true); }
    };

    std::mt19937 mtGen_{ rd_() };
    std::mutex monitoringMapLock_{};
    std::unordered_map<std::string, std::shared_ptr<OrdersInOrderbook>> productToOrdersMap_{};

public:
    explicit OrdersMonitoring(uint32_t maxNbOrders): maxNbOrders_{ maxNbOrders }{}
    ~OrdersMonitoring(); 

    bool insertOrderInLocalMonitoring(std::shared_ptr<OrderClient> & orderToInsert); 
    bool deleteOrderInLocalMonitoring(const std::string & internalID, 
                                      const std::string & product);
    bool updateOrderInLocalMonitoring(const std::string &internalID, 
                                      const int64_t boID,
                                      const double price,
                                      const double volume,
                                      const int32_t version,
                                      const std::string & product);

    void addTradedProductOrderbook(const std::string & product); 
    void removeTradedProductOrderbook(const std::string & product); 

    [[nodiscard]] std::vector<std::string> extractListOfTradedProducts();
    [[nodiscard]] std::shared_ptr<OrdersInOrderbook> getterSharedPointerToOrderbook(const std::string &product); 
    [[nodiscard]] std::pair<uint32_t, uint32_t> getterBuyAndSellNbOrders(const std::string & product);
};

#endif //ORDERBOOK_ORDERSINCLIENTORDERBOOK_H
