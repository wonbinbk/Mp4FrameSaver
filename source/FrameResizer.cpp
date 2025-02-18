#include "FrameResizer.h"
#include <spdlog/spdlog.h>

FrameResizer::FrameResizer(MessageQueue& inQueue, MessageQueue& outQueue) : Service(inQueue, outQueue)
{
    spdlog::info("FrameResizer constructor");
}

void FrameResizer::processMessage(const std::vector<uint8_t>& message)
{
    spdlog::info("FrameResizer processMessage: message len {}", message.size());
    spdlog::info("FrameResizer send to FrameSaver");
    mOutQueue.send("Message to FrameSaver len 29");
}


