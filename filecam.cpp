#include <sstream>

#include "filecam.h"

FileCam::FileCam(const std::string& folder, int frames)
    : Cam(true), currentFrame(0), totalFrames(frames), path(folder)
{}

bool FileCam::getImage(cv::Mat &mat)
{
    std::stringstream output;
    output << this->path << "/" <<  this->currentFrame << ".jpg";
    mat = cv::imread(output.str());
    if (mat.empty())
    {
        return false;
    }

    ++this->currentFrame;
    this->currentFrame %= this->totalFrames;

    return true;
}
