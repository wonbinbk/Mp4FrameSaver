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
    MessageQueue saverOutQueue(Config::PUBLISHER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);
    saverOutQueue.send(std::string("path:") + videoPath);
    return 0;
}
