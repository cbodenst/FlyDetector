#include "flycounter.h"
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <map>
#include <omp.h>

#include <QDir>

#include "opencv2/opencv.hpp"
#include "dbscan/hpdbscan.h"
#include <fstream>

#include "webcamera.h"
#include "samplecam.h"
#include "reflexcam.h"

FlyCounter::FlyCounter(QObject *parent)
:
QObject(parent),
running(false),
threshhold(100),
numFlies(0),
minPts(50),
eps(5),
ppf(300),
measureTime(60),
output("result.csv"),
save_imgs(true),
numLabels(0)
{
    /*Try out different cams*/
    cam = new ReflexCam();
    if (!cam->accessable)
    {
        delete cam;
        cam = new WebCamera();
        if (!cam->accessable)
        {
            delete cam;
            cam = new SampleCam("new",37);
        }
    }
    this->updateImage();
    calcThresh();
    this->clusters = image.clone();

    /*Create ouput directory*/
    QDir().mkdir("results");
}

FlyCounter::~FlyCounter()
{
    for(auto l : labels)
        delete[] l;
}

void FlyCounter::run()
{
    if(!this->running)
    {
        emit flieCount(QString("--"));
        thread = new std::thread(&FlyCounter::start,this);
    }
}
void FlyCounter::stop()
{
    this->running = false;;
    thread->join();
}

const cv::Mat &FlyCounter::getImage()
{
    return this->image;
}

const cv::Mat &FlyCounter::getThresh()
{
    return this->thresh;
}

const cv::Mat &FlyCounter::getClusters()
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


int FlyCounter::getMinPts()
{
    return this->minPts;
}

int FlyCounter::getEps()
{
    return this->eps;
}

int FlyCounter::getPPF()
{
    return this->ppf;
}

void FlyCounter::calcThresh()
{
    cv::Mat grey;
    cv::cvtColor( image, grey, 7);
    cv::threshold(grey,thresh,this->threshhold,255,1);
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

void FlyCounter::setThresh(int value)
{
    this->threshhold = value;
}

void FlyCounter::setMinPts(int value)
{
    this->minPts = value;
}

void FlyCounter::setEps(int value)
{
    this->eps = value;
}

void FlyCounter::setPPF(int value)
{
    this->ppf = value;
}

void FlyCounter::setMeasureTime(int value)
{
    this->measureTime = value;
}

void FlyCounter::setFocus(int value)
{
    cam->setFocus(value);
}

void FlyCounter::start()
{
        this->running = true;
        this->timer.start();
        QTime round;
        round.start();

        /*Loop*/
        while(this->running)
        {
            int roundTime = round.elapsed();
            int elapsed = this->timer.elapsed()/1000;
            if(roundTime > this->measureTime*1000)
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
            }
            emit updateTime(QString::number(elapsed));
        }
}

int FlyCounter::countFlies()
{
    updateImage();
    this->calcThresh();
    this->calcClusters();
    return this->numFlies;
}

void FlyCounter::updateImage()
{
    cam->getImage(image);
    cv::cvtColor(image,image,4);
}

void FlyCounter::setVials(Vials vials)
{
    this->vials = vials;
    this->flies = std::vector<int>(vials.size());
    this->labels = std::vector<ssize_t*>(vials.size());
    this->numLabels = std::vector<int>(vials.size());
    this->coords = std::vector<cv::Mat>(vials.size());
}

void FlyCounter::setOutput(std::string out)
{
    this->output = out;
}

void FlyCounter::saveImages(bool value)
{
    this->save_imgs = value;
}

void FlyCounter::writeResults(int time)
{
    std::ofstream file;
    file.open("results/" + this->output, std::ios::app);
    file << time ;
    for(int count :this->flies)
        file << "\t" << count;
    file  << std::endl;
    file.close();
}

