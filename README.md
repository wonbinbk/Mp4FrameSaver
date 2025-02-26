## Mp4 Frame Saver ##

### Introduction ###

Mp4FrameSaver is a C++ project demonstrating a set of micro-services that
together achieve the following purpose: given a MP4 video file, extract and
resize the frames of the video and save the resized frames on disk. The
requirements are to have the operation as several micro services:
- Frame Publisher: to read the video file, extract its frames and send the
  frames to a queue.
- Frame Resizer: read frames from the above queue, resize them and send them to
  another queue.
- Frame Saver: read frames from the second queue and save them to disk. Also,
  it must notify the Frame Publisher so that the next frame can be sent.

For convenience, a small C++ client is also included. The purpose of this
client is to provide a path of a video file to the Mp4FrameSaver service. The
client also needs a way to know if the video has been processed successfully or
the service is busy.

### Message queue design ###

Mp4FrameSaver contains 3 services: FramePublisher, FrameResizer, FrameSaver.
The 3 services inherit from a Service class because they all need a seperate
thread to listen for event on a message queue and handle it.

The message queues can be POSIX queue or more modern technology (such as MQTT).
However, for the purpose of this project, POSIX queues are simpler, easier to
use.

With POSIX queues, we can have an messages flow in one queue that each services
will have to poll for new event and peak the message to see if it's for them or
not (without removing the message from the queue).

Another way is to have separate message queues for each services. This way, the
design is much simpler, each service just needs a blocking poll for events on its own
InQueue and send messages out on its OutQueue.

Since the target of messages for each services are clear, we do not need a router mechanism.

As such, the 3 services and their message queues are designed as below:

    --> PUBLISHER_IN_QUEUE --> FramePublisher --> RESIZER_IN_QUEUE
    --> RESIZER_IN_QUEUE   --> FrameResizer   --> SAVER_IN_QUEUE
    --> SAVER_IN_QUEUE     --> FrameSaver     --> PUBLISHER_IN_QUEUE

The client can send video file path to the FramePublisher with a queue.

    --> CLIENT_IN_QUEUE    --> Client         --> PUBLISHER_IN_QUEUE

The FramePublisher needs a way to tell the client that the task has been done, successfully or not. This is done via a callback. We also use POSIX queue for this callback.

                               FramePublisher --> CLIENT_IN_QUEUE

### Message protocol ###

Since the project is pretty simple, we use normal text and JSON for messages.
The crud of the "message" flowing between services is the frames itself. However, we do not know the size limit of the frame but POSIX message queue does have a limit in size, sending the whole frame via a message is inefficient.

To efficiently transafer the frames between services, we should:
- Put the frames on dedicated shared memory locations (which were agreed upon between services).
- On the message, only send enough information so that the receiver can recompose the frame from the binary image on shared locations. These information are called _metadata_.

#### Shared memory ####

The following shared memory locations are reserved for the 3 services:

    FramePublisher --write to--> [SHM_FRAME] --read to--> FrameResizer --write to--> [SHM_RESIZED_FRAME] --read to--> FrameSaver

#### Message details ####

struct Message:
    - vidPath : (string) input video path: used to read the video file, use the file name as sub-directory to save frames to.
    - type    : (enum) type of message: currently only ACK type is used. But we can expand on this if required.
    - struct FrameInfo:
        - rows : (int) number of rows of the frame.
        - columns: (int) number of columns of the frame.
        - channels: (int) number of channels of the frame.
        - type: (int) type of pixel. This is an enum value used by OpenCV to reconstruct the image from pure pixel.

The message struct above can be converted to a JSON string using helper functions `frameInfoToJson` and `jsonToFrameInfo`.

### Logging ###

We use __spdlog__ as the logging system. All logs should be printed asynchronously to standard output. The reason for choosing __spdlog__ is because it's a power but simple to use logging system.

### Build and usage ###

The project uses CMake as its build system.
To build:
```bash
mkdir -p build && cd build
cmake ..
make -j4
```
The build will create 2 binaries: `mp4FrameSaver` and `extractMp4Frames`.
`mp4FrameSaver` is meant to be used as a service and it will create all the message queues and shared memory locations. To run it as a background process:
```bash
./mp4FrameSaver ${HOME}/ResizedFrames &
```
The services should be running and listening for new video file path on its input queueu. Then you can run the client with:
`extractMp4Frames PATH_TO_VIDEO_FILE`
