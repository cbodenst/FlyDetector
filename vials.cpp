#include <algorithm>
#include <cmath>
#include <string>

#include "vials.h"

static const cv::Scalar VIAL_COLOR  = cv::Scalar(0, 255, 0);
static const int        VIAL_STROKE = 5;
static const int        FONT        = CV_FONT_HERSHEY_SIMPLEX;
static const int        FONT_SCALE  = 1;
static const int        FONT_STROKE = 4;

bool compareVials(const Vial& first, const Vial& second)
{
    int threshold = first.radius;
    if (std::abs(first.center.y - second.center.y) < threshold)
    {
        return first.center.x < second.center.x;
    }

    return first.center.y < second.center.y;
}

cv::Mat drawVials(const Vials& vials, const cv::Mat& image)
{
    if (image.empty())
        return image;
    int baseline;
    int counter = 0;
    cv::Mat vialImage = image.clone();

    for (Vial vial : vials)
    {
        std::string vialNumber = std::to_string(counter);
        cv::Size fontSize = cv::getTextSize(vialNumber, FONT, FONT_SCALE, FONT_STROKE, &baseline);

        cv::drawContours(vialImage, std::vector<std::vector<cv::Point>>(1,vial.pts), 0, VIAL_COLOR, VIAL_STROKE);
        cv::Point center = vial.center;
        center.x -= fontSize.width/2,
        center.y += fontSize.height/2;
        cv::putText(vialImage, vialNumber, center, FONT, FONT_SCALE, VIAL_COLOR, FONT_STROKE);
        ++counter;
    }

    return vialImage;
}

Vials findVials(const cv::Mat& image, int vialSize)
{
    // Bluescreen image
    Vials vials;
    cv::Mat hsv;
    cv::cvtColor(image,hsv, CV_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(0,50,0), cv::Scalar(100,255,255), hsv);

    cv::Mat rez;
    cv::resize(hsv,rez,cv::Size(1500,1000));

    cv::imshow("HSV",rez);

    // Pad image
    cv::Mat padded;
    int padding = 5;
    padded.create(hsv.rows + 2*padding, hsv.cols + 2*padding, hsv.type());
    padded.setTo(cv::Scalar::all(255));
    hsv.copyTo(padded(cv::Rect(padding, padding, hsv.cols, hsv.rows)));

    // Find vial contours
    std::vector< std::vector<cv::Point> > contours;
    //cv::bitwise_not(padded,padded);
    cv::dilate(padded, padded, cv::Mat(),cv::Point(-1,-1),1);
    cv::findContours( padded, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE,cv::Point(-5,-5));
    int vialArea = vialSize*vialSize*M_PI;
    int maxArea = vialArea*1.33;
    int minArea = vialArea*0.66;
    for (unsigned int i = 0; i < contours.size(); i++)
    {
        if(contourArea(contours[i]) > minArea && contourArea(contours[i]) < maxArea)
            vials.push_back(Vial(contours[i]));
    }
    std::sort(vials.begin(), vials.end(), compareVials);

    return vials;
}
