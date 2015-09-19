#include "samplecam.h"

SampleCam::SampleCam(std::string folder, int frames)
{
    this->accessable=true;
    this->folderPath=folder;
    this->totalFrames=frames;
    this->currentFrame=0;
}

SampleCam::~SampleCam()
{

}

bool SampleCam::getImage(cv::Mat &mat)
{
    std::stringstream output;
    output <<  this->folderPath <<"/" <<  this->currentFrame << ".jpg";
    mat = cv::imread(output.str());
    this->currentFrame++;
    this->currentFrame %=  this->totalFrames;

    return true;
}


