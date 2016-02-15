#include "flycounter.h"

#include <QDir>
#include <QStandardPaths>
#include <QTemporaryFile>

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>

#include "dbscan/hpdbscan.h"
#include "filecam.h"
#include "logger.h"
#include "reflexcam.h"
#include "noshaker.h"
#include "usbshaker.h"
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

FlyCounter::FlyCounter(QObject* parent)
  : QObject(parent),

    // experiment parameters
    leadTime(Seconds(0)),
    roundTime(Seconds(0)),
    shakeTime(Seconds(0)),

    // analysis parameters
    epsilon(0),
    minPoints(0),
    pixelsPerFly(0),
    threshold(0),
    vialSize(0),

    // results
    output(QTemporaryFile().fileName().toStdString()),
    saveImages(false),
    // devices
    camera(nullptr),
    shaker(nullptr),

    // threaded execution
    running(false)

{}

/* image analysis mainloop */
void FlyCounter::process()
{
    this->experimentStart = Clock::now();
    Timepoint measure     = this->experimentStart + this->roundTime;
    Timepoint shake       = measure - this->leadTime;

    while (this->running)
    {
        Timepoint current = Clock::now();
        int elapsed = convertToInt(current - this->experimentStart);

        // Time to shake?
        if (current > shake)
        {
            shake += this->roundTime;
            this->shaker->shakeFor(this->shakeTime);
        }

        // Time to take a picture?
        if (current > measure)
        {
            measure += this->roundTime;
            this->updateImages();
            this->writeResults(elapsed);
            if (this->saveImages)
            {
                this->writeImage(elapsed);
            }
        }

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
        Logger::info("Detected reflex camera");
        return;
    }

    // reflex camera not available, try built-in webcam
    delete this->camera;
    Logger::warn("Could not find reflex camera");

    this->camera = new WebCamera();
    if (this->camera->isAccessable())
    {
        Logger::info("Detected web camera");
        return;
    }

    // webcam is not available, fallback to "camera" reading from disk
    delete this->camera;
    Logger::warn("Could not find web camera");

    this->camera = new FileCam(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).toStdString() + "/debug");
    Logger::info("Falling back to reading files from disk");
}

void FlyCounter::detectShaker()
{
    this->shaker = new USBShaker();
    if (this->shaker->isAccessable())
    {
        Logger::info("Detected USB shaker");
        return;
    }

    delete this->shaker;
    Logger::warn("Could not find USB shaker");
    this->shaker = new NoShaker();
    Logger::info("Unable to shake");
}

/* create the directory for all the output data - always called before write */
std::string FlyCounter::makeExperimentDirectory()
{
    std::stringstream pathSS;
    pathSS << this->output << "/" << timeToString(this->experimentStart);

    std::string path = pathSS.str();
    QDir().mkpath(QString::fromStdString(path));

    return path;
}

/* stores the fly image */
void FlyCounter::writeImage(int elapsed)
{
    std::stringstream path;
    path << this->makeExperimentDirectory() << "/" << elapsed << ".jpg";

    this->imageLock.lock();
    if (!cv::imwrite(path.str(), this->cameraImage))
    {
        Logger::error("Could not save image");
    }
    this->imageLock.unlock();
}

/* output the fly counts in a tab-separated list into a file, leading value is the collection timestamp */
void FlyCounter::writeResults(int elapsed)
{
    std::string path = this->makeExperimentDirectory();
    std::ofstream file(path + "/results.csv", std::ios::app);
    if (!file.good())
    {
        Logger::error("Could not write results.csv file");
        return;
    }

    file << elapsed;
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

/* fetches new image from the camera */
void FlyCounter::updateCameraImage()
{
    if (!this->camera->getImage(this->cameraImage))
    {
        this->cameraImage = cv::Mat(); // create an empty matrix
        Logger::error("Could not obtain camera image");
        return;
    }
    cv::cvtColor(this->cameraImage, this->cameraImage, CV_BGR2RGB);
    this->updateVials();
}

void FlyCounter::updateClusterImage()
{
    if (this->cameraImage.empty())
    {
        this->clusterImage = cv::Mat();
        return;
    }

    int index = 0;
    std::map<int, int> clusterSizes;
    std::map<int, int> colorMap;
    this->clusterImage = cv::Mat(this->cameraImage.size(), this->cameraImage.type(), cv::Scalar(0));

    for (Vial vial : this->vials)
    {
        cv::Mat flies;
        cv::Mat flyPixels;
        int     colorIndex = 0;
        cv::Mat mask =  cv::Mat(this->thresholdImage.size(), this->thresholdImage.type(), cv::Scalar(0));

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
        for (int i = 0; i < numberOfPixels; ++i)
        {
            Color color = COLORS[colorMap[std::abs(labels[i])] % COLORS.size()];
            cv::Vec2f coord = flyPixels.at<cv::Vec2f>(0, i);
            this->clusterImage.at<cv::Vec3b>(coord[1],coord[0]) = color;
        }
    }
}

/* update the threshold image from the currently set camera image */
void FlyCounter::updateThresholdImage()
{
    if (this->cameraImage.empty())
    {
        this->thresholdImage = cv::Mat();
        return;
    }
    cv::Mat greyscaleImage;
    cv::cvtColor(this->cameraImage, greyscaleImage, CV_RGB2GRAY);
    cv::threshold(greyscaleImage, this->thresholdImage, this->threshold, 255, CV_THRESH_BINARY_INV);
}

void FlyCounter::updateVials()
{
    this->vials = findVials(this->cameraImage, this->vialSize);
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

/* detects the external devices - camera and shaker */
void FlyCounter::detectDevices()
{
    delete this->camera;
    delete this->shaker;

    this->detectCamera();
    this->detectShaker();
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
    if (this->thread.joinable())
    {
        this->thread.join();
    }
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

/* destructor */
FlyCounter::~FlyCounter()
{
    delete this->camera;
    delete this->shaker;
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
