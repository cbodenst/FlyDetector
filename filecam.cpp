#include <sstream>

#include "filecam.h"

FileCam::FileCam(const std::string& folder, int frames)
    : accessable(true), path(folder), currentFrame(0), totalFrames(frames)
{}

bool FileCam::getImage(cv::Mat &mat)
{
    std::stringstream output;
    output << this->path << "/" <<  this->currentFrame << ".jpg";
    mat = cv::imread(output.str());
    if (mat.data() == nullptr)
    {
        return false;
    }

    ++this->currentFrame;
    this->currentFrame %= this->totalFrames;

    return true;
}
