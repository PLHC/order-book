#ifndef ORDERBOOK_ORDERLINKEDLIST_H
#define ORDERBOOK_ORDERLINKEDLIST_H

#include "order/Order.h"


class OrderLinkedList {
private:
    Order* dummyTail_;
    Order* head_{ dummyTail_ };

public:
    explicit OrderLinkedList(orderDirection bidsOrOffers): dummyTail_(new Order(bidsOrOffers)) {}

    ~OrderLinkedList(){
        std::cout<<"OrderLinkedList destructor begins"<<std::endl;
        auto next_to_be_deleted = dummyTail_;
        while(next_to_be_deleted){
            dummyTail_ = next_to_be_deleted->getterNextBO();
            delete next_to_be_deleted;
            next_to_be_deleted = dummyTail_;
        }
        std::cout<<"OrderLinkedList destructor ends"<<std::endl;
    }
    // move constructor and assignment operator deleted by creation of user-defined destructor
    // copy constructor and assignment operator deleted because not used and
    // because they required copying dynamically allocated Orders
    OrderLinkedList(const OrderLinkedList& other) = delete;
    OrderLinkedList& operator=(const OrderLinkedList& other) = delete;

    [[nodiscard]] Order* getterTail() const { return dummyTail_->getterNextBO(); };
    [[nodiscard]] Order* getterHead() const { return head_; };
    void updateHead(Order* newHead) { head_ = newHead; };
};


#endif //ORDERBOOK_ORDERLINKEDLIST_H
