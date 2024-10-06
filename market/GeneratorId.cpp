#include "GeneratorId.h"

GeneratorId::GeneratorId(uint64_t lastUsedValue) : lastID_(lastUsedValue){}

uint64_t GeneratorId::nextID() {
    uint64_t val;
    if(lastID_==std::numeric_limits<uint64_t>::max()){
        throw std::overflow_error("Overflow in generated ID");
    }
    lastID_++;
    val = lastID_;
    return val;
};
