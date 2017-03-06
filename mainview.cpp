#include "mainview.h"
#include "ui_mainview.h"

MainView::MainView(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::MainView)
{
    _ui->setupUi(this);

    // Setup Hand Recongition Class
    _detector = new HandDetector();
    // Setup Mouse Controller Class
    _mouse = new MouseController();

    // Setup child dialogs/windows
    _setting_dialog = new SettingDialog(this);
    _monitor_dialog = new MonitorDialog(this);
    _error_message_box = new QMessageBox(this);

    // Build connections between detector and dialogs
    connect(_setting_dialog, SIGNAL(resetSetting()), this, SLOT(makeDefaultSetting()));
    connect(_setting_dialog, SIGNAL(changeColorUpperBound(int,int, int)), _detector, SLOT(setSkinColorUpperBound(int,int,int)));
    connect(_setting_dialog, SIGNAL(changeColorLowerBound(int,int, int)), _detector, SLOT(setSkinColorLowerBound(int,int,int)));
    connect(_setting_dialog, SIGNAL(changeROI(int,int, int, int)), this, SLOT(setROI(int,int,int,int)));
    connect(_setting_dialog, SIGNAL(changeDetectionArea(int)), _detector, SLOT(setDetectionArea(int)));
    connect(_setting_dialog, SIGNAL(changeActionInterval(int)), _mouse, SLOT(setActionInterval(int)));
    connect(_setting_dialog, SIGNAL(changeSamplingFPS(int)), this, SLOT(setCameraFPS(int)));
    connect(_setting_dialog, SIGNAL(changeSensitivity(int)), this, SLOT(setActionSensitivity(int)));
    connect(_mouse, SIGNAL(mouseReleased()), this, SLOT(infoMouseReleased()));

    // Load default setting
    makeDefaultSetting();

    // Setup Camera
    _camera_capture_timer = new QTimer();
#ifndef USE_QCAMERA
    _camera = new VideoCapture();
    connect(_camera_capture_timer, SIGNAL(timeout()), this, SLOT(imageCapture()));
#else
    _camera = new QCamera;
    _camera_capture = new QCameraImageCapture(_camera);
    _camera_viewfinder = new QCameraViewfinder(this);
    _camera->setViewfinder(_camera_viewfinder);
    _camera_viewfinder->setAspectRatioMode(Qt::KeepAspectRatio);
    QImageEncoderSettings image_settings;
    image_settings.setResolution(_ui->lblVideo->width(), _ui->lblVideo->height());
    _camera_capture->setEncodingSettings(image_settings);
    _camera_capture->setBufferFormat(QVideoFrame::Format_ARGB32);
    // Setup Signal Triggers for Camera
    connect(_camera_capture_timer, SIGNAL(timeout()), this, SLOT(imageCapture()));
    connect(_camera_capture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processFrame(int, QImage)));
    connect(_camera_capture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this, SLOT(processCameraCaptureError(int,QCameraImageCapture::Error,QString)));
    connect(_camera, SIGNAL(error(QCamera::Error)), this, SLOT(processCameraError(QCamera::Error)));
#endif

}

MainView::~MainView()
{
    delete _detector;
    delete _mouse;
#ifndef USE_QCAMERA
    delete _camera;
#else
    delete _camera;
    delete _camera_capture;
    delete _camera_viewfinder;
#endif
    delete _camera_capture_timer;
    delete _error_message_box;
    delete _setting_dialog;
    delete _monitor_dialog;
    delete _ui;
}

void MainView::makeDefaultSetting()
{
    _setting_dialog->setMinH(DEFAULT_SKIN_COLOR_MIN_H);
    _setting_dialog->setMinS(DEFAULT_SKIN_COLOR_MIN_S);
    _setting_dialog->setMinV(DEFAULT_SKIN_COLOR_MIN_V);

    _setting_dialog->setMaxH(DEFAULT_SKIN_COLOR_MAX_H);
    _setting_dialog->setMaxS(DEFAULT_SKIN_COLOR_MAX_S);
    _setting_dialog->setMaxV(DEFAULT_SKIN_COLOR_MAX_V);

    _setting_dialog->setDetectionArea(DEFAULT_DETECTION_AREA);
    _setting_dialog->setSensitivity(DEFAULT_MOUSE_SENSITIVITY*1000/DEFAULT_CAMERA_FPS);
    _setting_dialog->setSamplingFPS(DEFAULT_CAMERA_FPS);
    _setting_dialog->setActionInterval(DEFAULT_MOUSE_ACTION_INTERVAL);

    _setting_dialog->setMinROIHorizon(48);
    _setting_dialog->setMaxROIHorizon(98);
    _setting_dialog->setMinROIVertical(2);
    _setting_dialog->setMaxROIVertical(96);

}

void MainView::setCameraFPS(const int & fps)
{
    _camera_FPS = fps;
    setActionSensitivity(_setting_dialog->getSensitivity());
    if (_camera_capture_timer != nullptr && _camera_capture_timer->isActive())
        _camera_capture_timer->start(_camera_FPS);
}

void MainView::setActionSensitivity(const int & sensitivity_in_ms)
{
    _mouse->setActionSensitivity(sensitivity_in_ms*_camera_FPS/1000);
}

void MainView::setROI(const int & start_x_percent,
                      const int & start_y_percent,
                      const int & end_x_percent,
                      const int & end_y_percent)
{
    // ROI should at least be 9x9 in order to avoid errors caused by morphologyEx
    _ROI = Rect(_ui->lblVideo->width() * start_x_percent/100,
                _ui->lblVideo->height() * start_y_percent/100,
                _ui->lblVideo->width() * (end_x_percent-start_x_percent)/100,
                _ui->lblVideo->height() * (end_y_percent-start_y_percent)/100);

    if (_ROI.width < 9)
    {
        _ROI.width = 9;
        if (_ROI.x + _ROI.width > _ui->lblVideo->width())
            _ROI.x = _ui->lblVideo->width() - _ROI.width;
    }
    if (_ROI.height < 9)
    {
        _ROI.height = 9;
        if (_ROI.y + _ROI.height > _ui->lblVideo->height())
            _ROI.y = _ui->lblVideo->height() - _ROI.height;
    }

    _detector->setROI(_ROI);
}

void MainView::processFrame(const Mat& captured_frame)
{
    if (_detector->detect(captured_frame))
    {
        auto cursor_pos = _mouse->estimateCursorPos(_detector->getTrackedPoint());

        circle(_detector->getOriginalFrame()(_ROI), _detector->getTrackedPoint(), 6, _detector->ColorRed, -1);
        circle(_detector->getOriginalFrame()(_ROI), cursor_pos, 6, _detector->ColorBlue, 2);

        if (_is_tracking == true)
        {
            cursor_pos.x = static_cast<float>(cursor_pos.x-50)/(_ROI.width-100)*_qt_widget.geometry().width();
            cursor_pos.y = static_cast<float>(cursor_pos.y-100)/(_ROI.height-200)*_qt_widget.geometry().height();

            bool has_released = _mouse->hasReleased();
            auto result = _mouse->makeAction(cursor_pos.x, cursor_pos.y, _detector->getFingerPoints());
            if (result == MOUSE_ACTION_SINGLE_CLICK)
                _ui->txtPanel->append("Single Click");
            else if (result == MOUSE_ACTION_DOUBLE_CLICK)
                _ui->txtPanel->append("Double Click");
            else if (result == MOUSE_ACTION_RIGHT_CLICK)
                _ui->txtPanel->append("Right Click");
            else if (result == MOUSE_ACTION_DRAG && has_released == true)
                _ui->txtPanel->append("Drag Begining");
        }
    }

    updateTrackingDialog();
    _ui->lblVideo->setPixmap(
                QtCVImageConverter::CvMat2QPixmap(
                    _detector->getOriginalFrame()
                    ).scaled(
                    QSize(_ui->lblVideo->width(),
                          _ui->lblVideo->height()),
                    Qt::KeepAspectRatio)
                );
}

void MainView::infoMouseReleased()
{
    _ui->txtPanel->append("Drag Released");
}

void MainView::processFrame(const int &, const QImage & captured_image)
{
    processFrame(QtCVImageConverter::QImage2CvMat(captured_image));
}

void MainView::updateTrackingDialog()
{
    if (_monitor_dialog->isVisible())
        _monitor_dialog->updateWindow(QtCVImageConverter::CvMat2QPixmap(
                                           _detector->getInterestedFrame()
                                           ),
                                       QtCVImageConverter::CvMat2QPixmap(
                                           _detector->getFilteredFrame()
                                           ),
                                       QtCVImageConverter::CvMat2QPixmap(
                                           _detector->getContourFrame()
                                           ),
                                       QtCVImageConverter::CvMat2QPixmap(
                                           _detector->getConvexityFrame()
                                           )
                                       );
}

void MainView::on_btnSetting_clicked()
{
    _setting_dialog->show();
    _setting_dialog->raise();
    _setting_dialog->activateWindow();
}

void MainView::on_btnMonitor_clicked()
{
    _monitor_dialog->show();
    _monitor_dialog->raise();
    _setting_dialog->activateWindow();
}

void MainView::on_btnStart_clicked()
{

    if (_camera_capture_timer->isActive() == false)
    {
#ifndef USE_QCAMERA
        _camera->open(0);
        if (!_camera->isOpened())
        {
            _ui->txtPanel->append(QString("Error: Camera cannot be accessed."));
            showCameraErrorMessage();
            return;
        }
#else
        try
        {
            _camera->start();
        }
        catch (...)
        {
            _ui->txtPanel->append(QString("Error: Camera cannot be accessed."));
            showCameraErrorMessage();
            return;
        }
#endif
        _camera_capture_timer->start(1000/_camera_FPS);
        _ui->btnStart->setText("Track");
        _is_tracking = false;
    }
    else if (_is_tracking == true)
    {
        _is_tracking = false;
        _ui->btnStart->setText("Track");
    }
    else
    {
        _is_tracking = true;
        _ui->btnStart->setText("Pause");
    }
}

void MainView::imageCapture()
{
#ifndef USE_QCAMERA
    if (_camera->isOpened())
    {
        Mat captured_frame;
        _camera->read(captured_frame);
        cv::resize(captured_frame, captured_frame,
                   Size(captured_frame.cols*_ui->lblVideo->height()/captured_frame.rows, _ui->lblVideo->height())
                   );
        processFrame(captured_frame(Rect((captured_frame.cols - _ui->lblVideo->width())/2, 0, _ui->lblVideo->width(), _ui->lblVideo->height())));
    }
#else
    if (_camera_capture->isReadyForCapture())
    {
        _camera_capture->capture();
    }
#endif
    else
    {
        _ui->txtPanel->append("Failed to Capture Image.");
        qWarning() << "Failed to Capture Image.";
        showCameraErrorMessage();
    }
}

#ifdef USE_QCAMERA
void MainView::processCameraCaptureError(const int &, const QCameraImageCapture::Error &, const QString & error_string)
{
    _ui->txtPanel->append("Failed to Capture Image.");
    qWarning() << "Failed to Capture Image.";
    _ui->txtPanel->append(error_string);
    showCameraErrorMessage();
}

void MainView::processCameraError(const QCamera::Error & )
{
    _ui->txtPanel->append("Failed to Capture Image.");
    qWarning() << "Failed to Capture Image.";
    _ui->txtPanel->append(_camera->errorString());
    showCameraErrorMessage();
}
#endif

void MainView::showCameraErrorMessage()
{
    _camera_capture_timer->stop();
    _ui->btnStart->setText("Start");
    _error_message_box->warning(this, QString("Error"), QString("Failed to load camera."));
    qWarning() << "Failed to load camera.";
    _error_message_box->show();
    _error_message_box->raise();
}

void MainView::on_btnSetBackground_clicked()
{
    auto input_frame = _detector->getOriginalFrame();
    if (!input_frame.empty())
    {
        _detector->setBackgroundImage(input_frame(_ROI));
        _ui->lblBackground->setPixmap(
                    QtCVImageConverter::CvMat2QPixmap(
                        input_frame(_ROI)
                        ).scaled(
                        QSize(_ui->lblBackground->width(),
                              _ui->lblBackground->height()),
                        Qt::KeepAspectRatio)
                    );
    }
}

