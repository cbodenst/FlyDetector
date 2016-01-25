#ifndef FILECAM_H
#define FILECAM_H

#include <string>

#include <QStringList>
#include <opencv2/opencv.hpp>

#include "cam.h"

/* "Special camera" that reads images from disk */
class FileCam : public Cam
{
private:
    const std::string path;
    QStringList       images;

public:
    FileCam(const std::string& folder);
    virtual bool getImage(cv::Mat& image);
};

#endif // FILECAM_H
