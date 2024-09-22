#include "OrderLinkedList.h"

OrderLinkedList::OrderLinkedList(orderDirection bidsOrOffers){
    dummyHead_ = new Order(bidsOrOffers);
    tail_ = dummyHead_;
}

OrderLinkedList::~OrderLinkedList(){
    auto next_to_be_deleted = dummyHead_;
    while(next_to_be_deleted){
        dummyHead_ = next_to_be_deleted->getterNextBO();
        delete next_to_be_deleted;
        next_to_be_deleted = dummyHead_;
    }
}