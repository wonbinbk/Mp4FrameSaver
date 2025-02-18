#include "FramePublisher.h"
#include <spdlog/spdlog.h>

FramePublisher::FramePublisher(MessageQueue& inQueue, MessageQueue& outQueue) : Service(inQueue, outQueue)
{
    spdlog::info("FramePublisher constructor");
}

void FramePublisher::processMessage(const std::vector<uint8_t>& message)
{
    spdlog::info("FramePublisher processMessage: message len {}", message.size());
}
