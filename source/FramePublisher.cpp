#include "FramePublisher.h"
#include "Utils.h"
#include <fcntl.h>
#include <opencv2/videoio.hpp>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static constexpr const char* PATH_HEADER = "path:";
static const size_t PATH_HEADER_SIZE = 5;

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
    // If message string starts with `path:` then the rest of the message is path to the input video file
    // Alternatively, we can implement a MESSAGE_TYPE_ENUMS::CMD for this kind of message, but it is more complicated
    // so for our purpose here, this works just fine
    if (message.size() > PATH_HEADER_SIZE && message.substr(0, PATH_HEADER_SIZE) == PATH_HEADER) {
	const auto videoPath = message.substr(PATH_HEADER_SIZE);
        return processVideo(videoPath);
    }

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

    spdlog::warn("FramePublisher: unexpected message: {}", message);
}

void FramePublisher::processVideo(const std::string& videoPath)
{
    if (mBusy.load()) {
        spdlog::error("FramePublisher: Still busy processing file {}", mVideoFileName);
        return;
    }

    if (videoPath.empty()) {
        spdlog::error("FramePublisher: No path in message");
        return;
    }

    joinVideoThread();

    mVideoFileName = videoPath;
    mVideoTask = std::thread([this]() -> void { processVideoThread(mVideoFileName); });

    return;
}

void FramePublisher::processVideoThread(const std::string& videoPath)
{
    cv::VideoCapture cap(videoPath, cv::CAP_FFMPEG);
    if (!cap.isOpened()) {
        spdlog::error("FramePublisher: unable to open video in path {} for processing", videoPath);
        return;
    }

    cv::Mat frame;
    mBusy = true;

    spdlog::info("FramePublisher: start extracting frames from {}", videoPath);
    while (cap.read(frame)) {
        if (frame.empty() || !Utils::Shm::writeFrameToShm(frame, mShmFd)) {
            spdlog::error("FramePublisher: failed to write this frame, go to next frame");
            continue;
        }

        Message msg(videoPath, MESSAGE_TYPE_ENUMS::ACK, frame);
        mOutQueue.send(Utils::Json::frameInfoToJson(msg));

        {
            std::unique_lock<std::mutex> lock(mMutex);
            mCv.wait(lock, [&] { return mFrameProcessed; });
            mFrameProcessed = false;
        }
    }
    spdlog::info("FramePublisher: done processing file {}", videoPath);
    mBusy = false;
}
