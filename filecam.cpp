#include <sstream>

#include <QDir>
#include <QString>

#include "filecam.h"

FileCam::FileCam(const std::string& folder)
  : Cam(true), path(folder)
{
    QStringList filters;
    filters << "*.jpg" << "*.jpeg";

    this->images = QDir(QString::fromStdString(folder)).entryList(filters, QDir::NoFilter, QDir::Time | QDir::Reversed);
}

bool FileCam::getImage(cv::Mat& image)
{
    if (this->images.isEmpty())
    {
        image = cv::Mat();
        return false;
    }

    std::stringstream imagePath;
    imagePath << this->path << "/" << this->images.takeFirst().toStdString();
    image = cv::imread(imagePath.str());
    return true;
}
