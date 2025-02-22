#include "FramePublisher.h"
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
    spdlog::info("FramePublisher: processMessage: message {} len {}", message, message.size());
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
        return processVideo(message);
    }

    spdlog::warn("FramePublisher: No handle for message: {}", message);
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
        copyFrameToShm(frame);
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

bool FramePublisher::copyFrameToShm(const cv::Mat& frame)
{
    const auto frameMemSize = frame.total() * frame.elemSize();
    if (ftruncate(mShmFd, frameMemSize) < 0) {
            spdlog::error("FramePublisher: Failed to resize shared memory {}: {}", mShmFrame, strerror(errno));
            return false;
    }
    auto* framePtr = mmap(0, frameMemSize, PROT_WRITE, MAP_SHARED, mShmFd, 0);
    if (framePtr == MAP_FAILED) {
        spdlog::error("FramePublisher: Failed to map shared memory with size {}", frameMemSize);
        return false;
    }

    memcpy(framePtr, frame.data, frameMemSize);
    if (munmap(framePtr, frameMemSize) < 0) {
        spdlog::error("FramePublisher: Failed to unmap shared memory");
    }

    return true;
}
