#include "Service.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <spdlog/spdlog.h>

Service::Service(MessageQueue& inQueue, MessageQueue& outQueue) : mInQueue(inQueue), mOutQueue(outQueue)
{
}

Service::~Service()
{
    waitThread();
}

void Service::startThread()
{
    task = std::thread([this]() -> void { pollInQueue(); });
}

void Service::waitThread()
{
    if (task.joinable())
        task.join();
}

void Service::pollInQueue()
{
    std::vector<uint8_t> message;
    while (true) {
        if (!mInQueue.receive(message)) {
            // Error logged in MessageQueue
            continue;
        }
        processMessage(message);
    }
}
