#include "FramePublisher.h"
#include "Utils.h"
#include <fcntl.h>
#include <opencv2/videoio.hpp>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

FramePublisher::FramePublisher(MessageQueue& inQueue, MessageQueue& outQueue, const std::string& shmFrame) : Service(inQueue, outQueue), mShmFrame(shmFrame)
{
    mShmFd = shm_open(shmFrame.c_str(), O_CREAT | O_RDWR, 0644);
    if (mShmFd < 0) {
        throw std::system_error(errno, std::generic_category(), std::string("Shared memory name: ") + mShmFrame);
    }
}

FramePublisher::~FramePublisher()
{
    joinVideoThread();
    if (shm_unlink(mShmFrame.c_str()) < 0) {
        spdlog::error("FramePublisher: Failed to unlink shared memory {}: {}", mShmFrame, strerror(errno));
    }
    if (mShmFd >= 0) {
        close(mShmFd);
        mShmFd = -1;
    }
}

void FramePublisher::joinVideoThread()
{
    if (mVideoTask.joinable())
        mVideoTask.join();
}
/*
 * FramePublisher needs to:
 *   - check message for new video file name.
 *   - check message for ACK signal from FrameSaver.
 *   - when it has a video to extract, it needs to loop through the frames of that video.
 */
void FramePublisher::processMessage(const std::string& message)
{
    if (message == "ACK") {
        // FrameSaver ACK that the pipeline is free now
        // notify other thread to send next frame
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mFrameProcessed = true;
            mCv.notify_one();
        }
        return;
    }

    if (message.substr(0, 5) == "path:") {
        spdlog::info("FramePublisher: start extracting frames from {}", message.substr(5));
        return processVideo(message);
    }

    spdlog::warn("FramePublisher: unexpected message: {}", message);
}

void FramePublisher::processVideo(const std::string& message)
{
    if (mBusy.load()) {
        spdlog::error("FramePublisher: Still busy processing file {}", mVideoFileName);
        return;
    }

    mVideoFileName = message.substr(5);
    if (mVideoFileName.empty()) {
        spdlog::error("FramePublisher: No path in message");
        return;
    }

    joinVideoThread();

    mVideoTask = std::thread([this]() -> void { processVideoThread(mVideoFileName); });

    return;
}

void FramePublisher::processVideoThread(const std::string& filePath)
{
    cv::VideoCapture cap(filePath);
    if (!cap.isOpened()) {
        spdlog::error("FramePublisher: Path does not exist or file is corrupted: {}", filePath);
        return;
    }

    cv::Mat frame;
    mBusy = true;

    while (cap.read(frame)) {
        spdlog::info("Frame size: {} x {} x {}, type {}", frame.cols, frame.rows, frame.channels(), frame.type());
        if (frame.empty() || !Utils::writeFrameToShm(frame, mShmFd)) {
            spdlog::error("FramePublisher: failed to write this frame, go to next frame");
            continue;
        }
        // Send an acknowledge messge to FrameResizer so it starts processing the frame on shared memory
        mOutQueue.send("ACK");
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mCv.wait(lock, [&] { return mFrameProcessed; });
            mFrameProcessed = false;
        }
    }

    mBusy = false;
}
