# order-book

This project implements and simulates a trading platform service enabling traders to send and interact with  orders. 
The purpose of this project is to demonstrate the knowledge I acquired in the field of computer science and more particularly
about C++, concurrency and multi-threading.
As the project was written over months in parallel of studying the concepts, the implementation choices are heterogeneous 
between the main components. For this reason the market structure having been coded early relies on raw pointers
(including the required destructors) and locking concurrency mechanisms; whereas the lately implemented features use 
smart pointers and a [templated lock-free queue](lock_free_queue//LockFreeQueue.h).

# Market
## OOP Structure
The market is one entity offering access to different tradable products, each represented by a unique orderbook. In OOP,
a [Market](market/Market.h) object is made of a collection of [Orderbook](market/OrderBook.h) objects. 
Each orderbook consists of two [OrderLinkedList](market/OrderLinkedList.h) objects, which are linked lists of 
[Order](market/order/Order.h) objects. The system can be seen as a reverse pyramid in which the higher level modules do
not depend on lower level modules, thus it follows the **Dependency Inversion Principle** (**SOLID** principles). 

pyramid graph

Moreover the concerns are clearly separated between the classes, e.g. [Market](market/Market.h) dispatches the trader 
request to the [Orderbook](market/OrderBook.h), which handles its execution.
On the other hand, [OrderLinkedList](market/OrderLinkedList.h) and [Order](market/order/Order.h) are containers 
designed for this project.

## Asynchronous operation
As orders can be sent by different traders simultaneously, it becomes interesting to maximize parallel execution.
Fairness is guaranteed if priority in request handling is defined by order of arrival. 
As requests could overlap in their execution it is necessary to handle one request at a time on a single Orderbook, 
so operations are synchronously executed in each Orderbook. Similarly the dispatch performs by Market requires
to be made synchronously to maintain fairness. Concurrency is therefore limited to a thread per Orderbook and one more for
the Market. Finally a [Customer Request Queue](market/CustomerRequestQueue/CustomerRequestQueue.h) object is included in
every Orderbook and is used to store the dispatched requests. Using a lock per node in the queue, a single push and a single 
pop operations can occur concurrently.

## Single ID generator
To guarantee generated order IDs are unique, it is necessary to limit the project to a single ID Generator. For that 
purpose, [GeneratorID](market/GeneratorId.h) implementation follows the **Singleton** design principle. 


# gRPC implementation

# Mongo database
## Requirements
The database in this project is designed to be used for performance check or for reconciliation, e.g. back 
office invoicing. Every version of an order needs to be saved and therefore the database is mainly used for insertion and
rarely for reading. As the structure of the order is rigid, using a SQL could have been judged better but as I already
implemented one in my [Option Pricer App](https://github.com/PLHC/option-pricer-app), I decided to learn about NoSQL through
a Mongo database.
## Indexes
Two indexes were created:
- sorted by Order ID to be able to extract the latest max ID when initializing the [GeneratorID](market/GeneratorId.h)
- trades sorted by date to be used to reconcile.
## Interface using a lock-free queue
Once again the interface is limited to one entity and [DatabaseInterface](database/DatabaseInterface.h) follows the **Singleton**
design principle. To minimize the transaction cost of a database request, new inputs are combined in a bulk which is pushed only once.
Finally for the Orderbook to communicate the events, a [LockFreeQueue](lock_free_queue/LockFreeQueue.h) is included in 
the [DatabaseInterface](database/DatabaseInterface.h) in which several Orderbooks can push simultaneously while a single
thread is popping and bulking order events.

# Random order generator

# Display client

# CMake: project configuration

# Running the simulation



# Performance

# Further development
- communicating deals to concerned client 
- streaming tick data
- implementing market makers bots from the tick data


