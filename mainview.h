#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QDialog>
#include <QPainter>
#include <QTimer>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <opencv2/opencv.hpp>

#include "qtcvimageconverter.h"
#include "handanalysis.h"
#include "settingdialog.h"

namespace Ui {
class MainView;
}

class MainView : public QDialog
{
    Q_OBJECT

public:
    explicit MainView(QWidget *parent = 0);
    ~MainView();

private slots:
    void imageCapture();
    void processCameraCaptureError(const int &, const QCameraImageCapture::Error &, const QString &);
    void processCameraError(const QCamera::Error &);
    void processFrame(const int &, const QImage &);
    void on_btnStart_clicked();
    void on_btnSetting_clicked();

private:
    QCamera * camera;
    QCameraImageCapture * camera_capture;
    QCameraViewfinder * camera_viewfinder;

    cv::Rect initial_ROI;

    HandAnalysis * tracker;

    QTimer * timer;

    Ui::MainView *ui;
    SettingDialog * setting_dialog;

};

#endif // MAINVIEW_H
