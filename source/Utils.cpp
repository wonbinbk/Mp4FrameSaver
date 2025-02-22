#include "Utils.h"
#include <fcntl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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

std::string frameInfoToJson(const Message& message)
{
    json j;
    j["type"] = message.type;
    j["shmName"] = message.shmName;
    j["vidPath"] = message.vidPath;
    j["metadata"]["rows"] = message.metadata.rows;
    j["metadata"]["columns"] = message.metadata.columns;
    j["metadata"]["channels"] = message.metadata.channels;
    j["metadata"]["type"] = message.metadata.type;

    return j.dump();
}

Message jsonToFrameInfo(const std::string& jsonString)
{
    Message message;
    try {
        auto j = json::parse(jsonString);
        message.type = j["type"];
        message.shmName = j["shmName"];
        message.vidPath = j["vidPath"];
        message.metadata.rows = j["metadata"]["rows"];
        message.metadata.columns = j["metadata"]["columns"];
        message.metadata.channels = j["metadata"]["channels"];
        message.metadata.type = j["metadata"]["type"];
    }
    catch(const json::parse_error& e) {
        spdlog::error("Utils::Json: failed to parse json string: {}. Error: {}", jsonString, e.what());
        return {};
    }
    catch(const json::exception& e) {
        spdlog::error("Utils::Json: exception when processing json string: {}. Error: {}", jsonString, e.what());
        return {};
    }

    return message;
}

} // namespace Utils::Json
