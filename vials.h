#ifndef VIALS_H
#define VIALS_H

#include <vector>

#include <opencv2/opencv.hpp>

typedef cv::Vec3f         Vial;
typedef std::vector<Vial> Vials;

static const int VIAL_TOLERANCE = 20; //px

bool    compareVials(const Vial& first, const Vial& second);
cv::Mat drawVials(const Vials& vials, const cv::Mat& image);
Vials   findVials(const cv::Mat& image, int vialSize);

#endif // VIALS_H
