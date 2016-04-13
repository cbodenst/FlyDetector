#ifndef FLYCOUNTER_H
#define FLYCOUNTER_H

#include "vials.h"
#include <opencv2/opencv.hpp>

typedef std::vector<Color> Colors;

class FlyCounter
{
protected:
    int   epsilon;
    int   minPoints;
    int   numberOfFlies;
    int   pixelsPerFly;
    int   threshold;

public:
    FlyCounter();

    /* External API */
    int count(const cv::Mat& img, Vials& vials);
    cv::Mat generateThresholdImage(const cv::Mat& img);
    cv::Mat generateClusterImage(const cv::Mat& thresh, Vials &vials);
    int countFlies(const cv::Mat & threshImg, Vials &vials);

    /* Getters */
    int getEpsilon();
    int getMinPoints();
    int getPixelsPerFly();
    int getThreshold();

    /* Setters */
    void setEpsilon(int value);
    void setMinPoints(int value);
    void setPixelsPerFly(int value);
    void setThreshold(int value);

    /* Color Map */
    static Colors COLORS;
};

#endif // FLYCOUNTER_H
