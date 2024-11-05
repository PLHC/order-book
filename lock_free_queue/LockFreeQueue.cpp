#include "LockFreeQueue.h"


template<typename T>
std::optional<T> LockFreeQueue<T>::pop(){
    std::shared_ptr<Node> oldDummyHead;
    std::atomic_store(&oldDummyHead, dummyHead_);

    while(std::atomic_load( &( oldDummyHead->next_)) != nullptr
           && std::atomic_compare_exchange_weak(&dummyHead_, &oldDummyHead, oldDummyHead->next_)
          ){}

    if(oldDummyHead->next == nullptr ){
        return std::nullopt;
    }
    return oldDummyHead->next_->data_;
}
