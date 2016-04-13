#include "flycounter.h"

#include "dbscan/hpdbscan.h"

FlyCounter::FlyCounter()
:
epsilon(0),
minPoints(0),
pixelsPerFly(0),
threshold(0)
{

}


Colors FlyCounter::COLORS {
    Color( 95, 162,  56),
    Color(204,  80, 223),
    Color(217,  62,  56),
    Color(140, 207, 216),
    Color(218,  66, 135),
    Color(224, 146,  46),
    Color(208, 154, 201),
    Color(117, 110, 209),
    Color(138, 113,  88),
    Color(194, 216, 148),
    Color(210, 211,  71),
    Color(107, 227, 181),
    Color(106,  55, 117),
    Color(206, 188, 176),
    Color(129, 224,  58),
    Color(216, 128, 125),
    Color(130, 110,  32),
    Color(117, 161, 213),
    Color( 74, 113, 107),
    Color( 74, 113,  53),
    Color(177,  90,  49),
    Color( 91, 100, 138),
    Color(151,  62,  93),
    Color(205, 172,  97),
    Color(198,  89, 185),
    Color(116, 222, 118),
    Color(100, 170, 137)
};


/* analysis parameter setters */
int FlyCounter::getEpsilon()
{
    return this->epsilon;
}

int FlyCounter::getMinPoints()
{
    return this->minPoints;
}

int FlyCounter::getPixelsPerFly()
{
    return this->pixelsPerFly;
}

int FlyCounter::getThreshold()
{
    return this->threshold;
}


/* Methods for external usage*/
int FlyCounter::count(const cv::Mat& img, Vials& vials)
{
    cv::Mat thresh = this->generateThresholdImage(img);
    int num_flies = countFlies(thresh, vials);
    return num_flies;
}

cv::Mat FlyCounter::generateThresholdImage(const cv::Mat &img)
{
    cv::Mat ret;
    cv::cvtColor(img, ret, CV_RGB2GRAY);
    cv::threshold(ret, ret, this->threshold, 255, CV_THRESH_BINARY_INV);
    return ret;
}

cv::Mat FlyCounter::generateClusterImage(const cv::Mat &img, Vials &vials)
{
    std::map<int, int> colorMap;
    cv::Mat clusterImg = cv::Mat(img.size(), img.type(), cv::Scalar(0));
    int colorIndex = 0;

    /* draw colored flies on the image */
    for (Vial& vial : vials)
    {
        for (auto size : vial.clusterSizes)
        {
            if (size.first == 0) continue;
            if (colorMap.find(size.first) == colorMap.end())
            {
                colorMap[size.first] = colorIndex;
                ++colorIndex;
            }
        }

        for (unsigned int i = 0; i < vial.labels.size(); ++i)
        {

            if (std::abs(vial.labels[i]) == 0) continue;
            Color color = COLORS[colorMap[std::abs(vial.labels[i])] % COLORS.size()];
            cv::Vec2f coord = vial.flyPixels.at<cv::Vec2f>(0, i);
            clusterImg.at<cv::Vec3b>(coord[1],coord[0]) = color;
        }
    }
    return clusterImg;
}

int FlyCounter::countFlies(const cv::Mat& threshImg, Vials& vials)
{
    int flies_total = 0;
    cv::Mat flies;
    for (Vial& vial : vials)
    {
        /* mask the vails */
        cv::Mat mask =  cv::Mat(threshImg.size(), threshImg.type(), cv::Scalar(0));
        cv::drawContours(mask, std::vector<std::vector<cv::Point>>(1,vial.pts), 0, cv::Scalar(255, 255, 255), -1);
        cv::bitwise_and(mask, threshImg, flies);

        /* get their pixel coordinates in an array */
        cv::findNonZero(flies, vial.flyPixels);
        vial.flyPixels.convertTo(vial.flyPixels, CV_32FC2);

        /* cluster the white pixels using DBSCAN */
        int numberOfPixels = vial.flyPixels.size().height;
        Cluster labels[numberOfPixels];


        HPDBSCAN dbscan((float*) vial.flyPixels.data, numberOfPixels, 2 /* dimensions */);
        dbscan.scan(this->epsilon, this->minPoints, labels);

        vial.labels = std::vector<Cluster>(labels, labels + numberOfPixels);

        /* accumulate the number of pixels belonging to one cluster */
        vial.clusterSizes.clear();
        for (int i = 0; i < numberOfPixels; ++i)
        {
            ++vial.clusterSizes[std::abs(labels[i])];
        }
        /* count the flies based on the clusters and color them in the cluster image */
        vial.flyCount = 0;
        for (auto size : vial.clusterSizes)
        {
            if (size.first == 0) continue;
            vial.flyCount += (int)std::ceil((float)size.second / (float)this->pixelsPerFly);
        }
        flies_total += vial.flyCount;
    }
    return flies_total;
}

/* analysis parameters */
void FlyCounter::setEpsilon(int value)
{
    this->epsilon = value;
}

void FlyCounter::setMinPoints(int value)
{
    this->minPoints = value;
}

void FlyCounter::setPixelsPerFly(int value)
{
    this->pixelsPerFly = value;
}

void FlyCounter::setThreshold(int value)
{
    this->threshold = value;
}
