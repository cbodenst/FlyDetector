#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsScene>
#include <QMainWindow>
#include <QString>

#include <opencv2/opencv.hpp>

#include "flycounter.h"

/* view type set */
enum ViewMode
{
    CAMERA    = 0,
    THRESHOLD = 1,
    CLUSTERS  = 2
};

namespace Ui {
    class MainWindow;
}

/*Main GUI class*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow* ui;
    QPixmap         image;
    QGraphicsScene* scene;
    FlyCounter      flyCounter;

    /* initialization */
    void setupUI();
    void setupSignals();

    /* image setters */
    void setImage(const cv::Mat& image);
    void setImage(const QPixmap& image);
    void setPixmap();
    void showCameraImage();
    void showClusterImage();
    void showThresholdImage();

    /* interface updates */
    void updateTimeSpinners();
    void userInterfaceEnabled(bool enabled);

private slots:
    void resizeEvent(QResizeEvent* event);
    void updateImage();

public:
    static const QString MODE;
    static const QString DISPLAY_VIALS;
    static const QString LEAD_TIME;
    static const QString ROUND_TIME;
    static const QString SHAKE_TIME;
    static const QString EPSILON;
    static const QString MIN_POINTS;
    static const QString PIXELS_PER_FLY;
    static const QString THRESHOLD;
    static const QString VIAL_SIZE;
    static const QString OUTPUT_PATH;
    static const QString SAVE_IMAGES;

    explicit MainWindow(QWidget* parent=nullptr);

    ViewMode    getViewMode();
    QPixmap toPixmap(const cv::Mat& image);

    ~MainWindow();

public slots:
    /* image settings */
    void on_detectDevices_clicked();
    void on_mode_currentIndexChanged(int index);
    void on_refresh_clicked();

    /* experiment parameter settings */
    void on_leadTime_valueChanged(int time);
    void on_roundTime_valueChanged(int time);
    void on_shakeTime_valueChanged(int time);

    /* analysis parameter setters */
    void on_epsilon_valueChanged(int epsilon);
    void on_minPoints_valueChanged(int minPoints);
    void on_pixelsPerFly_valueChanged(int pixelsPerFly);
    void on_threshold_valueChanged(int threshold);
    void on_vialSize_valueChanged(int vialSize);

    /* results */
    void on_outputPathBrowser_clicked();
    void on_output_textEdited(const QString& path);
    void on_saveImages_toggled(bool checked);

    /* experiment execution */
    void on_startButton_clicked();
    void on_stopButton_clicked();

    /* interface settings */
    void on_actionLoad_triggered();
    void on_actionSave_triggered();
};

#endif // MAINWINDOW_H
