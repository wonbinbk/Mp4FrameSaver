#ifndef FRAME_PUBLISHER_H
#define FRAME_PUBLISHER_H

#include "Service.h"

class FramePublisher : public Service {
public:
    FramePublisher(MessageQueue& inQueue, MessageQueue& outQueue);
    ~FramePublisher() override = default;

private:
    void processMessage(const std::vector<uint8_t>& message) override;
};

#endif // FRAME_PUBLISHER_H
