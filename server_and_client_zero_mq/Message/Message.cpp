#include "Message.h"

messageTypes extractMessageType(std::string_view serializedData){
    switch(serializedData[0]){
        case 'u':
            return UPDATE;
            break;
        case 'd':
            return DELETION;
            break;
        case 'i':
            return INSERTION;
            break;
        case 'e':
            return EXECUTION;
            break;
        default:
            throw std::invalid_argument("the type of the operation is unknown");
    }
}


//////////////////////// Deletion class
DeletionMessage::DeletionMessage(std::string_view serializedMessage): Message(0) {
    auto firstChar = std::find( begin(serializedMessage), end(serializedMessage), '#')+1;
    orderID_ = std::stoul( ( serializedMessage.substr(firstChar-begin(serializedMessage), end(serializedMessage) - firstChar)).data());
}

std::string DeletionMessage::serialize() const {
    return "deletion#" + std::to_string(orderID_);
}


//////////////////////// Update class
UpdateMessage::UpdateMessage(uint64_t newOrderID,
                             uint64_t oldOrderID,
                             double newVolume,
                             double newPrice,
                             uint32_t version)
            : Message(oldOrderID)
            , newOrderID_ (newOrderID)
            , newVolume_ (newVolume)
            , newPrice_ (newPrice)
            , version_ (version) {}

UpdateMessage::UpdateMessage(std::string_view serializedMessage): Message(0){
    auto firstChar = begin(serializedMessage);
    auto firstHash = std::find(firstChar, end(serializedMessage), '#');
    orderID_ = std::stoul( (serializedMessage.substr(0, firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = std::find(firstChar, end(serializedMessage), '#');
    newOrderID_ = std::stoul( (serializedMessage.substr(firstChar-begin(serializedMessage), firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = std::find(firstChar, end(serializedMessage), '#');
    newVolume_ = std::stod( (serializedMessage.substr(firstChar-begin(serializedMessage), firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = std::find(firstChar, end(serializedMessage), '#');
    newPrice_ = std::stod( (serializedMessage.substr(firstChar-begin(serializedMessage), firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = end(serializedMessage);
    version_ = static_cast<uint32_t>( std::stoul( (serializedMessage.substr( firstChar-begin(serializedMessage), firstHash-firstChar)).data()));
};

std::string UpdateMessage::serialize() const {
    return "update#"
            + std::to_string(orderID_)   + "#"
            + std::to_string(newOrderID_)   + "#"
            + std::to_string(newVolume_) + "#"
            + std::to_string(newPrice_)  + "#"
            + std::to_string(version_);
}


//////////////////////// Execution class
ExecutionMessage::ExecutionMessage(uint64_t orderID,
                                   double remainingVolume,
                                   double executedVolume,
                                   double newPrice,
                                   uint32_t version)
            : Message(orderID)
            , remainingVolume_ (remainingVolume)
            , executedVolume_ (executedVolume)
            , newPrice_ (newPrice)
            , version_ (version) {}

ExecutionMessage::ExecutionMessage(std::string_view serializedMessage): Message(0){
    auto firstChar = begin(serializedMessage);
    auto firstHash = std::find(firstChar, end(serializedMessage), '#');
    orderID_ = std::stoul( (serializedMessage.substr(0, firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = std::find(firstChar, end(serializedMessage), '#');
    remainingVolume_ = std::stod( (serializedMessage.substr(firstChar-begin(serializedMessage), firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = std::find(firstChar, end(serializedMessage), '#');
    executedVolume_ = std::stod( (serializedMessage.substr(firstChar-begin(serializedMessage), firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = std::find(firstChar, end(serializedMessage), '#');
    newPrice_ = std::stod( (serializedMessage.substr(firstChar-begin(serializedMessage), firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = end(serializedMessage);
    version_ = static_cast<uint32_t>( std::stoul( (serializedMessage.substr( firstChar-begin(serializedMessage), firstHash-firstChar)).data()));
};

std::string ExecutionMessage::serialize() const {
    return "execution#"
           + std::to_string(orderID_)           + "#"
           + std::to_string(remainingVolume_)   + "#"
           + std::to_string(executedVolume_)    + "#"
           + std::to_string(newPrice_)          + "#"
           + std::to_string(version_);
}


//////////////////////// Insertion class
InsertionMessage::InsertionMessage(uint64_t orderID,
                             double volume,
                             double price,
                             uint32_t version)
        : Message(orderID)
        , volume_ (volume)
        , price_ (price)
        , version_ (version) {}

InsertionMessage::InsertionMessage(std::string_view serializedMessage): Message(0){
    auto firstChar = begin(serializedMessage);
    auto firstHash = std::find(firstChar, end(serializedMessage), '#');
    orderID_ = std::stoul( (serializedMessage.substr(0, firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = std::find(firstChar, end(serializedMessage), '#');
    volume_ = std::stod( (serializedMessage.substr(firstChar-begin(serializedMessage), firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = std::find(firstChar, end(serializedMessage), '#');
    price_ = std::stod( (serializedMessage.substr(firstChar-begin(serializedMessage), firstHash-firstChar)).data());

    firstChar = firstHash+1;
    firstHash = end(serializedMessage);
    version_ = static_cast<uint32_t>( std::stoul( (serializedMessage.substr( firstChar-begin(serializedMessage), firstHash-firstChar)).data()));
};

std::string InsertionMessage::serialize() const {
    return "update#"
           + std::to_string(orderID_)   + "#"
           + std::to_string(volume_)    + "#"
           + std::to_string(price_)     + "#"
           + std::to_string(version_);
}