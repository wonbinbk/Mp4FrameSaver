#ifndef FRAME_RESIZER_H
#define FRAME_RESIZER_H

#include "Service.h"

class FrameResizer : public Service {
public:
    FrameResizer(MessageQueue& inQueue, MessageQueue& outQueue);
    ~FrameResizer() override = default;

private:
    void processMessage(const std::vector<uint8_t>& message) override;
};

#endif // FRAME_RESIZER_H
