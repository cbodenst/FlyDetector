#include "flycounter.h"

#include <QDir>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>

#include "dbscan/hpdbscan.h"
#include "filecam.h"
#include "reflexcam.h"
#include "webcamera.h"

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

FlyCounter::FlyCounter(QObject* parent,
                       const Duration& leadTime,
                       const Duration& roundTime,
                       const Duration& shakeTime,
                       int epsilon,
                       int minPoints,
                       int pixelsPerFly,
                       int threshold,
                       int vialSize,
                       const std::string& outputPath,
                       bool saveImages)
  : QObject(parent),

    // experiment data
    leadTime(leadTime),
    roundTime(roundTime),
    shakeTime(shakeTime),

    // analysis parameters
    epsilon(epsilon),
    minPoints(minPoints),
    pixelsPerFly(pixelsPerFly),
    threshold(threshold),
    vialSize(vialSize),

    // results
    output(outputPath),
    saveImages(saveImages),

    // threaded execution
    running(false)
{
    /* setup initial images */
    this->detectCamera();
    this->updateImages();

    /* create ouput directory */
    QDir().mkdir(QString::fromStdString(this->output));
}

/* image analysis mainloop */
void FlyCounter::process()
{
    Timepoint start   = Clock::now();
    Timepoint measure = start + this->roundTime;
    Timepoint shake   = measure - this->leadTime;

    while (this->running)
    {
        Timepoint current = Clock::now();

        // Time to shake?
        if (current > shake)
        {
            shake += this->roundTime;
            this->shaker.shakeFor(this->shakeTime);
        }

        // Time to take a picture?
        if (current > measure)
        {
            measure += this->roundTime;
            this->updateImages();
            this->writeResults(current);

            if (this->saveImages)
            {
                this->writeImage(current);
            }
        }

        int elapsed = convertToInt(current - start);
        emit timeUpdate(QString::number(elapsed) + "s");
    }
}

/* detect the built-in cameras; priorities: reflex, webcam, file */
void FlyCounter::detectCamera()
{
    // prefer reflex camera
    this->camera = new ReflexCam();
    if (this->camera->isAccessable())
    {
        return;
    }

    // reflex camera not available, try built-in webcam
    delete this->camera;
    this->camera = new WebCamera();
    if (this->camera->isAccessable())
    {
        return;
    }

    // webcam is not available, fallback to "camera" reading from disk
    delete this->camera;
    this->camera = new FileCam("new", 1);
}

/* converts a chrono timepoint to a string */
std::string FlyCounter::toString(const Timepoint& time)
{
    auto a_time_t = Clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&a_time_t), "%Y-%m-%d %H:%M:%S");

    return ss.str();
}

/* fetches new image from the camera */
void FlyCounter::updateCameraImage()
{
    if (!this->camera->getImage(this->cameraImage))
    {
        // TODO: debug
        return;
    }
    cv::cvtColor(this->cameraImage, this->cameraImage, CV_BGR2RGB);
    this->vials = findVials(this->cameraImage, this->vialSize);
}

void FlyCounter::updateClusterImage()
{
    int index = 0;
    std::map<int, int> clusterSizes;
    std::map<int, int> colorMap;
    this->clusterImage = cv::Mat(this->cameraImage.size(), this->cameraImage.type(), cv::Scalar(0));

    for (Vial vial : this->vials)
    {
        cv::Mat flies;
        cv::Mat flyPixels;
        int     colorIndex = 0;
        cv::Mat mask = this->clusterImage.clone();

        /* mask the veils */
        cv::circle(mask, cv::Point(vial[0],vial[1]), vial[2], cv::Scalar(255, 255, 255), -1 /* filled */);
        cv::bitwise_and(mask, this->thresholdImage, flies);

        /* get their pixel coordinates in an array */
        cv::findNonZero(flies, flyPixels);
        flyPixels.convertTo(flyPixels, CV_32FC2);

        /* cluster the white pixels using DBSCAN */
        int numberOfPixels = flyPixels.size().height;
        Cluster labels[numberOfPixels];

        HPDBSCAN dbscan((float*) flyPixels.data, numberOfPixels, 2 /* dimensions */);
        dbscan.scan(this->epsilon, this->minPoints, labels);

        /* accumulate the number of pixels belonging to one cluster */
        clusterSizes.empty();
        for (int i = 0; i < numberOfPixels; ++i)
        {
            ++clusterSizes[std::abs(labels[i])];
        }

        /* count the flies based on the clusters and color them in the cluster image */
        this->flies.resize(this->vials.size());
        for (auto size : clusterSizes)
        {
            if (size.first == 0) continue;
            this->flies[index] += (int)std::ceil(size.second / this->pixelsPerFly);

            if (colorMap.find(size.first) == colorMap.end())
            {
                colorMap[size.first] = colorIndex;
                ++colorIndex;
            }
        }

        /* draw colored flies on the image */
        for (int i = 0; i < numLabels[index]; ++i)
        {
            Color color = labels[abs(this->labels[index][i])];
            cv::Vec2f coord = this->coords[index].at<cv::Vec2f>(0,i);
            // TODO: choose different color for cluster that are larger then pixelsPerFly - how?
            this->clusterImage.at<cv::Vec3b>(coord[1],coord[0]) = color;
        }
    }
}

/* update the threshold image from the currently set camera image */
void FlyCounter::updateThresholdImage()
{
    cv::Mat greyscaleImage;
    cv::cvtColor(this->cameraImage, greyscaleImage, CV_RGB2GRAY);
    cv::threshold(greyscaleImage, this->thresholdImage, this->threshold, 255, CV_THRESH_BINARY_INV);
}

/* stores the fly image */
void FlyCounter::writeImage(const Timepoint& time)
{
    std::stringstream path;
    path << this->output << "/" << this->toString(time) << ".jpg";
    // TODO: debug
    this->imageLock.lock();
    cv::imwrite(path.str(), this->cameraImage);
    this->imageLock.unlock();
}

/* output the fly counts in a tab-separated list into a file, leading value is the collection timestamp */
void FlyCounter::writeResults(const Timepoint& time)
{
    std::ofstream file(this->output + "/" + this->output, std::ios::app);
    // TODO: debug
    file << this->toString(time) ;
    for (int count : this->flies)
    {
        file << "\t" << count;
    }
    file << std::endl;
    file.close();
}

/** public **/

/* image getters */
const cv::Mat& FlyCounter::getCameraImage()
{
    return this->cameraImage;
}

const cv::Mat& FlyCounter::getThresholdImage()
{
    return this->thresholdImage;
}

const cv::Mat& FlyCounter::getClusterImage()
{
    return this->clusterImage;
}

/* timer getters */
const Duration& FlyCounter::getLeadTime()
{
    return this->leadTime;
}

const Duration& FlyCounter::getRoundTime()
{
    return this->roundTime;
}

const Duration& FlyCounter::getShakeTime()
{
    return this->shakeTime;
}

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

int FlyCounter::getVialSize()
{
    return this->vialSize;
}

bool FlyCounter::isRunning()
{
    return this->running;
}

const Vials& FlyCounter::getVials()
{
    return this->vials;
}

/* results */
const std::string& FlyCounter::getOutput()
{
    return this->output;
}

const Flies& FlyCounter::getFlies()
{
    return this->flies;
}

/* validated time setters - adjust the respective two other timers according to the passed individual timer */
/* e.g. round time set to 2 -> shake time and lead time have been 5s and 6s before and get changed to 1s */
void FlyCounter::validatedSetLeadTime(const Duration& time)
{
    if (time <= Seconds(0))
    {
        return;
    }

    this->leadTime = time;
    Duration sum = this->leadTime + this->shakeTime;
    if (sum > this->roundTime)
    {
        this->roundTime = sum;
    }
}

void FlyCounter::validatedSetRoundTime(const Duration& time)
{
    int intTime = convertToInt(time);
    if (intTime <= 0)
    {
        return;
    }

    this->roundTime = time;
    if (this->shakeTime + this->leadTime > time)
    {
        this->leadTime  = Seconds(intTime / 2);
        this->shakeTime = Seconds(intTime / 2);
    }
}

void FlyCounter::validatedSetShakeTime(const Duration& time)
{
    if (time <= Seconds(0))
    {
        return;
    }

    this->shakeTime = time;
    Duration sum = this->shakeTime + this->leadTime;
    if (sum > this->roundTime)
    {
        this->roundTime = sum;
    }
}

/* start the fly counter */
void FlyCounter::start()
{
    if (!this->running)
    {
        this->running = true;
        this->thread  = std::thread(&FlyCounter::process, this);
    }
}

/* stop the fly counter */
void FlyCounter::stop()
{
    this->running = false;
    this->thread.join();
}

/* execution */
void FlyCounter::lock()
{
    this->imageLock.lock();
}

void FlyCounter::unlock()
{
    this->imageLock.unlock();
}

void FlyCounter::updateImages()
{
    this->imageLock.lock();
    this->updateCameraImage();
    this->updateThresholdImage();
    this->updateClusterImage();
    this->imageLock.unlock();

    emit countUpdate(QString::number(std::accumulate(this->flies.begin(), this->flies.end(), 0)));
    emit imageUpdate();
}

/* destructor */
FlyCounter::~FlyCounter()
{
    delete this->camera;
    for (auto label : labels)
    {
        delete[] label;
    }
}

/** public slots **/

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

void FlyCounter::setVialSize(int value)
{
    this->vialSize = value;
}

/* result setters */
void FlyCounter::setOutput(const std::string& out)
{
    this->output = out;
}

void FlyCounter::storeImages(bool value)
{
    this->saveImages = value;
}
