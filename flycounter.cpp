#include "flycounter.h"

#include <QDir>

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>

#include "settings.h"
#include "dbscan/hpdbscan.h"
#include "filecam.h"
#include "reflexcam.h"
#include "timer.h"
#include "webcamera.h"

FlyCounter::FlyCounter(QObject *parent)
  : QObject(parent),
    // experiment data
    output(Settings::OUTPUT_FILE),
    saveImages(Settings::SAVE_IMAGES),
    // analysis parameters
    measurementTime(Settings::MEASUREMENT_TIME),
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
    Timepoint start = Clock::now();
    Timepoint shake = start + Settings::ROUND_TIME - Settings::SHAKE_LEAD;

    while (this->running)
    {
        Timepoint current = Clock::now();

        if (current > shake)
        {
            this->shaker.shakeFor(Settings::SHAKE_TIME);
            shake += Settings::ROUND_TIME
        }

        if (roundTime > this->measureTime*1000)
        {
            round.start();
            countFlies();
            writeResults(elapsed);
            if(save_imgs)
            {
                std::stringstream filename;
                filename << "results/" << elapsed << ".jpg";
                cv::imwrite(filename.str(),image);
            }
            shaked=false;
        }
        emit updateTime(QString::number(elapsed));
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

/* output the fly counts in a tab-separated list into a file, leading value is the collection timestamp */
void FlyCounter::writeResults(int time)
{
    std::ofstream file(Settings::OUTPUT_PATH + "/" + this->output, std::ios::app);
    file << time ;
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
        emit flyCount(QString("--"));
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

/* fetch current image from the camera */
void FlyCounter::updateCameraImage()
{
    if (this->camera->getImage(image))
    {
        std::cerr << "Could not obtain image from the camera" << std::endl;
    }
    cv::cvtColor(image, image, CV_BGR2RGB);
}

/* calculate the threshold image from the currently set camera image */
void FlyCounter::calculateThresholdImage()
{
    cv::Mat greyscaleImage;
    cv::cvtColor(this->cameraImage, greyscaleImage, CV_RGB2GRAY);
    cv::threshold(greyscaleImage, this->threshold, this->getThresholdImage, 255, CV_THRESH_BINARY_INV);
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

void FlyCounter::calcClusters()
{
    int counter = 0;
    numFlies = 0;
    /*Iterate  for each vial*/
    for (auto vial : this->vials)
    {
        /*Mask current vial*/
        cv::Mat mask = cv::Mat(image.size(),thresh.type(),cv::Scalar(0));
        cv::circle(mask,cv::Point(vial[0],vial[1]),vial[2],cv::Scalar(255,255,255),-1);
        cv::Mat flies;
        cv::bitwise_and(mask,this->thresh,flies);
        cv::bitwise_and(flies, calibration ,flies);

        /*Extract white pixels*/
        cv::findNonZero(flies, coords[counter]);
        coords[counter].convertTo(coords[counter],CV_32FC2);
        if(!coords[counter].isContinuous())
            coords[counter] = coords[counter].clone();
        float* all = (float*)coords[counter].data;
        numLabels[counter] = coords[counter].size().height;

        /*DBSCAN on white pixels to find Clusters*/
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
            if(pair.first!=0)
                 this->flies[counter] += pair.second/ppf;
        }
        numFlies +=  this->flies[counter];
        counter++;
    }
    emit flieCount(QString::number(numFlies));
    emit updateImg();
}

int FlyCounter::countFlies()
{
    updateImage();
    this->calcThresh();
    this->calcClusters();
    return this->numFlies;
}

void FlyCounter::setVials(Vials vials)
{
    this->vials = vials;
    this->flies = std::vector<int>(vials.size());
    this->labels = std::vector<ssize_t*>(vials.size());
    this->numLabels = std::vector<int>(vials.size());
    this->coords = std::vector<cv::Mat>(vials.size());
}

void FlyCounter::calibrate()
{
    calcThresh();
    cv::Mat mask = cv::Mat(image.size(),thresh.type(),cv::Scalar(0));
    for (auto vial : this->vials)
    {
        cv::circle(mask,cv::Point(vial[0],vial[1]),vial[2],cv::Scalar(255,255,255),-1);
    }
    cv::bitwise_and(mask,this->thresh,this->calibration);
    cv::bitwise_not(calibration,calibration);
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

