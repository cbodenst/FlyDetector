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

typedef std::vector<Color> Colors;

class FlyCounter : public QObject
{
#ifndef SWIG
    Q_OBJECT
#endif

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
    int fliesTotal;

    /* results */
    Timepoint   experimentStart;
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

    std::string makeExperimentDirectory();
    void writeImage(int elapsed);
    void writeResults(int elapsed);

#ifndef SWIG
signals:
    /* GUI signals */
    void countUpdate(QString flies);
    void imageUpdate();
    void timeUpdate(QString time);
#endif
public:
    static Colors COLORS;

    explicit FlyCounter(QObject* parent=nullptr);

    /* External API */
    int count(const cv::Mat& img);
    cv::Mat generateThresholdImage(const cv::Mat& img);
    cv::Mat generateClusterImage(const cv::Mat& thresh, Vials &vials);
    int countFlies(const cv::Mat & threshImg, Vials &vials);

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

    /* results */
    const std::string& getOutput();

    /* image updates */
    void updateImages();
    void updateCameraImage();
    void updateThresholdImage();
    void updateClusterImage();
    void updateVials();

    /* validated setters */
    void validatedSetLeadTime(const Duration& time);
    void validatedSetRoundTime(const Duration& time);
    void validatedSetShakeTime(const Duration& time);

    /* execution */
    void lock();
    void unlock();
    void detectDevices();
    void start();
    void stop();

    /* destructor */
    ~FlyCounter();

#ifndef SWIG
public slots:
#endif
    void setEpsilon(int value);
    void setMinPoints(int value);
    void setPixelsPerFly(int value);
    void setThreshold(int value);
    void setVialSize(int value);
    void setOutput(const std::string& out);
    void storeImages(bool value);
};

#endif // FLYCOUNTER_H
