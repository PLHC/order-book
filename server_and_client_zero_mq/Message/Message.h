#ifndef ORDERBOOK_MESSAGE_H
#define ORDERBOOK_MESSAGE_H

#include <cstdint>
#include <string>
#include <stdexcept>

enum messageTypes {UPDATE, DELETION, INSERTION, EXECUTION};

// function used to extract message type without deserializing and therefore without building a Message object
messageTypes extractMessageType(std::string_view serializedData);


//////////////////////// Base class
class Message {
protected:
    uint64_t orderID_ = 0;

public:
    explicit Message(uint64_t orderID): orderID_(orderID){}
    virtual ~Message() = default;
    [[nodiscard]] uint64_t getterOrderID() const { return orderID_;}
    virtual std::string serialize() const = 0;
};


//////////////////////// Deletion class
class DeletionMessage: public Message{
public:
    explicit DeletionMessage(uint64_t orderID): Message(orderID){}
    explicit DeletionMessage(std::string_view serializedMessage);
    [[nodiscard]] std::string serialize() const override;
};


//////////////////////// Update class
class UpdateMessage: public Message{
    uint64_t newOrderID_ = 0;
    double newVolume_ = 0;
    double newPrice_ = 0;
    uint32_t version_ = 0;

public:
    UpdateMessage(uint64_t newOrderID,
                  uint64_t oldOrderID,
                  double newVolume,
                  double newPrice,
                  uint32_t version);

    explicit UpdateMessage(std::string_view serializedMessage);

    [[nodiscard]] std::string serialize() const override;
    [[nodiscard]] double getterNewVolume() const { return newVolume_; }
    [[nodiscard]] double getterNewPrice() const { return newPrice_; }
    [[nodiscard]] uint32_t getterVersion() const { return version_; }
};


//////////////////////// Execution class
class ExecutionMessage: public Message{
    double executedVolume_ = 0;
    double remainingVolume_ = 0;
    double newPrice_ = 0;
    uint32_t version_ = 0;

public:
    ExecutionMessage(uint64_t orderID,
                  double remainingVolume,
                  double executedVolume,
                  double newPrice,
                  uint32_t version);

    explicit ExecutionMessage(std::string_view serializedMessage);

    [[nodiscard]] std::string serialize() const override;
    [[nodiscard]] double getterExecutedVolume() const { return executedVolume_; }
    [[nodiscard]] double getterRemainingVolume() const { return remainingVolume_; }
    [[nodiscard]] double getterNewPrice() const { return newPrice_; }
    [[nodiscard]] uint32_t getterVersion() const { return version_; }
};


//////////////////////// Insertion class
class InsertionMessage: public Message{
    double volume_ = 0;
    double price_ = 0;
    uint32_t version_ = 0;

public:
    InsertionMessage(uint64_t orderID,
                  double volume,
                  double price,
                  uint32_t version);

    explicit InsertionMessage(std::string_view serializedMessage);

    [[nodiscard]] std::string serialize() const override;
    [[nodiscard]] double getterVolume() const { return volume_; }
    [[nodiscard]] double getterPrice() const { return price_; }
    [[nodiscard]] uint32_t getterVersion() const { return version_; }
};

#endif //ORDERBOOK_MESSAGE_H
