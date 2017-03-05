#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QDialog>
#include <QTimer>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QScreen>
#include <QDesktopWidget>

#include <opencv2/opencv.hpp>

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
    #define __WIN__
    #include <windows.h>
#else
    #ifdef __APPLE__
    #include <ApplicationServices/ApplicationServices.h>
    #else
        #include <X11/Xlib.h>

    #endif
#endif

#include "qtcvimageconverter.h"
#include "handanalysis.h"
#include "settingdialog.h"
#include "trackingdialog.h"

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
    void makeActionBasedOnFingersNum(const int & num_of_fingers);
    void makeActionBasedOnHandPosition(const int & x, const int & y);
    void makeAction(const int & x, const int & y, const int & num_of_fingers);

private slots:
    // Update Tracking Dialog
    void updateTrackingDialog();
    // Process Image captured by camera
    void processFrame(const int &, const QImage &);
    // Capture Image by camera
    void imageCapture();
    // Start/Stop Recognition
    void on_btnStart_clicked();
    // Open Setting Dialog
    void on_btnSetting_clicked();
    // Open Tracking Dialog
    void on_btnTracking_clicked();
    // Process Trivial Errors
    void processCameraCaptureError(const int &, const QCameraImageCapture::Error &, const QString &);
    void processCameraError(const QCamera::Error &);

private:
    // Widgets for Camera
    QCamera * _camera;
    QCameraImageCapture * _camera_capture;
    QCameraViewfinder * _camera_viewfinder;
    QTimer * _camera_capture_timer;

    // Hand Recognition Class
    HandAnalysis * _tracker;
    // region of interesting
    cv::Rect _ROI;

    // Main UI
    Ui::MainView *ui;
    // Setting Dialog
    SettingDialog * setting_dialog;
    // Tracking Dialog
    TrackingDialog * tracking_dialog;
};

#endif // MAINVIEW_H
