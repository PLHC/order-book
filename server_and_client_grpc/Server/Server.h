#ifndef ORDERBOOK_SERVER_H
#define ORDERBOOK_SERVER_H

#include "../ServiceImplementation.h"

// Function to run the server and initialize MyServiceImpl
void RunServer(Market *market, const std::string& serverAddress = "localhost:50051");

#endif //ORDERBOOK_SERVER_H
