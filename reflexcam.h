#ifndef REFLEXCAM_H
#define REFLEXCAM_H

#include "flycam.h"

#include <gphoto2/gphoto2-camera.h>

class ReflexCam : public FlyCam
{
    Camera* cam;
    GPContext* context;
public:
    ReflexCam();
    virtual ~ReflexCam();
    virtual bool getImage(cv::Mat& mat);
    virtual bool setFocus(int);
protected:
    void error_func(GPContext *context, const char *format, va_list args, void *data);
    void message_func(GPContext *context, const char *format, va_list args, void *data);
};

#endif // REFLEXCAM_H
