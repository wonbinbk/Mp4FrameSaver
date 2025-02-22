#ifndef SERVICE_H
#define SERVICE_H
#include <thread>
#include "MessageQueue.h"

class Service {
public:
    Service(MessageQueue& inQueue, MessageQueue& outQueue);
    virtual ~Service();

    Service(const Service&) = delete;
    Service(Service&&) = delete;
    Service& operator=(const Service&) = delete;
    Service& operator=(Service&&) = delete;

    void startThread();

protected:
    virtual void processMessage(const std::string& message) = 0;
    void waitThread();

    MessageQueue& mInQueue;
    MessageQueue& mOutQueue;

private:
    void pollInQueue();
    std::thread task;
};
#endif // SERVICE_H
