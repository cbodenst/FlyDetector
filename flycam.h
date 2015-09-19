#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>

/*Abstract class for defining different types of cameras*/
class FlyCam
{
public:
    FlyCam();
    virtual ~FlyCam();
    virtual bool getImage(cv::Mat& mat) = 0;
    bool accessable=false;
    virtual bool setFocus(int value) = 0;
};

#endif // CAMERA_H
