#ifndef VIALS_H
#define VIALS_H

#include <vector>
#include <math.h>

#include <opencv2/opencv.hpp>

struct Vial
{
    std::vector<cv::Point> pts;
    cv::Point center;
    int area;
    int radius;
    int flyCount;

    Vial(const std::vector<cv::Point>& points)
        :
        pts(points),
        center(cv::Point(0,0)),
        area(M_PI),
        radius(1),
        flyCount(0)
    {
        // Compute mean and radius
        for(cv::Point point : points)
            center += point;
        center.x /= points.size();
        center.y /= points.size();
        area = cv::contourArea(pts);

        radius = sqrt(area/M_PI);
    }
};


typedef std::vector<Vial> Vials;

static const int VIAL_TOLERANCE = 20; //px

bool    compareVials(const Vial& first, const Vial& second);
cv::Mat drawVials(const Vials& vials, const cv::Mat& image);
Vials   findVials(const cv::Mat& image, int vialSize);

#endif // VIALS_H
