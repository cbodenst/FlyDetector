#ifndef FLYCOUNTER_H
#define FLYCOUNTER_H
#include <QObject>
#include <QReadWriteLock>
#include <QTime>

#include <string>
#include <thread>

#include "opencv2/opencv.hpp"

#include "flycam.h"

#define SAMPLE_FILE "D:/FlyDetector/Detector/FlyDetector/flies.jpg"


typedef std::vector<cv::Vec3f> Vials;

class FlyCounter : public QObject
{
    Q_OBJECT
public:

    explicit FlyCounter(QObject *parent = 0);
    ~FlyCounter();

    /*Runs or stops the experiment*/
     void run();
     void stop();

     /*Returns Image, thrashhold, and cluster preview*/
     const cv::Mat& getImage();
     const cv::Mat& getThresh();
     const cv::Mat& getClusters();

     /*Getters for experiment parameters*/
     int getMinPts();
     int getEps();
     int getPPF();

     /*Setters for experiment parameters*/
     void setVials(Vials vials);
     void setOutput(std::string out);
     void saveImages(bool value);
     void calibrate();

     /*Update functions*/
     void updateImage();
     void calcThresh();
     void calcClusters();

     /*Flag if experiment running*/
     bool running;

public slots:
     /*More setters via slots*/
     void setThresh(int value);
     void setMinPts(int value);
     void setEps(int value);
     void setPPF(int value);
     void setMeasureTime(int value);
     void setFocus(int value);

private:
     /*Images*/
     cv::Mat image;
     cv::Mat thresh;
     cv::Mat clusters;

    /*Experiment parameters*/
    int threshhold;
    int numFlies;
    std::string filename;
    int minPts;
    int eps;
    int ppf;
    Vials vials;
    int measureTime;
    std::string output;
    bool save_imgs;

    /*Helper attributes*/
    std::vector<int> numLabels;
    std::thread* thread;
    std::vector<ssize_t*> labels;
    std::vector<cv::Mat> coords;
    std::vector<int> flies;
    QReadWriteLock clusterLock;
    QTime timer;
    cv::Mat calibration;

    /*Camera*/
    FlyCam* cam;

    /*Helper functions*/
    int countFlies();
    void start();
    void writeResults(int time);

signals:
    /*Signals to GUI*/
    void flieCount(QString flies);
    void updateImg();
    void updateTime(QString time);

};

#endif // FLYCOUNTER_H
