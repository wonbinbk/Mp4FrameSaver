#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H
#include <string>

class MessageQueue {
public:
    enum class DIRECTION {
        INPUT,
        OUTPUT,
    };
    MessageQueue(const std::string& queueName, DIRECTION direction, bool owner = false);
    ~MessageQueue();

    // Only on output queue, non block
    bool send(const std::string& message);

    // Only on input queue, block
    bool receive(std::string& message);

private:
    std::string mName;
    int mDescriptor = -1;
    bool mOwner;
    DIRECTION mDirection;
};
#endif // MESSAGE_QUEUE_H
