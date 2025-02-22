#include "Utils.h"
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace Utils {

bool writeFrameToShm(const cv::Mat& frame, int shmFd)
{
    const auto frameMemSize = frame.total() * frame.elemSize();
    if (shmFd < 0) {
        spdlog::error("Utils: invalid shared memory descriptor to write");
        return false;
    }
    if (ftruncate(shmFd, frameMemSize) < 0) {
        spdlog::error("Utils: Failed to truncate shared memory: {}", strerror(errno));
        return false;
    }
    auto* framePtr = mmap(0, frameMemSize, PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (framePtr == MAP_FAILED) {
        spdlog::error("Utils: Failed to map shared memory with size {}", frameMemSize);
        return false;
    }

    memcpy(framePtr, frame.data, frameMemSize);
    if (munmap(framePtr, frameMemSize) < 0) {
        spdlog::error("Utils: Failed to unmap shared memory");
        return false;
    }

    return true;
}

bool readFrameFromShm(cv::Mat& frame, int shmFd)
{
    if (shmFd < 0) {
        spdlog::error("Utils: invalid shared memory descriptor to read");
        return false;
    }
    const auto frameMemSize = frame.total() * frame.elemSize();
    auto* framePtr = mmap(0, frameMemSize, PROT_READ, MAP_SHARED, shmFd, 0);
    if (framePtr == MAP_FAILED) {
        spdlog::error("Utils: failed to map shared memory to read");
        return false;
    }
    memcpy(frame.data, framePtr, frameMemSize);
    if (munmap(framePtr, frameMemSize) < 0) {
        spdlog::error("Utils: Failed to unmap shared memory");
        return false;
    }
    return true;
}

} // namespace Utils
