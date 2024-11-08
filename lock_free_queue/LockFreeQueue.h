#ifndef ORDERBOOK_LOCKFREEQUEUE_H
#define ORDERBOOK_LOCKFREEQUEUE_H

#include <memory>
#include <optional>
#include <atomic>


template<typename T>
class LockFreeQueue {
    class Node {
    public:
        Node* next_;
        T data_;

        Node() : data_(), next_(nullptr) {}
        explicit Node(const T &data) : data_(data), next_(nullptr) {}
    };

    std::atomic<int32_t> size_;
    std::atomic<Node*> dummyHead_;
    std::atomic<Node*> tail_;

public:
    LockFreeQueue(){
        size_.store(0);
        dummyHead_.store(new Node());
        tail_.store(nullptr);
    }
    ~LockFreeQueue(){
        std::cout<<"in LFQ destructor\n";
        delete dummyHead_.load();
        std::cout<<"end LFQ destructor\n";
    }
    void push(const T &data);
    std::optional<T> pop();
    int32_t getterSize() {return size_.load();}
};

template<typename T>
std::optional<T> LockFreeQueue<T>::pop(){
    Node* oldDummyHead { dummyHead_.load() };
    while(true){
        if(dummyHead_.load()->next_ == nullptr){
            return std::nullopt;
        }
        if(dummyHead_.compare_exchange_weak(oldDummyHead, oldDummyHead->next_)){
            break;
        }
    }
    auto data { oldDummyHead->next_->data_ };
    delete oldDummyHead;
    size_--;
    return data;
}


template<typename T>
void LockFreeQueue<T>::push(const T &data) {
    auto insertedNode { new Node(data) };
    Node* oldTail { nullptr };
    while ( !tail_.compare_exchange_weak(oldTail, insertedNode) ) {}
    size_++;
    if (oldTail) {
        oldTail->next_ = insertedNode;
    } else {
        dummyHead_.load()->next_ = insertedNode;
    }
}

#endif //ORDERBOOK_LOCKFREEQUEUE_H
