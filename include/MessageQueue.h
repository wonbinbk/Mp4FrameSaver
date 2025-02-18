#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H
#include <string>
#include <vector>
#include <cstdint>

class MessageQueue {
public:
    enum class DIRECTION {
        INPUT,
        OUTPUT,
    };
    MessageQueue(const std::string& queueName, DIRECTION direction, bool owner = false);
    ~MessageQueue();

    // Only on output queue, non block
    bool send(const std::vector<uint8_t>& message);
    bool send(const std::string& message);

    // Only on input queue, block
    bool receive(std::vector<uint8_t>& message);

private:
    std::string mName;
    int mDescriptor = -1;
    bool mOwner;
    DIRECTION mDirection;
};
#endif // MESSAGE_QUEUE_H
