#include "Service.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <spdlog/spdlog.h>

Service::Service(const std::string& queueName) : mQueueName(queueName)
{
    mDescriptor = mq_open(mQueueName.c_str(), O_CREAT | O_RDWR, 0644, nullptr);
    // if (mDescriptor < 0) {
    // }
    spdlog::info("Hello, this is a info Message");
}
