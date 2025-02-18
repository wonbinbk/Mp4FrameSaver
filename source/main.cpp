#include "MessageQueue.h"
#include "FramePublisher.h"
#include "FrameResizer.h"
#include "FrameSaver.h"
#include <spdlog/spdlog.h>

constexpr const char* RESIZER_IN_QUEUE="/RESIZER_IN_QUEUE";
constexpr const char* SAVER_IN_QUEUE="/SAVER_IN_QUEUE";
constexpr const char* PUBLISHER_IN_QUEUE="/PUBLISHER_IN_QUEUE";

int main()
{
    spdlog::info("Main Mp4FrameSaver");
    MessageQueue resizerInQueue(RESIZER_IN_QUEUE, MessageQueue::DIRECTION::INPUT, true);
    MessageQueue saverInQueue(SAVER_IN_QUEUE, MessageQueue::DIRECTION::INPUT, true);
    MessageQueue publisherInQueue(PUBLISHER_IN_QUEUE, MessageQueue::DIRECTION::INPUT, true);

    MessageQueue publishOutQueue(RESIZER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);
    MessageQueue resizerOutQueue(SAVER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);
    MessageQueue saverOutQueue(PUBLISHER_IN_QUEUE, MessageQueue::DIRECTION::OUTPUT);

    FramePublisher publisher(publisherInQueue, publishOutQueue);
    FrameResizer resizer(resizerInQueue, resizerOutQueue);
    FrameSaver saver(saverInQueue, saverOutQueue);

    publisher.startThread();
    resizer.startThread();
    saver.startThread();
}
