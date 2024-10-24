#ifndef ORDERBOOK_ORDERLINKEDLIST_H
#define ORDERBOOK_ORDERLINKEDLIST_H

#include "order/Order.h"

// move constructor and assignment deleted by creation of user-defined destructor

// copy constructor and assignment deleted as not used and as they required copying dynamically allocated Orders
class OrderLinkedList {
private:
    Order* dummyTail_;
    Order* head_;

public:
    explicit OrderLinkedList(orderDirection bidsOrOffers);
    ~OrderLinkedList();

    OrderLinkedList(const OrderLinkedList& other) = delete;
    OrderLinkedList& operator=(const OrderLinkedList& other) = delete;

    [[nodiscard]] Order* getterTail() const { return dummyTail_->getterNextBO(); };
    [[nodiscard]] Order* getterHead() const { return head_; };
    void updateHead(Order* newHead) { head_ = newHead; };
};


#endif //ORDERBOOK_ORDERLINKEDLIST_H
