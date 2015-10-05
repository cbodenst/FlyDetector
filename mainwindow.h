#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

#include "opencv2/opencv.hpp"

#include "flycounter.h"

/*Enum for different Kind of visiualization types*/
enum
{
    VIEW_RGB = 1,
    VIEW_THRESH = 2,
    VIEW_CLUSTERS = 3

};

namespace Ui {
class MainWindow;
}

/*Main GUI class*/
class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    /*checks if experiment is running and disables experiment manipulation*/
     void isRunning(bool running);

     /*Displays the image in the GUI*/
     void showImage();

/*Widget callbacks*/
private slots:
     void on_startButton_clicked();

     void on_stopButton_clicked();

     void on_thresh_toggled(bool checked);

     void on_rgb_toggled(bool checked);

     void on_threshhold_valueChanged(int arg1);

     void on_cluster_toggled(bool checked);

     void on_eps_valueChanged(int arg1);

     void on_minPts_valueChanged(int arg1);

     void on_ppf_valueChanged(int arg1);

     void updateImage();

     void on_findVials_clicked();

     void on_output_textEdited(const QString &arg1);

     void on_focus_valueChanged(int arg1);

     void on_preview_clicked();

     void on_saveImages_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    /*Image to display*/
    QGraphicsScene *scn;
    QPixmap image;


    FlyCounter counter;
    std::vector<cv::Vec3f> vials;
    QPixmap convertMat(const cv::Mat &image);

    int view_mode;
    /*Adjust Image size to Window size*/
    void resizeEvent(QResizeEvent* event);

    static bool compVials(const cv::Vec3f& first, const cv::Vec3f& second);
    void saveSettings(QString settings_path);
    void loadSettings(QString settings_path);
private slots:
    void onLoad();
    void on_actionSave_triggered();
    void on_actionLoad_triggered();
};

#endif // MAINWINDOW_H
