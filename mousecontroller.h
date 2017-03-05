#ifndef MOUSECONTROL_H
#define MOUSECONTROL_H

#include <QObject>
#include <QTimer>
#include <opencv2/opencv.hpp>

namespace mouse {
#if defined(__APPLE__)
    #include <ApplicationServices/ApplicationServices.h>
#elif defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
    #define _WIN_
    #include <windows.h>
#else
    #include <X11.h>
#endif
}

using namespace cv;
#ifdef USE_GPU
using namespace cv::cuda;
#endif

#define DEFAULT_MOUSE_ACTION_INTERVAL  500 // ms
#define DEFAULT_MOUSE_SENSITIVITY 5 // frames
#define MOUSE_ACTION_MOVE 1
#define MOUSE_ACTION_DRAG 3
#define MOUSE_ACTION_SINGLE_CLICK 2
#define MOUSE_ACTION_DOUBLE_CLICK 5
#define MOUSE_ACTION_RIGHT_CLICK  4

class MouseController : public QObject
{
    Q_OBJECT
signals:
    void mouseReleased();

public:
    typedef int MouseAction;

    MouseController();
    ~MouseController();
    Point estimateCursorPos(const Point & tracked_point);
    int makeAction(const int & cursor_pos_x, const int & cursor_pos_y,
                    const std::vector<Point> & finger_points);
    void clearActionFrameCount();
    void mouseMove(const int & x, const int & y);
    void mouseLeftClick(const int &x, const int &y);
    void mouseDoubleClick(const int &x, const int &y);
    void mouseDrag(const int &x, const int &y);
    void mouseRightClick(const int &x, const int &y);
    bool hasReleased();

public slots:
    void setActionInterval(const int & interval_in_ms);

protected slots:
    void _releaseMouse();

protected:
    KalmanFilter * _kalman_filter;

    std::vector<int> _action_frame_count{0,0,0,0,0,0};
    QTimer * _action_timer; // Timer who counts the interval between two actions
    bool _has_released = true; // if mouse button has been released
    int _action_interval = DEFAULT_MOUSE_ACTION_INTERVAL;
    std::pair<int,int> _last_drag_pos;  // last pos of drag action, used to release mouse left button

};

#endif // MOUSECONTROL_H
