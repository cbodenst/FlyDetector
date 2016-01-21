#ifndef WEBCAMERA_H
#define WEBCAMERA_H

#include <opencv2/opencv.hpp>

#include "cam.h"

class WebCamera : public Cam
{
    cv::VideoCapture cam;

public:
    static const int IMAGE_WIDTH   = 1080;
    static const int IMAGE_HEIGHT  =  720;
    static const int DEFAULT_FOCUS =  110;

    WebCamera();

    virtual bool getImage(cv::Mat& mat);
    virtual bool setFocus(int value);

    virtual ~WebCamera();
};

#endif // WEBCAMERA_H
