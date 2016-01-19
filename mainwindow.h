#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

#include <opencv2/opencv.hpp>

#include "flycounter.h"

/* view type set */
enum View
{
    RGB       = 1,
    THRESHOLD = 2,
    CLUSTERS  = 3
};

namespace UI {
    class MainWindow;
}

/*Main GUI class*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    UI::MainWindow *ui;

    QPixmap         image;
    QGraphicsScene* scene;

    FlyCounter      flyCounter;
    Vials           vials;

    View            mode;
    /*Adjust Image size to Window size*/
    QPixmap         toPixmap(const cv::Mat &image);
    void resizeEvent(QResizeEvent* event);

    static bool compareVials(const Vial& first, const Vial& second);
    void saveSettings(QString settings_path);
    void loadSettings(QString settings_path);
private slots:
    void onLoad();

    void on_startButton_clicked();
    void on_stopButton_clicked();

    void on_saveImages_toggled(bool checked);
    void on_output_textEdited(const QString &arg1);
    void on_actionLoad_triggered();
    void on_actionSave_triggered();

    void on_cluster_toggled(bool checked);
    void on_findVials_clicked();
    void on_preview_clicked();
    void on_rgb_toggled(bool checked);
    void on_threshold_toggled(bool checked);
    void on_threshhold_valueChanged(int arg1);

    void on_focus_valueChanged(int arg1);
    void on_epsilon_valueChanged(int arg1);
    void on_minPoints_valueChanged(int arg1);
    void on_pixelsPerFly_valueChanged(int arg1);

    void updateImage();

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

public slots:
    /* checks if experiment is running and disables experiment manipulation */
     void isRunning(bool running);
     /* displays the image in the GUI */
     void showImage();
};

#endif // MAINWINDOW_H
