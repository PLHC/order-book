cmake_minimum_required(VERSION 3.28)
project(OrderBook)

set(CMAKE_CXX_STANDARD 17)

add_executable(OrderBook main.cpp
        Order.cpp
        Order.h
        OrderBook.cpp
        OrderBook.h
        OrderLinkedList.cpp
        OrderLinkedList.h
        Market.cpp
        Market.h
        Bot.cpp
        Bot.h
        CustomerRequestQueue.cpp
        CustomerRequestQueue.h)
