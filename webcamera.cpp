#include "webcamera.h"

WebCamera::WebCamera()
    : cam(cv::VideoCapture(0))
{
    try
    {
        if (cam.isOpened())
        {
            //Check if this is really working
            cv::Mat test;
            cam >> test;
            this->accessable = true;

            cam.set(CAP_PROP_FRAME_WIDTH,  WebCamera::IMAGE_WIDTH);
            cam.set(CAP_PROP_FRAME_HEIGHT, WebCamera::IMAGE_HEIGHT);
            cam.set(CAP_PROP_FOCUS,        WebCamera::DEFAULT_FOCUS);
        }
        else
        {
            this->accessable = false;
        }

    }
    catch (const cv::Exception& e)
    {
        this->accessable = false;
    }
}

bool WebCamera::getImage(cv::Mat& mat)
{
    return cam.read(mat);
}

bool WebCamera::setFocus(int value)
{
    return cam.set(CAP_PROP_FOCUS, value);
}
