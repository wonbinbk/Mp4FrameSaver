#include "Utils.h"
#include <fcntl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

using nlohmann::json;

namespace Utils::Shm {

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

} // namespace Utils::Shm

namespace Utils::Json {

std::string frameInfoToJson(const Message& info)
{
    const auto j = json{{"vidPath", info.vidPath}, {"type", static_cast<int>(info.type)}, {"frameInfo", info.frameInfo}};

    return j.dump();
}

Message jsonToFrameInfo(const std::string& jsonString)
{
    try {
        Message info;
        auto j = json::parse(jsonString);
        j.at("vidPath").get_to(info.vidPath);
        int typeInt;
        j.at("type").get_to(typeInt);
        info.type = static_cast<MESSAGE_TYPE_ENUMS>(typeInt);
        j.at("frameInfo").get_to(info.frameInfo);
        return info;
    }
    catch (const json::parse_error& e) {
        spdlog::error("Utils::Json: failed to parse json string: {}. Error: {}", jsonString, e.what());
        return {};
    }
    catch (const json::exception& e) {
        spdlog::error("Utils::Json: exception when processing json string: {}. Error: {}", jsonString, e.what());
        return {};
    }
}

} // namespace Utils::Json
