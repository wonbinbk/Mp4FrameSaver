#ifndef UTILS_H
#define UTILS_H
#include <opencv2/core/mat.hpp>
#include <nlohmann/json.hpp>

enum class MESSAGE_TYPE_ENUMS : uint8_t {
    ACK,
    // More message types can be added but for now, we only need ACK
    SENTINEL,
};

struct Message {
    std::string vidPath; // input video file path
    MESSAGE_TYPE_ENUMS type;    // type of message, as of now, only ACK type is used
    struct FrameInfo {
        int rows;
        int columns;
        int channels;
        int type;
        FrameInfo(int rows, int columns, int channels, int type) : rows(rows), columns(columns), channels(channels), type(type) {}
        FrameInfo(const cv::Mat& mat) : rows(mat.rows), columns(mat.cols), channels(mat.channels()), type(mat.type()) {}
        FrameInfo() {};
	// Helpful macro: ref: https://json.nlohmann.me/api/macros/nlohmann_define_type_intrusive/
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(FrameInfo, rows, columns, channels, type);
    } frameInfo;
    Message(const std::string& vidPath, MESSAGE_TYPE_ENUMS type, int rows, int columns, int channels, int pixelType)
        : vidPath(vidPath), type(type), frameInfo(rows, columns, channels, pixelType) {};
    Message(const std::string& vidPath, MESSAGE_TYPE_ENUMS type, const cv::Mat& mat) : vidPath(vidPath), type(type), frameInfo(mat) {};
    Message() {};
};

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

void check_utf8(const std::string& str);
std::string frameInfoToJson(const Message& info);
Message jsonToFrameInfo(const std::string& json);
} // namespace Utils::Json

#endif // UTILS_H
