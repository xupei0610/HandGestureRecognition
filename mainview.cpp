#include "mainview.h"
#include "ui_mainview.h"

MainView::MainView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainView)
{
    ui->setupUi(this);

    // Setup Camera
    camera = new QCamera;
    camera_capture = new QCameraImageCapture(camera);
    camera_viewfinder = new QCameraViewfinder(this);
    camera->setViewfinder(camera_viewfinder);
    camera_viewfinder->setAspectRatioMode(Qt::KeepAspectRatio);
    QImageEncoderSettings image_settings;
    image_settings.setResolution(ui->lblVideo->width(), ui->lblVideo->height());
    camera_capture->setEncodingSettings(image_settings);
    camera_capture->setBufferFormat(QVideoFrame::Format_ARGB32);

    // Setup Triggers
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(imageCapture()));
    connect(camera_capture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processFrame(int, QImage)));
    connect(camera_capture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this, SLOT(processCameraCaptureError(int,QCameraImageCapture::Error,QString)));
    connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(processCameraError(QCamera::Error)));

    tracker = new HandAnalysis();
    initial_ROI = cv::Rect(ui->lblVideo->width()/2-20, 20, ui->lblVideo->width()/2, ui->lblVideo->height()-40);


    setting_dialog = new SettingDialog(this);
//    setting_dialog->setMinH(HandAnalysis::LowerBoundH);
//    setting_dialog->setMaxH(HandAnalysis::UpperBoundH);
//    setting_dialog->setMinS(HandAnalysis::LowerBoundS);
//    setting_dialog->setMaxS(HandAnalysis::UpperBoundS);
//    setting_dialog->setMinV(HandAnalysis::LowerBoundV);
//    setting_dialog->setMaxV(HandAnalysis::UpperBoundV);

    setting_dialog->setMinH(30);
    setting_dialog->setMinS(0);
    setting_dialog->setMinV(100);

    setting_dialog->setMaxH(180);
    setting_dialog->setMaxS(255);
    setting_dialog->setMaxV(255);

    setting_dialog->setErode(false);
    setting_dialog->setDilate(false);
    setting_dialog->setMedianBlur(false);

}

MainView::~MainView()
{
    if (tracker != nullptr)
        delete tracker;
    delete ui;
}

void MainView::on_btnStart_clicked()
{
    if (timer->isActive() == true)
    {
        camera_capture->blockSignals(true);
        timer->stop();
        ui->btnStart->setText("Start");
    }
    else
    {
        try
        {
            camera_capture->blockSignals(false);
            camera->start();
            timer->start(20);
            ui->btnStart->setText("Pause");
        }
        catch (...)
        {
            ui->txtPanel->append(QString("Error: Camera cannot be accessed."));
        }
    }
}

void MainView::imageCapture()
{
    if (camera_capture->isReadyForCapture())
    {
        camera_capture->capture();
    }
    else
    {
        ui->txtPanel->append("Failed to Capture Image.");
    }
}

void MainView::processCameraCaptureError(const int &, const QCameraImageCapture::Error &, const QString & error_string)
{
    ui->txtPanel->append("Failed to Capture Image.");
    ui->txtPanel->append(error_string);
}

void MainView::processCameraError(const QCamera::Error & )
{
    ui->txtPanel->append("Failed to Load Image.");
    ui->txtPanel->append(camera->errorString());
}

void MainView::processFrame(const int &, const QImage & captured_image)
{

    if (tracker->isTracking())
    {
    }
    else
    {
        tracker->detect(QtCVImageConverter::QImage2CvMat(captured_image), initial_ROI,
                        cv::Scalar(setting_dialog->getMinH(), setting_dialog->getMinS(), setting_dialog->getMinV()),
                        cv::Scalar(setting_dialog->getMaxH(), setting_dialog->getMaxS(), setting_dialog->getMaxV()),
                        setting_dialog->getErode(), setting_dialog->getDilate(), setting_dialog->getMedianBlur()
                        );
    }

    ui->txtPanel->append(QString("Fingers Found: ") + QString::number(tracker->fingersFound()));


    ui->lblVideo->setPixmap(
                    QtCVImageConverter::CvMat2QPixmap(
                        tracker->getOriginalFrame()
                        ).scaled(
                        QSize(ui->lblVideo->width(),
                              ui->lblVideo->height()),
                        Qt::KeepAspectRatio)
                    );


    ui->lblContour->setPixmap(
                QtCVImageConverter::CvMat2QPixmap(
                    tracker->getContourFrame()
                    ).scaled(
                    QSize(ui->lblContour->width(),
                          ui->lblContour->height()),
                    Qt::KeepAspectRatio)
                );

    ui->lblConvexity->setPixmap(
                QtCVImageConverter::CvMat2QPixmap(
                    tracker->getConvexHullFrame()
                    ).scaled(
                    QSize(ui->lblConvexity->width(),
                          ui->lblConvexity->height()),
                    Qt::KeepAspectRatio)
                );

//    tracker->getOriginalFrame();

//    ui->lblVideo->setPixmap(
//                QtCVImageConverter::CvMat2QPixmap(
//                    tracker->getOriginalFrame()
//                    ).scaled(
//                    QSize(ui->lblVideo->width(),
//                          ui->lblVideo->height()),
//                    Qt::KeepAspectRatio)
//                );
//    ui->lblYCrCb->setPixmap(
//                QtCVImageConverter::CvMat2QPixmap(
//                    tracker->getYCrCbFrame()
//                    ).scaled(
//                    QSize(ui->lblYCrCb->width(),
//                          ui->lblYCrCb->height()),
//                    Qt::KeepAspectRatio)
//                );
}


void MainView::on_btnSetting_clicked()
{
    setting_dialog->show();
}
