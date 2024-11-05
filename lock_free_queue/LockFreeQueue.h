#ifndef ORDERBOOK_LOCKFREEQUEUE_H
#define ORDERBOOK_LOCKFREEQUEUE_H

//#include "../server_and_client_zero_mq/Message/Message.h"
#include <memory>
#include <optional>
#include <atomic>

template<typename T>
class LockFreeQueue {
    class Node {
    public:
        std::shared_ptr<Node> next_;
        T data_;

        Node() : data_(), next_(nullptr) {}
        explicit Node(const T &data) : data_(data), next_(nullptr) {}
        Node(T &data, std::shared_ptr<Node> next) : data_(data), next_(next) {}
        Node(Node& other){
            data_ = other.data;
            next_ = other.next_;
        }
    };
    std::shared_ptr<Node> tail_;
    std::shared_ptr<Node> dummyHead_;

public:
    LockFreeQueue() : tail_(nullptr), dummyHead_ (std::make_shared<Node>()) {}
    void push(const T &data){
        auto insertedNode = std::make_shared<Node>(data);
        std::shared_ptr<Node> oldTail;
        std::atomic_store(&oldTail, tail_);
        while( !std::atomic_compare_exchange_weak(&tail_, &oldTail, insertedNode) ) {}

        if(oldTail) {
            oldTail->next_ = insertedNode;
        } else {
            std::atomic_store( &( dummyHead_->next_), insertedNode);
        }
    }
    std::optional<T> pop();
};



#endif //ORDERBOOK_LOCKFREEQUEUE_H
