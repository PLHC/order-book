#include "RequestNode.h"

RequestNode::RequestNode():
    prevMutex_(),
    statusMutex_(),
    statusConditionVariable_(),
    status_(CREATED){}
