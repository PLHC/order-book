#include "OrderLinkedList.h"

OrderLinkedList::OrderLinkedList(orderDirection bidsOrOffers){
    dummyTail_ = new Order(bidsOrOffers);
    head_ = dummyTail_;
}

OrderLinkedList::~OrderLinkedList(){
    auto next_to_be_deleted = dummyTail_;
    while(next_to_be_deleted){
        dummyTail_ = next_to_be_deleted->getterNextBO();
        delete next_to_be_deleted;
        next_to_be_deleted = dummyTail_;
    }
}