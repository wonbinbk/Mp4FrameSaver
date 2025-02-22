#include "FrameSaver.h"
#include "Utils.h"
#include <fcntl.h>
#include <filesystem>
#include <opencv2/imgcodecs.hpp>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <unistd.h>

const cv::Size ORIGINAL_SIZE(1024, 768);
const cv::Size RESIZED_SIZE(640, 480);

FrameSaver::FrameSaver(MessageQueue& inQueue, MessageQueue& outQueue, const std::string& shmResizedFrame)
    : Service(inQueue, outQueue), mShmResizedFrame(shmResizedFrame)
{
    mShmResizedFd = shm_open(mShmResizedFrame.c_str(), O_RDONLY, 0644);
    if (mShmResizedFd < 0) {
        throw std::system_error(errno, std::generic_category(), std::string("Shared memory name: ") + mShmResizedFrame);
    }
}

FrameSaver::~FrameSaver()
{
    if (mShmResizedFd >= 0) {
        close(mShmResizedFd);
        mShmResizedFd = -1;
    }
}

void FrameSaver::processMessage(const std::string& message)
{
    if (message == "ACK") {
        cv::Mat resizedFrame(RESIZED_SIZE, CV_8UC3);
        if (!Utils::readFrameFromShm(resizedFrame, mShmResizedFd)) {
            spdlog::error("FrameSaver: failed to read resized frame from shared memory");
            return;
        }
        saveFrame(resizedFrame);
        return;
    }

    spdlog::warn("FrameSaver: unexpected message: {}", message);
}

void FrameSaver::saveFrame(const cv::Mat& frame)
{
    std::error_code ec;
    if (!std::filesystem::create_directories(mOutputDir, ec)) {
        spdlog::error("FrameSaver: failed to create directories {}: {}", mOutputDir, ec.message());
        return;
    }
    const auto savedImgPath = std::filesystem::path(mOutputDir).append(std::to_string(mFrameCnt++) + ".jpg");
    cv::imwrite(savedImgPath.string(), frame);
    mOutQueue.send("ACK");
}
