#include "webcamera.h"

WebCamera::WebCamera()
    : cam(0)
{
    try
    {
        if (this->cam.isOpened())
        {
            // check whether we can read an image from the camera
            cv::Mat test;
            this->cam >> test;
            this->accessable = true;

            // setup the camera properties (image dimensions and camera focus)
            this->cam.set(CV_CAP_PROP_FRAME_WIDTH,  WebCamera::IMAGE_WIDTH);
            this->cam.set(CV_CAP_PROP_FRAME_HEIGHT, WebCamera::IMAGE_HEIGHT);
            this->cam.set(CV_CAP_PROP_FOCUS,        WebCamera::DEFAULT_FOCUS);
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
    return this->cam.read(mat);
}

bool WebCamera::setFocus(int value)
{
    return this->cam.set(CV_CAP_PROP_FOCUS, value);
}

WebCamera::~WebCamera()
{
    this->cam.release();
}
