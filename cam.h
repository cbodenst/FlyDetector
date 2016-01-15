#ifndef CAM_H
#define CAM_H

#include <opencv2/opencv.hpp>

/* Abstract camera interface */
class Cam
{
protected:
    bool accessable = false;

public:
    /* Accessort for camera accesability */
    virtual inline bool isAccessable() { return this->accessable; }

    /* Reads an image from the camera into the mat parameter and returns true if successful, false otherwise */
    virtual bool getImage(cv::Mat& mat) = 0;

    /* Focuses the camera to the passed focus length */
    virtual bool setFocus(int value) = 0;
};

#endif // CAM_H
