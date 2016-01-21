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
    int threshold = first[2];
    if (std::abs(first[1] - second[1]) < threshold)
    {
        return first[0] < second[0];
    }

    return first[1] < second[1];
}

cv::Mat drawVials(const Vials& vials, const cv::Mat& image)
{
    int baseline;
    int counter = 0;
    cv::Mat vialImage = image.clone();

    for (Vial vial : vials)
    {
        std::string vialNumber = std::to_string(counter);
        cv::Size fontSize = cv::getTextSize(vialNumber, FONT, FONT_SCALE, FONT_STROKE, &baseline);

        cv::circle(vialImage, cv::Point(vial[0], vial[1]), vial[2], VIAL_COLOR, VIAL_STROKE);
        cv::putText(vialImage, vialNumber, cv::Point(vial[0] - fontSize.width / 2, vial[1] + fontSize.height / 2), FONT, FONT_SCALE, VIAL_COLOR, FONT_STROKE);
        ++counter;
    }

    return vialImage;
}

Vials findVials(const cv::Mat& image, int vialSize)
{
    Vials vials;
    cv::Mat grayscaleImage;
    cv::cvtColor(image, grayscaleImage, CV_RGB2GRAY);
    cv::HoughCircles(grayscaleImage, vials, CV_HOUGH_GRADIENT, 2, vialSize * 2, 100, 100, vialSize - VIAL_TOLERANCE, vialSize + VIAL_TOLERANCE);
    std::sort(vials.begin(), vials.end(), compareVials);

    return vials;
}
