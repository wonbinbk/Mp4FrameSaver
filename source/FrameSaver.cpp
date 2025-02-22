#include "FrameSaver.h"
#include "Utils.h"
#include <fcntl.h>
#include <filesystem>
#include <opencv2/imgcodecs.hpp>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <unistd.h>

FrameSaver::FrameSaver(MessageQueue& inQueue, MessageQueue& outQueue, const std::string& shmResizedFrame, const std::string& outputDir)
    : Service(inQueue, outQueue), mShmResizedFrame(shmResizedFrame), mOutputDir(outputDir)
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
    const auto msg = Utils::Json::jsonToFrameInfo(message);

    switch (msg.type) {
    case MESSAGE_TYPE_ENUMS::ACK: {
        cv::Mat resizedFrame(msg.frameInfo.rows, msg.frameInfo.columns, msg.frameInfo.type);
        if (!Utils::Shm::readFrameFromShm(resizedFrame, mShmResizedFd)) {
            spdlog::error("FrameSaver: failed to read resized frame from shared memory");
            return;
        }
        const std::filesystem::path vidPath{msg.vidPath};
	const auto innerDir = vidPath.stem().string();
        saveFrame(innerDir, resizedFrame);
	mOutQueue.send("ACK");
        return;
    }
    case MESSAGE_TYPE_ENUMS::SENTINEL:
	spdlog::warn("FrameSaver: unexpected message: {}", message);
	break;
    }
}

void FrameSaver::saveFrame(const std::string& innerDir, const cv::Mat& frame)
{
    if (innerDir.empty()) {
	spdlog::error("FrameSaver: no directory to save frame");
	return;
    }
    if (frame.empty()) {
	spdlog::error("FrameSaver: no frame to save");
	return;
    }

    std::error_code ec;
    // reset frame count if we are to save in a different inner directory (ie: new video path)
    if (innerDir != mInnerDir) {
	mFrameCnt = 0;
	mInnerDir = innerDir;
    }

    const auto targetDir = std::filesystem::path(mOutputDir).append(innerDir);
    std::filesystem::create_directories(targetDir, ec);
    if (ec) {
        spdlog::error("FrameSaver: failed to create directories {}: {}", targetDir.string(), ec.message());
        return;
    }
    const auto savedImgPath = std::filesystem::path(targetDir).append(std::to_string(mFrameCnt++) + ".jpg");
    cv::imwrite(savedImgPath.string(), frame);
}
