#ifndef UTILS_H
#define UTILS_H
#include <opencv2/core/mat.hpp>

namespace Utils::Shm {
bool writeFrameToShm(const cv::Mat& frame, int shmFd);
bool readFrameFromShm(cv::Mat& frame, int shmFd);
} // namespace Utils::Shm

namespace Utils::Json {
/*
 * Messages between Frame... services need to carry a bunch of information such as
 * Frame image data shared memory name: string
 * Metadata of the frame so that the receiver can reconstruct a cv::Mat object from the raw pixel data:
 * 	- video file name: this information is used to create a directory with the same name under the output directory. For output directory in ~/frames/, a
 * video named `big_buck_bunny.mp4` can have all extracted and resized frames under `~/frames/big_buck_bunny/` directory.
 * 	- rows: frame rows.
 * 	- columns: frame columns.
 * 	- channels: frame channels.
 * 	- type:	frame pixel type.
 * For simplicity, we use json as message protocol here but we can think of ProtoBuf if we want to be more efficient.
 */
struct Message {
    std::string type;    // type of message, as of now, it's only ACK
    std::string shmName; // shared memory name
    std::string vidPath; // input video file path
    struct Metadata {
        int rows;
        int columns;
        int channels;
        int type;
    } metadata;
};

std::string frameInfoToJson(const Message& info);
Message jsonToFrameInfo(const std::string& json);
} // namespace Utils::Json

#endif // UTILS_H
