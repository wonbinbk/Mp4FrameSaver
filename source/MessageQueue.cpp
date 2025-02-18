#include <mqueue.h>
#include <errno.h>
#include <system_error>
#include <spdlog/spdlog.h>
#include "MessageQueue.h"

constexpr const int MAX_MSG = 10;
constexpr const int MAX_MSG_SIZE = 1024;

MessageQueue::MessageQueue(const std::string& queueName, DIRECTION direction, bool owner) : mName(queueName), mOwner(owner), mDirection(direction)
{
    int flag = (owner ? O_CREAT : 0) | (direction == DIRECTION::OUTPUT ? (O_NONBLOCK | O_WRONLY) : O_RDONLY);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MAX_MSG_SIZE;

	if (mOwner) {
		mDescriptor = mq_open(mName.c_str(), flag, 0644, &attr);
	} else {
		mDescriptor = mq_open(mName.c_str(), flag);
	}

    if (mDescriptor < 0) {
        throw std::system_error(errno, std::generic_category(), mName);
    }
}

MessageQueue::~MessageQueue()
{
    if (mDescriptor < 0)
        return;

    mq_close(mDescriptor);
    mDescriptor = -1;

	if (mOwner)
		mq_unlink(mName.c_str());
}

bool MessageQueue::send(const std::vector<uint8_t>& message)
{
    if (mDirection == DIRECTION::INPUT) {
        throw std::domain_error(std::string("Can not send on input queue ") + mName);
    }

	if (mq_send(mDescriptor, reinterpret_cast<const char*>(message.data()), message.size(), 0) == 0)
		return true;

	spdlog::error("Error sending message on queue {}: {}", mName, strerror(errno));
	return false;
}

bool MessageQueue::send(const std::string& message)
{
	return send(std::vector<uint8_t>(message.begin(), message.end()));
}

bool MessageQueue::receive(std::vector<uint8_t>& message)
{
	if (mDirection == DIRECTION::OUTPUT) {
		throw std::domain_error(std::string("Can not receive on output queue ") + mName);
	}

    message.resize(MAX_MSG_SIZE);
    auto msgLen = mq_receive(mDescriptor, reinterpret_cast<char*>(message.data()), message.size(), nullptr);
    if (msgLen < 0) {
        spdlog::error("Error receiving message from queue {}: {}", mName, strerror(errno));
        return false;
    }

    message.resize(msgLen);
    return true;
}
