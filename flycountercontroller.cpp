#include "flycountercontroller.h"

#include <QDir>
#include <QStandardPaths>
#include <QTemporaryFile>

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>

#include "filecam.h"
#include "logger.h"
#include "reflexcam.h"
#include "noshaker.h"
#include "usbshaker.h"
#include "webcamera.h"

FlyCounterController::FlyCounterController(QObject* parent)
  : QObject(parent),

    // experiment parameters
    leadTime(Seconds(0)),
    roundTime(Seconds(0)),
    shakeTime(Seconds(0)),

    // analysis parameters
    vialSize(0),
    fliesTotal(0),

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
void FlyCounterController::process()
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
void FlyCounterController::detectCamera()
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

void FlyCounterController::detectShaker()
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
std::string FlyCounterController::makeExperimentDirectory()
{
    std::stringstream pathSS;
    pathSS << this->output << "/" << timeToString(this->experimentStart);

    std::string path = pathSS.str();
    QDir().mkpath(QString::fromStdString(path));

    return path;
}

/* stores the fly image */
void FlyCounterController::writeImage(int elapsed)
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
void FlyCounterController::writeResults(int elapsed)
{
    std::string path = this->makeExperimentDirectory();
    std::ofstream file(path + "/results.csv", std::ios::app);
    if (!file.good())
    {
        Logger::error("Could not write results.csv file");
        return;
    }

    file << elapsed;
    for (Vial vial : this->vials)
    {
        file << "\t" << vial.flyCount;
    }
    file << std::endl;
    file.close();
}

/** public **/

/* image getters */
const cv::Mat& FlyCounterController::getCameraImage()
{
    return this->cameraImage;
}

const cv::Mat& FlyCounterController::getThresholdImage()
{
    return this->thresholdImage;
}

const cv::Mat& FlyCounterController::getClusterImage()
{
    return this->clusterImage;
}

/* timer getters */
const Duration& FlyCounterController::getLeadTime()
{
    return this->leadTime;
}

const Duration& FlyCounterController::getRoundTime()
{
    return this->roundTime;
}

const Duration& FlyCounterController::getShakeTime()
{
    return this->shakeTime;
}

/* analysis parameter getters */
int FlyCounterController::getEpsilon()
{
    return this->flycounter.getEpsilon();
}

int FlyCounterController::getMinPoints()
{
    return this->flycounter.getMinPoints();
}

int FlyCounterController::getPixelsPerFly()
{
    return this->flycounter.getPixelsPerFly();
}

int FlyCounterController::getThreshold()
{
    return this->flycounter.getThreshold();
}

int FlyCounterController::getVialSize()
{
    return this->vialSize;
}

bool FlyCounterController::isRunning()
{
    return this->running;
}

const Vials& FlyCounterController::getVials()
{
    return this->vials;
}

/* results */
const std::string& FlyCounterController::getOutput()
{
    return this->output;
}


void FlyCounterController::updateImages()
{
    this->imageLock.lock();
    this->updateCameraImage();
    this->updateThresholdImage();
    this->updateClusterImage();
    this->imageLock.unlock();

    emit countUpdate(QString::number(this->fliesTotal));
    emit imageUpdate();
}

/* fetches new image from the camera */
void FlyCounterController::updateCameraImage()
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

void FlyCounterController::updateClusterImage()
{
    if (this->cameraImage.empty())
    {
        this->clusterImage = cv::Mat();
        return;
    }

    this->flycounter.countFlies(this->thresholdImage, this->vials);

    clusterImage = this->flycounter.generateClusterImage(this->cameraImage, this->vials);
}

/* update the threshold image from the currently set camera image */
void FlyCounterController::updateThresholdImage()
{
    if (this->cameraImage.empty())
    {
        this->thresholdImage = cv::Mat();
        return;
    }
    this->thresholdImage = this->flycounter.generateThresholdImage(this->cameraImage);
}

void FlyCounterController::updateVials()
{
    this->vials = findVials(this->cameraImage, this->vialSize);
}

/* validated time setters - adjust the respective two other timers according to the passed individual timer */
/* e.g. round time set to 2 -> shake time and lead time have been 5s and 6s before and get changed to 1s */
void FlyCounterController::validatedSetLeadTime(const Duration& time)
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

void FlyCounterController::validatedSetRoundTime(const Duration& time)
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

void FlyCounterController::validatedSetShakeTime(const Duration& time)
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
void FlyCounterController::detectDevices()
{
    delete this->camera;
    delete this->shaker;

    this->detectCamera();
    this->detectShaker();
}

/* start the fly counter */
void FlyCounterController::start()
{
    if (!this->running)
    {
        this->running = true;
        this->thread  = std::thread(&FlyCounterController::process, this);
    }
}

/* stop the fly counter */
void FlyCounterController::stop()
{
    this->running = false;
    if (this->thread.joinable())
    {
        this->thread.join();
    }
}

/* execution */
void FlyCounterController::lock()
{
    this->imageLock.lock();
}

void FlyCounterController::unlock()
{
    this->imageLock.unlock();
}

/* destructor */
FlyCounterController::~FlyCounterController()
{
    delete this->camera;
    delete this->shaker;
}

/** public slots **/

/* analysis parameters */
void FlyCounterController::setEpsilon(int value)
{
    this->flycounter.setEpsilon(value);
}

void FlyCounterController::setMinPoints(int value)
{
    this->flycounter.setMinPoints(value);
}

void FlyCounterController::setPixelsPerFly(int value)
{
    this->flycounter.setPixelsPerFly(value);
}

void FlyCounterController::setThreshold(int value)
{
    this->flycounter.setThreshold(value);
}

void FlyCounterController::setVialSize(int value)
{
    this->vialSize = value;
}

/* result setters */
void FlyCounterController::setOutput(const std::string& out)
{
    this->output = out;
}

void FlyCounterController::storeImages(bool value)
{
    this->saveImages = value;
}
