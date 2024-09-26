#ifndef ORDERBOOK_RUNNINGSERVER_H
#define ORDERBOOK_RUNNINGSERVER_H

#include "../Service/ServiceAsync.h"

// Function to run the server and initialize RpcServiceAsync
void RunServer(Market *market, const std::string& serverAddress = "localhost:50051");

#endif //ORDERBOOK_RUNNINGSERVER_H
