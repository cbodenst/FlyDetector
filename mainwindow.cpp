#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QSettings>
#include <QTimer>

#include <opencv2/opencv.hpp>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "vials.h"

/* settings file keys */
const QString MainWindow::MODE           = "mode";
const QString MainWindow::DISPLAY_VIALS  = "displayVials";
const QString MainWindow::LEAD_TIME      = "leadTime";
const QString MainWindow::ROUND_TIME     = "roundTime";
const QString MainWindow::SHAKE_TIME     = "shakeTime";
const QString MainWindow::EPSILON        = "epsilon";
const QString MainWindow::MIN_POINTS     =  "minPoints";
const QString MainWindow::PIXELS_PER_FLY = "pixelsPerFly";
const QString MainWindow::THRESHOLD      = "threshold";
const QString MainWindow::VIAL_SIZE      = "vialSize";
const QString MainWindow::OUTPUT_PATH    = "outputPath";
const QString MainWindow::SAVE_IMAGES    = "saveImages";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    flyCounter(this,
               Seconds(this->ui->leadTime->value()),
               Seconds(this->ui->roundTime->value()),
               Seconds(this->ui->shakeTime->value()),
               this->ui->epsilon->value(),
               this->ui->minPoints->value(),
               this->ui->pixelsPerFly->value(),
               this->ui->threshold->value(),
               this->ui->vialSize->value(),
               this->ui->outputPath->text().toStdString(),
               this->ui->saveImages->isChecked())
{
    this->setupUI();
    this->setupSignals();

    /* XXX: workaround for image resize bug */
    QTimer::singleShot(0, this, SLOT(resizeEvent()));
}

/** private **/

/* window resize handler */
void MainWindow::resizeEvent(QResizeEvent* /*event*/)
{
    this->ui->image->fitInView(this->scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

/* initializes the GUI elements; specifically: constructs a scene in the graphics view and get an initial camera shot */
void MainWindow::setupUI()
{
    this->ui->setupUi(this);
    this->scene = new QGraphicsScene(this->ui->image);
    this->ui->image->setScene(this->scene);
    this->showCameraImage();
    this->resizeEvent(nullptr);
}

/* registers the QT interface signals */
void MainWindow::setupSignals()
{
    connect(&this->flyCounter, SIGNAL(countUpdate(QString)),        this->ui->flies, SLOT(setText(QString)));
    connect(&this->flyCounter, SIGNAL(imageUpdate(const cv::Mat&)), this,            SLOT(updateImage()));
    connect(&this->flyCounter, SIGNAL(timeUpdate(QString)),         this->ui->timer, SLOT(setText(QString)));
}

/* set the passed OpenCV image in the graphics view */
void MainWindow::setImage(const cv::Mat& image)
{
    this->image = this->toPixmap(image);
    this->setPixmap();
}

/* set the passed QT pixmap image in the graphics view */
void MainWindow::setImage(const QPixmap& image)
{
    this->image = image;
    this->setPixmap();
}

/* creates an actual pixmap item in the scene after setting a new image in the setter */
void MainWindow::setPixmap()
{
    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(this->image);
    this->scene->clear();
    this->scene->addItem(pixmapItem);
}

void MainWindow::showCameraImage()
{
    this->flyCounter.lock();
    const cv::Mat& cameraImage = this->flyCounter.getCameraImage();
    if (this->ui->displayVials->isChecked())
    {
        this->setImage(drawVials(this->flyCounter.getVials(), cameraImage));
    }
    else
    {
        this->setImage(cameraImage);
    }
    this->flyCounter.unlock();
}

void MainWindow::showClusterImage()
{
    this->flyCounter.lock();
    this->setImage(this->flyCounter.getClusterImage());
    this->flyCounter.unlock();
}

void MainWindow::showThresholdImage()
{
    this->flyCounter.lock();
    this->setImage(this->flyCounter.getThresholdImage());
    this->flyCounter.unlock();
}

/* update the spin box values of the lead/round/shake timer after validating the model */
void MainWindow::updateTimeSpinners()
{
    this->ui->leadTime->setValue(convertToInt(this->flyCounter.getLeadTime()));
    this->ui->roundTime->setValue(convertToInt(this->flyCounter.getRoundTime()));
    this->ui->shakeTime->setValue(convertToInt(this->flyCounter.getShakeTime()));
}

/* dis-/enables the user interface widgets */
void MainWindow::userInterfaceEnabled(bool enabled)
{
    /* disable the interface during the run */
    this->ui->start->setEnabled(enabled);
    this->ui->stop->setEnabled(!enabled);

    /* image settings */
    this->ui->detectDevices->setEnabled(enabled);
    this->ui->displayVials->setEnabled(enabled);
    this->ui->refresh->setEnabled(enabled);

    /* experiment settings */
    this->ui->leadTime->setEnabled(enabled);
    this->ui->roundTime->setEnabled(enabled);
    this->ui->shakeTime->setEnabled(enabled);

    /* analysis */
    this->ui->epsilon->setEnabled(enabled);
    this->ui->minPoints->setEnabled(enabled);
    this->ui->pixelsPerFly->setEnabled(enabled);
    this->ui->threshold->setEnabled(enabled);
    this->ui->vialSize->setEnabled(enabled);

    /* results */
    this->ui->outputPath->setEnabled(enabled);
    this->ui->outputPathBrowser->setEnabled(enabled);
    this->ui->saveImages->setEnabled(enabled);
}

/** public **/

ViewMode MainWindow::getViewMode()
{
    return (ViewMode)this->ui->mode->currentIndex();
}

/* converts a cv image (matrix) to a qt pixmap object */
QPixmap MainWindow::toPixmap(const cv::Mat &image)
{
    QImage::Format format = image.type() != CV_8UC3 ? QImage::Format_Indexed8 : QImage::Format_RGB888;
    return QPixmap::fromImage(QImage((unsigned char*) image.data, image.cols, image.rows, format));
}

/** public slots **/

void MainWindow::on_detectDevices_clicked()
{
    // TODO: implement me
}

/* image settings */
void MainWindow::on_mode_currentIndexChanged(int mode)
{
    switch (mode)
    {
    case ViewMode::CAMERA:
    {
        this->showCameraImage();
        break;
    }
    case ViewMode::THRESHOLD:
        this->showThresholdImage();
        break;
    case ViewMode::CLUSTERS:
        this->showClusterImage();
        break;
    default:
        // TODO: debug
        break;
    }
}

void MainWindow::on_refresh_clicked()
{
    this->flyCounter.updateImages();
    this->on_mode_currentIndexChanged(this->getViewMode());
}

/* lead time setter that validates the input beforehand with the fly counter */
void MainWindow::on_leadTime_valueChanged(int time)
{
    this->flyCounter.validatedSetLeadTime(Seconds(time));
    this->updateTimeSpinners();
}

/* round time setter that validates the input beforehand with the fly counter */
void MainWindow::on_roundTime_valueChanged(int time)
{
    this->flyCounter.validatedSetRoundTime(Seconds(time));
    this->updateTimeSpinners();
}

/* shake time setter that validates the input beforehand with the fly counter */
void MainWindow::on_shakeTime_valueChanged(int time)
{
    this->flyCounter.validatedSetShakeTime(Seconds(time));
    this->updateTimeSpinners();
}

/* analysis parameters */
void MainWindow::on_epsilon_valueChanged(int epsilon)
{
    this->flyCounter.setEpsilon(epsilon);
}

void MainWindow::on_minPoints_valueChanged(int minPoints)
{
    this->flyCounter.setMinPoints(minPoints);
}

void MainWindow::on_pixelsPerFly_valueChanged(int pixelsPerFly)
{
    this->flyCounter.setPixelsPerFly(pixelsPerFly);
}

void MainWindow::on_threshold_valueChanged(int threshold)
{
    this->flyCounter.setThreshold(threshold);
}

void MainWindow::on_vialSize_valueChanged(int vialSize)
{
    this->flyCounter.setVialSize(vialSize);
}

/* result settings */
void MainWindow::on_outputPathBrowser_clicked()
{
    this->on_output_textEdited(QFileDialog::getExistingDirectory(this, "Output path", "/home"));
}

void MainWindow::on_output_textEdited(const QString& path)
{
    this->flyCounter.setOutput(path.toStdString());
}

void MainWindow::on_saveImages_toggled(bool checked)
{
    this->flyCounter.storeImages(checked);
}

/* experiment execution */
void MainWindow::on_startButton_clicked()
{
    this->userInterfaceEnabled(false);
    this->flyCounter.start();
}

void MainWindow::on_stopButton_clicked()
{
    this->userInterfaceEnabled(true);
    this->flyCounter.stop();
}

void MainWindow::updateImage()
{
    this->on_mode_currentIndexChanged(this->getViewMode());
}

void MainWindow::on_actionLoad_triggered()
{
    QString settings_path = QFileDialog::getOpenFileName(this, "Load settings file");
    if (settings_path.isEmpty()) return;
    QSettings settings(settings_path,QSettings::NativeFormat);

    this->ui->saveImages->toggled(settings.value(MainWindow::SAVE_IMAGES).toBool());
    this->ui->leadTime->valueChanged(settings.value(MainWindow::LEAD_TIME).toInt());
    this->ui->roundTime->valueChanged(settings.value(MainWindow::ROUND_TIME).toInt());
    this->ui->shakeTime->valueChanged(settings.value(MainWindow::SHAKE_TIME).toInt());
    this->ui->epsilon->valueChanged(settings.value(MainWindow::EPSILON).toInt());
    this->ui->minPoints->valueChanged(settings.value(MainWindow::MIN_POINTS).toInt());
    this->ui->pixelsPerFly->valueChanged(settings.value(MainWindow::PIXELS_PER_FLY).toInt());
    this->ui->threshold->valueChanged(settings.value(MainWindow::THRESHOLD).toInt());
    this->ui->vialSize->valueChanged(settings.value(MainWindow::VIAL_SIZE).toInt());
    this->ui->outputPath->textChanged(settings.value(MainWindow::OUTPUT_PATH).toString());
    this->ui->saveImages->toggled(settings.value(MainWindow::SAVE_IMAGES).toBool());
    // needs to go last as it triggers the image update
    this->ui->mode->setCurrentIndex(settings.value(MainWindow::MODE).toInt());
}

void MainWindow::on_actionSave_triggered()
{
    QString settings_path = QFileDialog::getSaveFileName(this, "Save settings file");
    QSettings settings(settings_path, QSettings::NativeFormat);

    settings.setValue(MainWindow::MODE,           this->ui->mode->currentIndex());
    settings.setValue(MainWindow::DISPLAY_VIALS,  this->ui->displayVials->isChecked());
    settings.setValue(MainWindow::LEAD_TIME,      this->ui->leadTime->value());
    settings.setValue(MainWindow::ROUND_TIME,     this->ui->roundTime->value());
    settings.setValue(MainWindow::SHAKE_TIME,     this->ui->shakeTime->value());
    settings.setValue(MainWindow::EPSILON,        this->ui->epsilon->value());
    settings.setValue(MainWindow::MIN_POINTS,     this->ui->minPoints->value());
    settings.setValue(MainWindow::PIXELS_PER_FLY, this->ui->pixelsPerFly->value());
    settings.setValue(MainWindow::THRESHOLD,      this->ui->threshold->value());
    settings.setValue(MainWindow::VIAL_SIZE,      this->ui->vialSize->value());
    settings.setValue(MainWindow::OUTPUT_PATH,    this->ui->outputPath->text());
    settings.setValue(MainWindow::SAVE_IMAGES,    this->ui->saveImages->isChecked());
}

MainWindow::~MainWindow()
{
    this->flyCounter.stop();
    delete this->ui;
}
