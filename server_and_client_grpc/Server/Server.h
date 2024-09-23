#ifndef ORDERBOOK_SERVER_H
#define ORDERBOOK_SERVER_H

#include "../ServiceImplementation.h"

// Function to run the server and initialize RpcService
void RunServer(Market *market, const std::string& serverAddress = "localhost:50051");

#endif //ORDERBOOK_SERVER_H
