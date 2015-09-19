#ifndef SAMPLECAM_H
#define SAMPLECAM_H

#include "opencv2/opencv.hpp"

#include "flycam.h"

class SampleCam : public FlyCam
{
public:
    SampleCam(std::string folder, int frames);
    ~SampleCam();

    virtual bool getImage(cv::Mat& mat);
    virtual bool setFocus(int){return false;}

///MEMBERS///
public:
    int currentFrame;
    int totalFrames;
    std::string folderPath;

};


#endif // SAMPLECAM_H
