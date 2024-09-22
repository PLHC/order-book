#ifndef ORDERBOOK_ORDERLINKEDLIST_H
#define ORDERBOOK_ORDERLINKEDLIST_H

#include "Order.h"

class OrderLinkedList {
private:
    Order* dummyHead_;
    Order* tail_;

public:
    explicit OrderLinkedList(orderDirection bidsOrOffers);
    ~OrderLinkedList();

    OrderLinkedList(OrderLinkedList&& other) = delete;
    OrderLinkedList& operator=(const OrderLinkedList&& other) = delete;

    [[nodiscard]] inline Order* getterHead() const {return dummyHead_->getterNextBO();};
    [[nodiscard]] inline Order* getterTail() const {return tail_;};
    inline void updateTail(Order* newTail) {tail_=newTail;};
};


#endif //ORDERBOOK_ORDERLINKEDLIST_H
