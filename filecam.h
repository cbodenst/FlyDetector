#ifndef FILECAM_H
#define FILECAM_H

#include <opencv2/opencv.hpp>
#include <string>

#include "cam.h"

/* "Special camera" that reads images from disk */
class FileCam : public Cam
{
    int currentFrame;
    int totalFrames;

    std::string path;

public:
    FileCam(const std::string& folder, int frames);
    virtual bool getImage(cv::Mat& mat);
};

#endif // FILECAM_H
