#include "MessageQueue.h"
#include "FramePublisher.h"
#include "FrameResizer.h"
#include "FrameSaver.h"
#include <spdlog/spdlog.h>

constexpr const char* RESIZER_IN_QUEUE="/RESIZER_IN_QUEUE";
constexpr const char* SAVER_IN_QUEUE="/SAVER_IN_QUEUE";
constexpr const char* PUBLISHER_IN_QUEUE="/PUBLISHER_IN_QUEUE";

constexpr const char* SHM_FRAME="/SHM_ORIGINAL_FRAME";
constexpr const char* SHM_RESIZED_FRAME="/SHM_RESIZED_FRAME";
constexpr const char* DEFAULT_OUTPUT_DIR="/home/pokyuser/Mp4ResizedFrames";

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

    const std::string outputDir = (argc == 2) ? argv[1] : DEFAULT_OUTPUT_DIR;

    MessageQueue resizerInQueue(RESIZER_IN_QUEUE, MessageQueue::DIRECTION::INPUT, true);
    MessageQueue saverInQueue(SAVER_IN_QUEUE, MessageQueue::DIRECTION::INPUT, true);
    MessageQueue publisherInQueue(PUBLISHER_IN_QUEUE, MessageQueue::DIRECTION::INPUT, true);

    MessageQueue publishOutQueue(RESIZER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);
    MessageQueue resizerOutQueue(SAVER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);
    MessageQueue saverOutQueue(PUBLISHER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);

    FramePublisher publisher(publisherInQueue, publishOutQueue, SHM_FRAME);
    FrameResizer resizer(resizerInQueue, resizerOutQueue, SHM_FRAME, SHM_RESIZED_FRAME);
    FrameSaver saver(saverInQueue, saverOutQueue, SHM_RESIZED_FRAME, outputDir);

    publisher.startThread();
    resizer.startThread();
    saver.startThread();
    spdlog::info("Ready to process video file and output to {}. Listening on queue for 'path:path_to_video_file'", outputDir);
    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
