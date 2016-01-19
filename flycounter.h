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
#include "timer.h"

typedef cv::Vec3f         Vial;
typedef std::vector<Vial> Vials;

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
    bool        running;
    std::thread thread;

    /* clustering */
    std::vector<int>      numLabels;
    std::vector<ssize_t*> labels;
    std::vector<cv::Mat>  coords;
    std::vector<int>      flies;
    cv::Mat               calibration;

    /* internal implementation meat */
    void        analyze();
    void        detectCamera();
    std::string toString(const Timepoint& time);
    void        writeImage(const Timepoint& time);
    void        writeResults(const Timepoint& time);

signals:
    /* GUI signals */
    void countUpdate(QString flies);
    void imageUpdate(cv::Mat& image);
    void timeUpdate(QString time);

public:
    explicit FlyCounter(QObject *parent = nullptr);

    /* imaging getters */
    inline const cv::Mat& getCameraImage()    { return this->cameraImage; }
    inline const cv::Mat& getThresholdImage() { return this->thresholdImage; }
    inline const cv::Mat& getClusterImage()   { return this->clusterImage; }

    /* analysis parameter getters */
    inline int  getMinPoints()       { return this->minPoints; }
    inline int  getEpsilon()         { return this->epsilon; }
    inline int  getMeasurementTime() { return this->measurementTime; }
    inline int  getNumberOfFlies()   { return this->numberOfFlies; }
    inline int  getPixelsPerFly()    { return this->pixelsPerFly; }
    inline int  getThreshold()       { return this->threshold; }
    inline bool isRunning()          { return this->running; }

    /* image setters */
    void setVials(Vials vials);
    inline void setOutput(const std::string& out) { this->output = out; }
    inline void storeImages(bool value)           { this->saveImages = value; }

    /* start or stop the experiment */
    void start();
    void stop();

    /* image processing */
    void calculateThresholdImage();
    void calculateClusterImage();
    int  count();
    void updateCameraImage();

    /* destructor */
    ~FlyCounter();

public slots:
    /* additional slot setters */
    inline void setThreshold(int value)       { this->threshold = value; }
    inline void setMinPoints(int value)       { this->minPoints = value; }
    inline void setEpsilon(int value)         { this->epsilon = value; }
    inline void setPixelsPerFly(int value)    { this->pixelsPerFly = value; }
    inline void setMeasurementTime(int value) { this->measurementTime = value; }
    inline void setFocus(int value)           { this->camera->setFocus(value); }
};

#endif // FLYCOUNTER_H
