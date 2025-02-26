#include "Config.h"
#include "FramePublisher.h"
#include "FrameResizer.h"
#include "FrameSaver.h"
#include "MessageQueue.h"
#include <spdlog/spdlog.h>

/*
 * Mp4FrameSaver
 * usage: mp4FrameSaver output_dir
 * the extracted and resized frames are saved in incrementing order under an extra directory having the same name as the input video file
 * input video file path can be passed to FramePublisher by writing to its monitored message queue.
 *
 * FramePublish, FrameResizer and FrameSaver communicates via separate POSIX message queues but only for acknowledgement and image metadata information.
 * The actual frame pixel data is written to and read from share memory locations for efficiency.
 */
int main(int argc, char** argv)
{
    if (argc > 2) {
	spdlog::error("Too many arguments.\nUsage: {} [output_path]", argv[0]);
	return -1;
    }

    const std::string outputDir = (argc == 2) ? argv[1] : Config::DEFAULT_OUTPUT_DIR;

    MessageQueue resizerInQueue(Config::RESIZER_IN_QUEUE, MessageQueue::DIRECTION::INPUT, true);
    MessageQueue saverInQueue(Config::SAVER_IN_QUEUE, MessageQueue::DIRECTION::INPUT, true);
    MessageQueue publisherInQueue(Config::PUBLISHER_IN_QUEUE, MessageQueue::DIRECTION::INPUT, true);

    MessageQueue publishOutQueue(Config::RESIZER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);
    MessageQueue resizerOutQueue(Config::SAVER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);
    MessageQueue saverOutQueue(Config::PUBLISHER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);

    // A method for publisher to notify its client
    MessageQueue clientInQueue(Config::CLIENT_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT, true);
    std::function<void(bool)> notifyCb = [&clientInQueue](bool busy) -> void {
        std::string message = busy ? "BUSY" : "DONE";
        clientInQueue.send(message);
    };
    FramePublisher publisher(publisherInQueue, publishOutQueue, Config::SHM_FRAME, notifyCb);
    FrameResizer resizer(resizerInQueue, resizerOutQueue, Config::SHM_FRAME, Config::SHM_RESIZED_FRAME);
    FrameSaver saver(saverInQueue, saverOutQueue, Config::SHM_RESIZED_FRAME, outputDir);

    publisher.startThread();
    resizer.startThread();
    saver.startThread();
    spdlog::info("Ready to process video file and output to {}. Listening on queue for 'path:path_to_video_file'", outputDir);
    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
