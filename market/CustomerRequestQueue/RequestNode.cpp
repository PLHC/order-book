#include "RequestNode.h"

RequestNode::RequestNode():
    prevMutex_(),
    statusMutex_(),
    prevConditionVariable_(),
    statusConditionVariable_(),
    status_(CREATED){}
