#include "FrameResizer.h"
#include "Utils.h"
#include <fcntl.h>
#include <opencv2/imgproc.hpp>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <unistd.h>

const cv::Size RESIZED_SIZE(960, 540);
const cv::Size ORIGINAL_SIZE(480, 270);

FrameResizer::FrameResizer(MessageQueue& inQueue, MessageQueue& outQueue, const std::string& shmFrame, const std::string& shmResizedFrame)
    : Service(inQueue, outQueue), mShmFrame(shmFrame), mShmResizedFrame(shmResizedFrame)
{
    mShmFd = shm_open(mShmFrame.c_str(), O_RDONLY, 0644);
    if (mShmFd < 0) {
        throw std::system_error(errno, std::generic_category(), std::string("Shared memory name: ") + mShmFrame);
    }
    mShmResizedFd = shm_open(mShmResizedFrame.c_str(), O_CREAT | O_RDWR, 0644);
    if (mShmResizedFd < 0) {
        throw std::system_error(errno, std::generic_category(), std::string("Shared memory name: ") + mShmResizedFrame);
    }
}

FrameResizer::~FrameResizer()
{
    if (shm_unlink(mShmResizedFrame.c_str()) < 0) {
        spdlog::error("FrameResizer: Failed to unlink shared memory {}: {}", mShmResizedFrame, strerror(errno));
    }
    if (mShmFd >= 0) {
        close(mShmFd);
        mShmFd = -1;
    }
    if (mShmResizedFd >= 0) {
        close(mShmResizedFd);
        mShmResizedFd = -1;
    }
}

void FrameResizer::processMessage(const std::string& message)
{
    if (message == "ACK") {
        cv::Mat frame(ORIGINAL_SIZE, CV_8UC3);
        if (!Utils::readFrameFromShm(frame, mShmFd)) {
            spdlog::error("FrameResizer: failed to read original frame from shared memory");
            return;
        }
        cv::Mat resizedFrame(RESIZED_SIZE, CV_8UC3);
        cv::resize(frame, resizedFrame, resizedFrame.size());
        if (!Utils::writeFrameToShm(resizedFrame, mShmResizedFd)) {
            spdlog::error("FrameResizer: failed to write resized frame to shared memory");
            return;
        }

        mOutQueue.send("ACK");
        return;
    }

    spdlog::warn("FrameResizer: unexpected message: {}", message);
}


