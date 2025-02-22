#ifndef FRAME_PUBLISHER_H
#define FRAME_PUBLISHER_H

#include "Service.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <opencv2/videoio.hpp>

class FramePublisher : public Service {
public:
    FramePublisher(MessageQueue& inQueue, MessageQueue& outQueue, const std::string& shmFrame);
    ~FramePublisher() override;

    FramePublisher(const FramePublisher&) = delete;
    FramePublisher(FramePublisher&&) = delete;
    FramePublisher& operator=(const FramePublisher&) = delete;
    FramePublisher& operator=(FramePublisher&&) = delete;

private:
    void processMessage(const std::string& message) override;
    void processVideo(const std::string& videoPath);
    void processVideoThread(const std::string& filePath);
    void joinVideoThread();

    std::string mShmFrame;
    int mShmFd = -1;
    std::string mVideoFileName;

    std::thread mVideoTask;
    std::atomic_bool mBusy = false;
    bool mFrameProcessed = false;
    std::mutex mMutex;
    std::condition_variable mCv;
};

#endif // FRAME_PUBLISHER_H
