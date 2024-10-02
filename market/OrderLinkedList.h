#ifndef ORDERBOOK_ORDERLINKEDLIST_H
#define ORDERBOOK_ORDERLINKEDLIST_H

#include "Order.h"

class OrderLinkedList {
private:
    Order* dummyTail_;
    Order* head_;

public:
    explicit OrderLinkedList(orderDirection bidsOrOffers);
    ~OrderLinkedList();

    OrderLinkedList(OrderLinkedList&& other) = delete;
    OrderLinkedList& operator=(const OrderLinkedList&& other) = delete;

    [[nodiscard]] inline Order* getterTail() const {return dummyTail_->getterNextBO();};
    [[nodiscard]] inline Order* getterHead() const {return head_;};
    inline void updateHead(Order* newHead) {head_ = newHead;};
};


#endif //ORDERBOOK_ORDERLINKEDLIST_H
