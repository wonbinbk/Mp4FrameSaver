#ifndef FRAME_SAVER_H
#define FRAME_SAVER_H

#include "Service.h"

class FrameSaver : public Service {
public:
    FrameSaver(MessageQueue& inQueue, MessageQueue& outQueue);
    ~FrameSaver() override = default;

private:
    void processMessage(const std::vector<uint8_t>& message) override;
};

#endif // FRAME_SAVER_H

