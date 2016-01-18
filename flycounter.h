#ifndef FLYCOUNTER_H
#define FLYCOUNTER_H

#include <string>
#include <thread>
#include <vector>

#include <QObject>
#include <QReadWriteLock>
#include <QTime>

#include <opencv2/opencv.hpp>

#include "cam.h"
#include "shaker.h"

typedef std::vector<cv::Vec3f> Vials;

class FlyCounter : public QObject
{
    Q_OBJECT

private:
    /* experiment data */
    std::string output;
    bool        saveImages;

    /* images */
    cv::Mat cameraImage;
    cv::Mat thresholdImage;
    cv::Mat clusterImage;

    /* analysis parameters */
    int   epsilon;
    int   minPoints;
    int   measurementTime;
    int   numberOfFlies;
    int   pixelsPerFly;
    int   threshold;
    Vials vials;

    /* devices */
    Cam*   camera;
    Shaker shaker;

    /* threaded execution */
    QReadWriteLock        clusterLock;
    bool                  running;
    std::thread           thread;

    /* clustering */
    std::vector<int>      numLabels;
    std::vector<ssize_t*> labels;
    std::vector<cv::Mat>  coords;
    std::vector<int>      flies;
    cv::Mat               calibration;

    /* internal implementation meat */
    void analyze();
    int  countFlies();
    void detectCamera();
    void writeResults(int time);

signals:
    /* GUI signals */
    void flyCount(QString flies);
    void updateImage();
    void updateTime(QString time);

public:
    explicit FlyCounter(QObject *parent = nullptr);

    /* imaging getters */
    inline const cv::Mat& getCameraImage()    { return this->cameraImage; }
    inline const cv::Mat& getThresholdImage() { return this->thresholdImage; }
    inline const cv::Mat& getClusterImage()   { return this->clusterImage; }

    /* analysis parameter getters */
    inline int getMinPoints()       { return this->minPoints; }
    inline int getEpsilon()         { return this->epsilon; }
    inline int getMeasurementTime() { return this->measurementTime; }
    inline int getNumberOfFlies()   { return this->numberOfFlies; }
    inline int getPixelsPerFly()    { return this->pixelsPerFly; }
    inline int getThreshold()       { return this->threshold; }

    /* image setters */
    void setVials(Vials vials);
    inline void setOutput(const std::string& out) { this->output = out; }
    inline void storeImages(bool value)           { this->saveImages = value; }
    void calibrate();

    /* start or stop the experiment */
    void start();
    void stop();

    /* image processing */
    void updateCameraImage();
    void calculateThresholdImage();
    void calculateClusterImage();

    /* destructor */
    ~FlyCounter();

public slots:
    /* additional slot setters */
    inline void setThreshold(int value)       { this->threshold = value; }
    inline void setMinPoints(int value)       { this->minPoints = value; }
    inline void setEpsilon(int value)         { this->epsilon = value; }
    inline void setPixelsPerFly(int value)    { this->pixelsPerFly = value; }
    inline void setMeasurementTime(int value) { this->measurementTime = value; }
    inline void setFocus(int value)           { this->cam->setFocus(value); }
};

#endif // FLYCOUNTER_H
