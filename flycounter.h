#ifndef FLYCOUNTER_H
#define FLYCOUNTER_H

#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <QObject>

#include <opencv2/opencv.hpp>

#include "cam.h"
#include "shaker.h"
#include "timer.h"
#include "vials.h"

typedef std::vector<int>   Flies;
typedef cv::Vec3b          Color;
typedef std::vector<Color> Colors;

class FlyCounter : public QObject
{
    Q_OBJECT

private:
    /* experiment data */
    Duration leadTime;
    Duration roundTime;
    Duration shakeTime;

    /* images */
    cv::Mat cameraImage;
    cv::Mat clusterImage;
    cv::Mat thresholdImage;

    /* analysis parameters */
    int   epsilon;
    int   minPoints;
    int   numberOfFlies;
    int   pixelsPerFly;
    int   threshold;
    int   vialSize;
    Vials vials;
    Flies flies;

    /* results */
    std::string output;
    bool        saveImages;

    /* devices */
    Cam*    camera;
    Shaker* shaker;

    /* threaded execution */
    std::mutex  imageLock;
    bool        running;
    std::thread thread;

    /* internal implementation meat */
    void detectCamera();
    void detectShaker();
    void process();

    void updateCameraImage();
    void updateThresholdImage();
    void updateClusterImage();

    std::string toString(const Timepoint& time);
    void        writeImage(const Timepoint& time);
    void        writeResults(const Timepoint& time);

signals:
    /* GUI signals */
    void countUpdate(QString flies);
    void imageUpdate();
    void timeUpdate(QString time);

public:
    static Colors COLORS;

    explicit FlyCounter(QObject* parent=nullptr);

    /* imaging getters */
    const cv::Mat& getCameraImage();
    const cv::Mat& getThresholdImage();
    const cv::Mat& getClusterImage();

    /* experiment settings getters */
    const Duration& getLeadTime();
    const Duration& getRoundTime();
    const Duration& getShakeTime();

    /* analysis parameter getters */
    int getEpsilon();
    int getMinPoints();
    int getPixelsPerFly();
    int getThreshold();
    int getVialSize();
    bool isRunning();
    const Vials& getVials();

    /* validated setters */
    void validatedSetLeadTime(const Duration& time);
    void validatedSetRoundTime(const Duration& time);
    void validatedSetShakeTime(const Duration& time);

    /* results */
    const Flies& getFlies();
    const std::string& getOutput();

    /* execution */
    void lock();
    void unlock();

    void detectDevices();

    void start();
    void stop();

    /* image processing */
    void updateImages();

    /* destructor */
    ~FlyCounter();

public slots:
    void setEpsilon(int value);
    void setMinPoints(int value);
    void setPixelsPerFly(int value);
    void setThreshold(int value);
    void setVialSize(int value);
    void setOutput(const std::string& out);
    void storeImages(bool value);
};

#endif // FLYCOUNTER_H
