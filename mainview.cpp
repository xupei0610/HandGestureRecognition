#include "mainview.h"
#include "ui_mainview.h"

MainView::MainView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainView)
{
    ui->setupUi(this);

    // Setup Camera
    _camera = new QCamera;
    _camera_capture = new QCameraImageCapture(_camera);
    _camera_viewfinder = new QCameraViewfinder(this);
    _camera->setViewfinder(_camera_viewfinder);
    _camera_viewfinder->setAspectRatioMode(Qt::KeepAspectRatio);
    QImageEncoderSettings image_settings;
    image_settings.setResolution(ui->lblVideo->width(), ui->lblVideo->height());
    _camera_capture->setEncodingSettings(image_settings);
    _camera_capture->setBufferFormat(QVideoFrame::Format_ARGB32);

    // Setup Signal Triggers
    _camera_capture_timer = new QTimer();
    connect(_camera_capture_timer, SIGNAL(timeout()), this, SLOT(imageCapture()));
    connect(_camera_capture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processFrame(int, QImage)));
    connect(_camera_capture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this, SLOT(processCameraCaptureError(int,QCameraImageCapture::Error,QString)));
    connect(_camera, SIGNAL(error(QCamera::Error)), this, SLOT(processCameraError(QCamera::Error)));

    // Setup Hand Recongition Class
    _tracker = new HandAnalysis();

    // Setup child dialogs/windows
    setting_dialog = new SettingDialog(this);
    tracking_dialog = new TrackingDialog(this);
    connect(setting_dialog, SIGNAL(resetSetting()), this, SLOT(makeDefaultSetting()));
    connect(setting_dialog, SIGNAL(changeColorUpperBound(int,int, int)), _tracker, SLOT(setSkinColorUpperBound(int,int, int)));
    connect(setting_dialog, SIGNAL(changeColorLowerBound(int,int, int)), _tracker, SLOT(setSkinColorLowerBound(int,int, int)));
    connect(setting_dialog, SIGNAL(changeROI(int,int, int, int)), this, SLOT(setROI(int,int, int, int)));
    connect(setting_dialog, SIGNAL(changeErode(bool)), _tracker, SLOT(setErode(bool)));
    connect(setting_dialog, SIGNAL(changeDilate(bool)), _tracker, SLOT(setDilate(bool)));
    connect(setting_dialog, SIGNAL(changeMedianBlur(bool)), _tracker, SLOT(setMedianBlur(bool)));
    makeDefaultSetting();

    // Update tracking dialog if detect something
    connect(_tracker, SIGNAL(detected()), this, SLOT(updateTrackingDialog()));
    // Make Action according to number of fingers found
    connect(_tracker, SIGNAL(fingersFound(int)), this, SLOT(makeActionBasedOnFingersNum(int)));
    // Make Action according to the moverment of hand
    connect(_tracker, SIGNAL(handPos(int,int)), this, SLOT(makeActionBasedOnHandPosition(int,int)));
}

MainView::~MainView()
{
    delete _tracker;
    delete _camera;
    delete _camera_capture;
    delete _camera_viewfinder;
    delete _camera_capture_timer;
    delete setting_dialog;
    delete tracking_dialog;
    delete ui;
}

void MainView::makeDefaultSetting()
{

    setting_dialog->setMinH(DEFAULT_SKIN_COLOR_MIN_H);
    setting_dialog->setMinS(DEFAULT_SKIN_COLOR_MIN_S);
    setting_dialog->setMinV(DEFAULT_SKIN_COLOR_MIN_V);

    setting_dialog->setMaxH(DEFAULT_SKIN_COLOR_MAX_H);
    setting_dialog->setMaxS(DEFAULT_SKIN_COLOR_MAX_S);
    setting_dialog->setMaxV(DEFAULT_SKIN_COLOR_MAX_V);

    setting_dialog->setErode(DEFAULT_CONDUCT_ERODE);
    setting_dialog->setDilate(DEFAULT_CONDUCT_DILATE);
    setting_dialog->setMedianBlur(DEFAULT_CONDUCT_MEDIAN_BLUR);

    setting_dialog->setMinROIHorizon(48);
    setting_dialog->setMaxROIHorizon(98);
    setting_dialog->setMinROIVertical(2);
    setting_dialog->setMaxROIVertical(96);

}

void MainView::setROI(const int & start_x_percent,
                      const int & start_y_percent,
                      const int & end_x_percent,
                      const int & end_y_percent)
{
    // ROI should at least be 5x5
    _ROI = cv::Rect(ui->lblVideo->width() * start_x_percent/100,
                    ui->lblVideo->height() * start_y_percent/100,
                    ui->lblVideo->width() * (end_x_percent-start_x_percent)/100,
                    ui->lblVideo->height() * (end_y_percent-start_y_percent)/100);

    if (_ROI.width < 5)
    {
        _ROI.width = 5;
        if (_ROI.x + _ROI.width > ui->lblVideo->width())
            _ROI.x = ui->lblVideo->width() - _ROI.width;
    }
    if (_ROI.height < 5)
    {
        _ROI.height = 5;
        if (_ROI.y + _ROI.height > ui->lblVideo->height())
            _ROI.y = ui->lblVideo->height() - _ROI.height;
    }

    _tracker->setROI(_ROI);
}

void MainView::processFrame(const int &, const QImage & captured_image)
{

    _tracker->detect(QtCVImageConverter::QImage2CvMat(captured_image));

    ui->lblVideo->setPixmap(
                QtCVImageConverter::CvMat2QPixmap(
                    _tracker->getOriginalFrame()
                    ).scaled(
                    QSize(ui->lblVideo->width(),
                          ui->lblVideo->height()),
                    Qt::KeepAspectRatio)
                );

}

void MainView::makeActionBasedOnFingersNum(const int &num_of_fingers)
{
    ui->txtPanel->append(QString("Fingers Found: ") + QString::number(num_of_fingers));
}

void MainView::makeActionBasedOnHandPosition(const int & x, const int & y)
{
    ui->txtPanel->append(QString("Hand Position: ") + QString::number(x) + ", " + QString::number(y));

    //    QRect screen = QApplication::desktop()->screen(
    //                QApplication::desktop()->screenNumber(QCursor::pos())
    //                )->geometry();

    //    QCursor::setPos(static_cast<int>(static_cast<float>(x > 20 ? x - 20:0)/_ROI.width*screen.width()),
    //                    static_cast<int>(static_cast<float>(y > 20 ? y - 20:0)/_ROI.height*screen.height()));
}

void MainView::updateTrackingDialog()
{
    if (tracking_dialog->isVisible())
        tracking_dialog->updateWindow(QtCVImageConverter::CvMat2QPixmap(
                                          _tracker->getInterestedFrame()
                                          ),
                                      QtCVImageConverter::CvMat2QPixmap(
                                          _tracker->getFilteredFrame()
                                          ),
                                      QtCVImageConverter::CvMat2QPixmap(
                                          _tracker->getContourFrame()
                                          ),
                                      QtCVImageConverter::CvMat2QPixmap(
                                          _tracker->getConvexityFrame()
                                          )
                                      );
}

void MainView::on_btnSetting_clicked()
{
    setting_dialog->show();
    setting_dialog->raise();
}

void MainView::on_btnTracking_clicked()
{
    tracking_dialog->show();
    tracking_dialog->raise();
}

void MainView::on_btnStart_clicked()
{
    if (_camera_capture_timer->isActive() == true)
    {
        _camera_capture->blockSignals(true);
        _camera_capture_timer->stop();
        ui->btnStart->setText("Start");
    }
    else
    {
        try
        {
            _camera_capture->blockSignals(false);
            _camera->start();
            _camera_capture_timer->start(20);
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
    if (_camera_capture->isReadyForCapture())
    {
        _camera_capture->capture();
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
    ui->txtPanel->append(_camera->errorString());
}
