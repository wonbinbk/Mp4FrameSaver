#ifndef FRAME_SAVER_H
#define FRAME_SAVER_H

#include "Service.h"
#include <opencv2/core/mat.hpp>

class FrameSaver : public Service {
public:
    FrameSaver(MessageQueue& inQueue, MessageQueue& outQueue, const std::string& shmResizedFrame);
    ~FrameSaver() override;

    FrameSaver(const FrameSaver&) = delete;
    FrameSaver(FrameSaver&&) = delete;
    FrameSaver& operator=(const FrameSaver&) = delete;
    FrameSaver& operator=(FrameSaver&&) = delete;

private:
    void processMessage(const std::string& message) override;
    void saveFrame(const cv::Mat& frame);
    std::string mShmResizedFrame;
    std::string mOutputDir {"resized"};
    int mShmResizedFd = -1;
    int mFrameCnt = 0;
};

#endif // FRAME_SAVER_H

