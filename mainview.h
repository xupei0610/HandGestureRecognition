#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QDialog>
#include <QTimer>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QScreen>
#include <QDesktopWidget>
#include <QMessageBox>

#include <opencv2/opencv.hpp>

#include "qtcvimageconverter.h"
#include "handdetector.h"
#include "mousecontroller.h"
#include "settingdialog.h"
#include "monitordialog.h"

using namespace cv;
#ifdef USE_GPU
using namespace cv::cuda;
#endif

#define DEFAULT_CAMERA_FPS 50

namespace Ui {
class MainView;
}

class MainView final: public QDialog
{
    Q_OBJECT

public:
    explicit MainView(QWidget *parent = 0);
    ~MainView();

public slots:
    void makeDefaultSetting();
    void setROI(const int & start_x_percent,
                const int & start_y_percent,
                const int & end_x_percent,
                const int & end_y_percent);
    void setActionSensitivity(const int &sensitivity_in_ms);
    void setCameraFPS(const int & fps);

private slots:
    // Update Tracking Dialog
    void updateTrackingDialog();
    // Process Image captured by camera
    void processFrame(const Mat&);
    void processFrame(const int &, const QImage &);
    void infoMouseReleased();
    // Capture Image by camera
    void imageCapture();
    // Start/Stop Recognition
    void on_btnStart_clicked();
    // Open Setting Dialog
    void on_btnSetting_clicked();
    // Open Tracking Dialog
    void on_btnMonitor_clicked();
    // Set background for substractor
    void on_btnSetBackground_clicked();
#ifdef USE_QCAMERA
    // Process Campera Errors
    void processCameraCaptureError(const int &, const QCameraImageCapture::Error &, const QString &);
    void processCameraError(const QCamera::Error &);
#endif
    // Show Error Message when failing to load the camera
    void showCameraErrorMessage();

private:
    // Widgets for Camera
    QTimer * _camera_capture_timer = nullptr;
#ifndef USE_QCAMERA
    VideoCapture * _camera = nullptr;
#else
    QCamera * _camera = nullptr;
    QCameraImageCapture * _camera_capture = nullptr;
    QCameraViewfinder * _camera_viewfinder = nullptr;
#endif


    // Hand Recognition Class
    HandDetector * _detector = nullptr;
    // Mouse Controller Class
    MouseController * _mouse = nullptr;

    // region of interesting
    Rect _ROI;

    // Screen Information
    QDesktopWidget _qt_widget;

    // tracking has begun or not
    bool _is_tracking = false;

    // FPS of camera
    int _camera_FPS = DEFAULT_CAMERA_FPS;

    // Main UI
    Ui::MainView *_ui;
    // Setting Dialog
    SettingDialog * _setting_dialog;
    // Tracking Dialog
    MonitorDialog * _monitor_dialog;
    // Error Message Box
    QMessageBox * _error_message_box;

};

#endif // MAINVIEW_H
