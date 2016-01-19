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
#include "settings.h"
#include "webcamera.h"

FlyCounter::FlyCounter(QObject *parent)
  : QObject(parent),
    // experiment data
    output(Settings::OUTPUT_FILE),
    saveImages(Settings::SAVE_IMAGES),

    // analysis parameters
    epsilon(Settings::EPSILON),
    minPoints(Settings::MIN_POINTS),
    pixelsPerFly(Settings::PIXELS_PER_FLY),
    threshhold(Settings::THRESHOLD),

    // threaded execution
    running(false)

    // clustering
    numLabels(0),
    numberOfFlies(0),
{
    /* setup initial images */
    this->detectCamera();
    this->updateCameraImage();
    this->calculateThresholdImage();
    this->clusters    = this->cameraImage.clone();
    this->calibration = cv::Mat(2, this->cameraImage.size, CV_8U, cv::Scalar(255));

    /* create ouput directory */
    QDir().mkdir(Constants::OUTPUT_PATH);
}

/* image analysis mainloop */
void FlyCounter::analyze()
{
    Timepoint start   = Clock::now();
    Timepoint measure = start + Settings::ROUND_TIME;
    Timepoint shake   = measure - Settings::SHAKE_LEAD;

    while (this->running)
    {
        Timepoint current = Clock::now();

        // Time to shake?
        if (current > shake)
        {
            shake += Settings::ROUND_TIME;
            this->shaker.shakeFor(Settings::SHAKE_TIME);
        }

        // Time to take a picture?
        if (current > measure)
        {
            measure += Settings::ROUND_TIME;
            this->countFlies();
            this->writeResults(current);

            if (this->saveImages)
            {
                this->writeImage(current);
            }
        }

        int elapsed = std::chrono::duration_cast<Seconds>(current - start).count();
        emit timeUpdate(QString::number(elapsed));
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
std::string toString(const Timepoint& time)
{
    auto a_time_t = Clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&a_time_t), Settings::TIME_FORMAT);

    return ss.str();
}

/* stores the fly image */
void FlyCounter::writeImage(const Timepoint& time)
{
    std::stringstream path;
    path << Settings::OUTPUT_PATH << "/" << this->toString(time) << ".jpg";
    cv::imwrite(path.str(), this->cameraImage);
}

/* output the fly counts in a tab-separated list into a file, leading value is the collection timestamp */
void FlyCounter::writeResults(const Timepoint& time)
{
    std::ofstream file(Settings::OUTPUT_PATH + "/" + this->output, std::ios::app);
    file << this->toString(time) ;
    for (int count : this->flies)
    {
        file << "\t" << count;
    }
    file << std::endl;
    file.close();
}

/** public **/

/* start the fly counter */
void FlyCounter::start()
{
    if (!this->running)
    {
        emit countUpdate(QString("n/a"));
        this->running = true;
        this->thread  = std::thread(&FlyCounter::analyze, this);
    }
}

/* stop the fly counter */
void FlyCounter::stop()
{
    this->running = false;
    this->thread.join();
}

/* calculate the threshold image from the currently set camera image */
void FlyCounter::calculateThresholdImage()
{
    cv::Mat greyscaleImage;
    cv::cvtColor(this->cameraImage, greyscaleImage, CV_RGB2GRAY);
    cv::threshold(greyscaleImage, this->thresholdImage, this->threshold, 255, CV_THRESH_BINARY_INV);
}

void FlyCounter::calculateClusterImage()
{
    int counter = 0;
    this->numberOfFlies = 0;

    /* iterate over each vial */
    for (auto vial : this->vials)
    {
        /* mask vial */
        cv::Mat mask = cv::Mat(this->cameraImage.size(), this->thresholdImage.type(), cv::Scalar(0));
        cv::circle(mask, cv::Point(vial[0],vial[1]), vial[2], cv::Scalar(255,255,255), -1);
        cv::Mat flies;
        cv::bitwise_and(mask, this->thresholdImage, flies);
        cv::bitwise_and(flies, this->calibration, flies);

        /* extract white pixels */
        cv::findNonZero(flies, coords[counter]);
        coords[counter].convertTo(coords[counter], CV_32FC2);
        if (!coords[counter].isContinuous())
        {
            coords[counter] = coords[counter].clone();
        }
        float* all = (float*) coords[counter].data;
        numLabels[counter] = coords[counter].size().height;

        /* cluster white pixels with DBSCAN */
        HPDBSCAN dbscan(all, numLabels[counter], 2);
        delete[] this->labels[counter];
        this->labels[counter] = new ssize_t[numLabels[counter]];
        dbscan.scan(eps, minPts, this->labels[counter]);

        /*Count flies*/
        std::map<int,int> clusters;

        for (int i = 0; i < numLabels[counter]; ++i)
        {
            clusters[abs(labels[counter][i])]++;
        }
        this->flies[counter]=0;

        for (auto pair : clusters)
        {
            if (pair.first != 0)
                 this->flies[counter] += pair.second / ppf;
        }
        numFlies +=  this->flies[counter];
        ++counter;
    }

    emit countUpdate(QString::number(numFlies));
    emit imageUpdate(this->cameraImage);
}

/* perform a full fly counting process - get a camera image, calculate threshold and cluster image and return the number of flies */
int FlyCounter::count()
{
    this->updateCameraImage();
    this->calculateThresholdImage();
    this->calculateClusterImage();

    return this->numberOfFlies;
}

/* fetch current image from the camera */
void FlyCounter::updateCameraImage()
{
    if (!this->camera->getImage(image))
    {
        std::cerr << "Could not obtain image from the camera" << std::endl;
        return;
    }
    cv::cvtColor(image, image, CV_BGR2RGB);
}

const cv::Mat& FlyCounter::getClusters()
{
    /*Lock while calculating*/
    this->clusterLock.lockForWrite();
    this->clusters = cv::Mat(this->image.size(), this->thresh.type(),cv::Scalar(0));

    /*Iterate for each vial*/
    for(size_t counter = 0; counter < vials.size();counter++)
    {
        /*Chek if flies where found*/
        if(this->flies[counter]==0)
        {
            continue;
        }

        /*Count number of clusters*/
        std::map<int,int> clusters;
        for (int i = 0; i < numLabels[counter]; ++i)
        {
            clusters[abs(this->labels[counter][i])]++;
        }
        int offset = 125 / flies[counter] + 1;
        int color = 125 + offset;

        /*Set color of cluster*/
        for(auto pair : clusters)
        {
            if(pair.first!=0 && pair.second > this->ppf)
            {
                clusters[pair.first] = color;
                color+=offset;
            }
            else
            {
                clusters[pair.first] = 0;
            }
        }
        /*Draw clusters to image*/
        for (int i = 0; i < numLabels[counter]; ++i)
        {
            color = clusters[abs(this->labels[counter][i])];
            cv::Vec2f coord = this->coords[counter].at<cv::Vec2f>(0,i);
            this->clusters.at<uchar>(coord[1],coord[0]) = color;
        }
    }

    this->clusterLock.unlock();
    return this->clusters;
}

void FlyCounter::setVials(Vials vials)
{
    this->vials     = vials;
    this->flies     = std::vector<int>(vials.size());
    this->labels    = std::vector<ssize_t*>(vials.size());
    this->numLabels = std::vector<int>(vials.size());
    this->coords    = std::vector<cv::Mat>(vials.size());
}

/* destructor */
FlyCounter::~FlyCounter()
{
    delete this->cam;
    for (auto label : labels)
    {
        delete[] label;
    }
}
