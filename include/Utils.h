#ifndef UTILS_H
#define UTILS_H
#include <opencv2/core/mat.hpp>

namespace Utils {
    bool writeFrameToShm(const cv::Mat& frame, int shmFd);
    bool readFrameFromShm(cv::Mat& frame, int shmFd);
};

#endif // UTILS_H
