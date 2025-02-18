#include "FrameSaver.h"
#include <spdlog/spdlog.h>

FrameSaver::FrameSaver(MessageQueue& inQueue, MessageQueue& outQueue) : Service(inQueue, outQueue)
{
    spdlog::info("FrameSaver constructor");
}

void FrameSaver::processMessage(const std::vector<uint8_t>& message)
{
    spdlog::info("FrameSaver processMessage: message len {}", message.size());
    spdlog::info("FrameSaver send to FramePublisher");
    mOutQueue.send("Message to FramePublisher len 33");
}

