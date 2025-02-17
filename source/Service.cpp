#include "Service.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <spdlog/spdlog.h>
#include <errno.h>
#include <system_error>

Service::Service(const std::string& queueName) : mQueueName(queueName)
{
    mDescriptor = mq_open(mQueueName.c_str(), O_CREAT | O_RDWR, 0644, nullptr);
    if (mDescriptor < 0) {
        throw std::system_error(errno, std::generic_category(), mQueueName);
    }
    spdlog::info("Hello, this is a info Message");
}

Service::~Service()
{
    waitThread();

    if (mDescriptor >= 0) {
        mq_close(mDescriptor);
        mq_unlink(mQueueName.c_str());
        mDescriptor = -1;
    }
}

void Service::startThread()
{
    task = std::thread([this]() -> void { pollQueue(); });
}

void Service::waitThread()
{
    if (task.joinable())
        task.join();
}

void Service::pollQueue()
{
    std::vector<uint8_t> message(256);
    while (true) {
        auto msgLen = mq_receive(mDescriptor, reinterpret_cast<char*>(message.data()), message.size(), nullptr);
        if (msgLen < 0) {
                spdlog::error("Error receiving message from queue {}", mQueueName);
                continue;
        }
        processMessage(message);
    }
}
