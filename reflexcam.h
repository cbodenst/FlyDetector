#ifndef REFLEXCAM_H
#define REFLEXCAM_H

#include <gphoto2/gphoto2-camera.h>
#include <opencv2/opencv.hpp>

#include "cam.h"

class ReflexCam : public Cam
{
protected:
    Camera*    cam;
    GPContext* context;

public:
    ReflexCam();
    virtual bool getImage(cv::Mat& mat);
    virtual ~ReflexCam();
};

#endif // REFLEXCAM_H
