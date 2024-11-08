#include "GeneratorId.h"

// static variables single initialization
std::mutex GeneratorId::generatorMtx_ {};
GeneratorId* GeneratorId::instance_{ nullptr };

int64_t GeneratorId::nextID() {
    std::unique_lock<std::mutex> genLock (generatorMtx_);
    if(lastID_==std::numeric_limits<int64_t>::max()){
        throw std::overflow_error("Overflow in generated ID");
    }
    auto val = ++lastID_;
    return val;
};
