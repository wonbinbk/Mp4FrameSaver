#ifndef SERVICE_H
#define SERVICE_H
#include <vector>
#include <cstdint>
#include <thread>
#include "MessageQueue.h"

class Service {
public:
    Service(MessageQueue& inQueue, MessageQueue& outQueue);
    virtual ~Service();
    void startThread();

protected:
    virtual void processMessage(const std::vector<uint8_t>& message) = 0;
    void waitThread();

    MessageQueue& mInQueue;
    MessageQueue& mOutQueue;

private:
    void pollInQueue();
    std::thread task;
};
#endif // SERVICE_H
