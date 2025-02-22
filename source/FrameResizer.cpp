#include "FrameResizer.h"
#include "Utils.h"
#include <fcntl.h>
#include <opencv2/imgproc.hpp>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <unistd.h>

constexpr float RESIZED_SCALE = 2;

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
    const auto msg = Utils::Json::jsonToFrameInfo(message);

    switch (msg.type) {
    case MESSAGE_TYPE_ENUMS::ACK: {
        cv::Mat frame(msg.frameInfo.rows, msg.frameInfo.columns, msg.frameInfo.type);
        if (!Utils::Shm::readFrameFromShm(frame, mShmFd)) {
            spdlog::error("FrameResizer: failed to read original frame from shared memory");
            return;
        }
        cv::Mat resizedFrame(static_cast<int>(msg.frameInfo.rows * RESIZED_SCALE), static_cast<int>(msg.frameInfo.columns * RESIZED_SCALE), msg.frameInfo.type);
        cv::resize(frame, resizedFrame, resizedFrame.size());
        if (!Utils::Shm::writeFrameToShm(resizedFrame, mShmResizedFd)) {
            spdlog::error("FrameResizer: failed to write resized frame to shared memory");
            return;
        }


        const Message outMsg(msg.vidPath, MESSAGE_TYPE_ENUMS::ACK, resizedFrame);
        mOutQueue.send(Utils::Json::frameInfoToJson(outMsg));

        return;
    }
    case MESSAGE_TYPE_ENUMS::SENTINEL:
	spdlog::warn("FrameResizer: unexpected message: {}", message);
	break;
    }
}


