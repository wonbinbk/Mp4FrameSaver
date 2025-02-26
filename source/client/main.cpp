// client/main.cpp
#include "Config.h"
#include "MessageQueue.h"
#include <spdlog/spdlog.h>
#include <string>

int main(int argc, char** argv)
{
    if (argc != 2) {
        spdlog::error("Incorrect number of arguments.\nUsage: {} [input_video_path.mp4]", argv[0]);
        return -1;
    }
    std::string videoPath(argv[1]);
    MessageQueue publisherInQueue(Config::PUBLISHER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);
    MessageQueue clientInQueue(Config::CLIENT_IN_QUEUE, MessageQueue::DIRECTION::INPUT);
    publisherInQueue.send(std::string("path:") + videoPath);
    std::string response;
    if (!clientInQueue.receive(response)) { // blocking read
        spdlog::error("Error reading from queue {}", Config::CLIENT_IN_QUEUE);
        return -1;
    }

    if (response == "DONE") {
        spdlog::info("File {} frames were extracted successfully.", videoPath);
        return 0;
    }
    if (response == "BUSY") {
        spdlog::warn("mp4FrameSaver is busy. Try again later.");
        return 0;
    }

    spdlog::error("Unhandled response from mp4FrameSaver: {}", response);
    return -1;
}
