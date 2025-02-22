#ifndef FRAME_RESIZER_H
#define FRAME_RESIZER_H

#include "Service.h"

class FrameResizer : public Service {
public:
    FrameResizer(MessageQueue& inQueue, MessageQueue& outQueue, const std::string& shmFrame, const std::string& shmResizedFrame);
    ~FrameResizer() override;

    FrameResizer(const FrameResizer&) = delete;
    FrameResizer(FrameResizer&&) = delete;
    FrameResizer& operator=(const FrameResizer&) = delete;
    FrameResizer& operator=(FrameResizer&&) = delete;

private:
    void processMessage(const std::string& message) override;

    int mShmFd = -1;
    int mShmResizedFd = -1;
    std::string mShmFrame;
    std::string mShmResizedFrame;
};

#endif // FRAME_RESIZER_H
