#include "webcamera.h"

WebCamera::WebCamera()
{
    try
    {
        cam = cv::VideoCapture(0);
        if(cam.isOpened())
        {
            //Check if this is really working
            cv::Mat test;
            cam >> test;
            this->accessable=true;
            cam.set(3,1080);
            cam.set(4,720);
            cam.set(28,110); //FOCUS
        }

    }
    catch(cv::Exception)
    {
        this->accessable=false;
    }
}

bool WebCamera::getImage(cv::Mat& mat)
{
    return cam.read(mat);
}

bool WebCamera::setFocus(int value)
{
    cam.set(28,value);
    return true;
}

