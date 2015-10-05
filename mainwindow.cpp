#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsPixmapItem>

#include "opencv2/opencv.hpp"
#include <QTimer>
#include <QSettings>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    view_mode(VIEW_RGB)
{
    ui->setupUi(this);
    /*Load and setup image*/
    scn = new QGraphicsScene(ui->graphicsView);
    ui->graphicsView->setScene(scn);
    this->image = convertMat(counter.getImage());
    showImage();
    ui->graphicsView->fitInView(scn->itemsBoundingRect(),Qt::KeepAspectRatio);

    /*Connect experiment parameters elements to the detector class */
    connect(ui->threshhold,SIGNAL(valueChanged(int)),&counter,SLOT(setThresh(int)));
    connect(ui->eps,SIGNAL(valueChanged(int)),&counter,SLOT(setEps(int)));
    connect(ui->minPts,SIGNAL(valueChanged(int)),&counter,SLOT(setMinPts(int)));
    connect(ui->ppf,SIGNAL(valueChanged(int)),&counter,SLOT(setPPF(int)));
    connect(&counter,SIGNAL(flieCount(QString)),ui->flyCount,SLOT(setText(QString)));
    connect(&counter,SIGNAL(updateImg()),this,SLOT(updateImage()));
    connect(&counter,SIGNAL(updateTime(QString)),ui->time,SLOT(setText(QString)));
    connect(ui->inteval,SIGNAL(valueChanged(int)),&counter,SLOT(setMeasureTime(int)));

    /*Workaround for image resize bug*/
    QTimer::singleShot( 0, this, SLOT( onLoad() ));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::isRunning(bool running)
{
    /*While experiment is running, disable all Gui parameter elements*/
    if(running)
    {
        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        ui->threshhold->setEnabled(false);
        ui->eps->setEnabled(false);
        ui->minPts->setEnabled(false);
        ui->ppf->setEnabled(false);
        ui->inteval->setEnabled(false);
        ui->cluster->setEnabled(false);
        ui->thresh->setEnabled(false);
        ui->rgb->setEnabled(false);
        ui->findVials->setEnabled(false);
        ui->preview->setEnabled(false);
        ui->output->setEnabled(false);
        ui->saveImages->setEnabled(false);
    }
    else
    {
        ui->startButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        ui->threshhold->setEnabled(true);
        ui->eps->setEnabled(true);
        ui->minPts->setEnabled(true);
        ui->ppf->setEnabled(true);
        ui->inteval->setEnabled(true);
        ui->cluster->setEnabled(true);
        ui->thresh->setEnabled(true);
        ui->rgb->setEnabled(true);
        ui->findVials->setEnabled(true);
        ui->preview->setEnabled(true);
        ui->output->setEnabled(true);
        ui->saveImages->setEnabled(true);
    }
}

void MainWindow::showImage()
{
    scn->clear();
    QGraphicsPixmapItem* pixItem = new  QGraphicsPixmapItem(this->image);
    scn->addItem(pixItem);
}

void MainWindow::on_startButton_clicked()
{
    isRunning(true);
    counter.run();
}

void MainWindow::on_stopButton_clicked()
{
    isRunning(false);
    counter.stop();
}

QPixmap MainWindow::convertMat(const cv::Mat &image)
{
    QImage::Format format = QImage::Format_RGB888;
    if(image.type() != CV_8UC3)
        format = QImage::Format_Indexed8;
    return QPixmap::fromImage(QImage((unsigned char*) image.data, image.cols, image.rows, format));
}

void MainWindow::resizeEvent(QResizeEvent */*event*/)
{
    ui->graphicsView->fitInView(scn->itemsBoundingRect(),Qt::KeepAspectRatio);
}

void MainWindow::onLoad()
{
    ui->graphicsView->fitInView(scn->itemsBoundingRect(),Qt::KeepAspectRatio);
}

void MainWindow::on_thresh_toggled(bool checked)
{
    if(checked)
    {
        view_mode = VIEW_THRESH;
        counter.calcThresh();
        this->image = convertMat(counter.getThresh());
        showImage();
    }
}

void MainWindow::on_rgb_toggled(bool checked)
{
    if(checked)
    {
        view_mode = VIEW_RGB;
        this->image = convertMat(counter.getImage());
        updateImage();
    }

}

void MainWindow::on_threshhold_valueChanged(int)
{
    if(view_mode == VIEW_THRESH)
    {
        this->on_thresh_toggled(true);
    }
}

void MainWindow::on_cluster_toggled(bool checked)
{
    if(checked)
    {
        view_mode = VIEW_CLUSTERS;
        if(!counter.running)
        {
            counter.calcThresh();
            counter.calcClusters();
        }
        this->image = convertMat(counter.getClusters());
        showImage();
    }

}

void MainWindow::on_eps_valueChanged(int)
{
    if(view_mode == VIEW_CLUSTERS)
    {
        this->on_cluster_toggled(true);
    }
}

void MainWindow::on_minPts_valueChanged(int)
{
    if(view_mode == VIEW_CLUSTERS)
    {
        this->on_cluster_toggled(true);
    }
}

void MainWindow::on_ppf_valueChanged(int)
{
    if(view_mode == VIEW_CLUSTERS)
    {
        this->on_cluster_toggled(true);
    }
}

void MainWindow::on_preview_clicked()
{
    this->counter.updateImage();    
    updateImage();

}

void MainWindow::updateImage()
{
    switch(view_mode)
    {
        case VIEW_RGB :
        {
            auto img = counter.getImage().clone();
            int counter = 0;
            for (auto vial : this->vials)
            {
                cv::circle(img,cv::Point(vial[0],vial[1]),vial[2],cv::Scalar(0,255,0),5);
                std::stringstream s;
                s << counter++;
                cv::putText(img,s.str(),cv::Point(vial[0]-10,vial[1]+10),0,1,cv::Scalar(0,255,0),4);
            }
            this->image = convertMat(img);
            break;
        }
        case VIEW_THRESH : this->image = convertMat(counter.getThresh());break;
        case VIEW_CLUSTERS : this->image = convertMat(counter.getClusters());break;
    }
    showImage();

}

void MainWindow::on_output_textEdited(const QString &arg1)
{
    counter.setOutput(arg1.toStdString());
}

void MainWindow::on_focus_valueChanged(int arg1)
{
    counter.setFocus(arg1);
    this->counter.updateImage();
    updateImage();
}

void MainWindow::on_findVials_clicked()
{
    this->vials.clear();
    cv::Mat grey;
    cv::cvtColor( counter.getImage(), grey, 7);
    int vial_size =ui->vialsize->value();
    cv::HoughCircles(grey,this->vials, CV_HOUGH_GRADIENT,2,ui->vialsize->value()*2 ,100,100,ui->vialsize->value() - 20,ui->vialsize->value() + 20);
    if (this->vials.size() == 0)
        this->vials.push_back(cv::Vec3f(550,360,340));
    std::sort(this->vials.begin(), this->vials.end(), compVials);
    counter.setVials(this->vials);
    if(ui->Calibration->isChecked())
        this->counter.calibrate();
    this->updateImage();

}

void MainWindow::on_saveImages_toggled(bool checked)
{
    this->counter.saveImages(checked);
}

bool MainWindow::compVials(const cv::Vec3f& first, const cv::Vec3f& second)
{
    int threshold = first[2];
    if(std::abs(first[1]-second[1]) < threshold)
        return first[0] < second[0];
    return first[1] < second[1];
}

void MainWindow::saveSettings(QString settings_path)
{
    QSettings settings(settings_path,QSettings::NativeFormat);
    settings.setValue("minPts", ui->minPts->value());
    settings.setValue("calibration", ui->Calibration->isChecked());
    settings.setValue("epsilon", ui->eps->value());
    settings.setValue("focus", ui->focus->value());
    settings.setValue("interval", ui->inteval->value());
    settings.setValue("output", ui->output->text());
    settings.setValue("ppf", ui->ppf->value());
    settings.setValue("saveImages", ui->saveImages->isChecked());
    settings.setValue("threshhold", ui->threshhold->value());
    settings.setValue("vialsize", ui->vialsize->value());
}
void MainWindow::loadSettings(QString settings_path)
{
    if(settings_path == "")
        return;
    QSettings settings(settings_path,QSettings::NativeFormat);
    ui->minPts->setValue(settings.value("minPts").toInt());
    ui->Calibration->setChecked(settings.value("calibration").toBool());
    ui->eps->setValue(settings.value("epsilon").toInt());
    ui->focus->setValue(settings.value("focus").toInt());
    ui->inteval->setValue(settings.value("interval").toInt());
    ui->output->setText(settings.value("output").toString());
    ui->ppf->setValue(settings.value("ppf").toInt());
    ui->saveImages->setChecked(settings.value("saveImages").toBool());
    ui->threshhold->setValue(settings.value("threshhold").toInt());
    ui->vialsize->setValue(settings.value("vialsize").toInt());

    //Tiggers
    ui->minPts->valueChanged(settings.value("minPts").toInt());
    ui->eps->valueChanged(settings.value("epsilon").toInt());
    ui->focus->valueChanged(settings.value("focus").toInt());
    ui->inteval->valueChanged(settings.value("interval").toInt());
    ui->output->textChanged(settings.value("output").toString());
    ui->ppf->valueChanged(settings.value("ppf").toInt());
    ui->saveImages->toggled(settings.value("saveImages").toBool());
    ui->threshhold->valueChanged(settings.value("threshhold").toInt());
}

void MainWindow::on_actionSave_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save file");
    this->saveSettings(filename);
}

void MainWindow::on_actionLoad_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open file");
    this->loadSettings(filename);
}
