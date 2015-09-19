#ifndef CVCAMERA_H
#define CVCAMERA_H

#include "flycam.h"


class WebCamera : public FlyCam
{
    cv::VideoCapture cam;
public:
    WebCamera();
    virtual bool getImage(cv::Mat& mat);
    virtual bool setFocus(int value);
};

#endif // CVCAMERA_H
