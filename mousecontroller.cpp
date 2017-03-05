#include "mousecontroller.h"

MouseController::MouseController()
{
    _kalman_filter = new KalmanFilter(4, 2);
    _kalman_filter->statePre = Mat::zeros(4,1,CV_32F);
    _kalman_filter->transitionMatrix = Mat::eye(4,4,CV_32F);
    setIdentity(_kalman_filter->measurementMatrix);
    setIdentity(_kalman_filter->processNoiseCov, Scalar::all(1e-4));
    setIdentity(_kalman_filter->measurementNoiseCov, Scalar::all(1e-3));
    setIdentity(_kalman_filter->errorCovPost, Scalar::all(0.2));

    _action_timer = new QTimer();
    _action_timer->setSingleShot(true);
    connect(_action_timer, SIGNAL(timeout()), this, SLOT(_releaseMouse()));
}

MouseController::~MouseController()
{
    if (_kalman_filter != nullptr)
        delete _kalman_filter;
}

Point MouseController::estimateCursorPos(const Point & tracked_point)
{
    _kalman_filter->predict();
    auto estimated = _kalman_filter->correct((Mat_<float>(2,1) << tracked_point.x, tracked_point.y));
    return Point(estimated.at<float>(0), estimated.at<float>(1));
}

MouseController::MouseAction MouseController::makeAction(const int & cursor_pos_x, const int & cursor_pos_y,
                                                         const std::vector<Point> & finger_points)
{
    int num_of_fingers = finger_points.size();

    // make action if the action interval is longer than the threshold
    // or make action directly for movement and dragging action

    if (num_of_fingers == 3)
    {
        if (_has_released == false || (_action_timer->isActive() == false && _action_frame_count[MOUSE_ACTION_DRAG] > DEFAULT_MOUSE_SENSITIVITY))
        {
            _action_timer->start(_action_interval);
            clearActionFrameCount();
            mouseDrag(cursor_pos_x, cursor_pos_y);
            return MOUSE_ACTION_DRAG;
        }
        else
        {
            _action_frame_count[MOUSE_ACTION_DRAG]++;
        }
    }
    else if (_action_timer->isActive() == false)
    {

        if (num_of_fingers == 2)
        {
            if (_action_frame_count[MOUSE_ACTION_SINGLE_CLICK] > DEFAULT_MOUSE_SENSITIVITY)
            {
                _action_timer->start(_action_interval);
                clearActionFrameCount();
                mouseLeftClick(cursor_pos_x, cursor_pos_y);
                return MOUSE_ACTION_SINGLE_CLICK;
            }
            else
            {
                _action_frame_count[MOUSE_ACTION_SINGLE_CLICK]++;
            }
        }
        else if (num_of_fingers == 4)
        {
            if (_action_frame_count[MOUSE_ACTION_RIGHT_CLICK] > DEFAULT_MOUSE_SENSITIVITY)
            {
                _action_timer->start(_action_interval);
                clearActionFrameCount();
                mouseRightClick(cursor_pos_x, cursor_pos_y);
                return MOUSE_ACTION_RIGHT_CLICK;
            }
            else
            {
                _action_frame_count[MOUSE_ACTION_RIGHT_CLICK]++;
            }
        }
        else if (num_of_fingers >= 5)
        {
            if (_action_frame_count[MOUSE_ACTION_DOUBLE_CLICK] > DEFAULT_MOUSE_SENSITIVITY)
            {
                _action_timer->start(_action_interval);
                clearActionFrameCount();
                mouseDoubleClick(cursor_pos_x, cursor_pos_y);
                return MOUSE_ACTION_DOUBLE_CLICK;
            }
            else
            {
                _action_frame_count[MOUSE_ACTION_DOUBLE_CLICK]++;
            }
        }
    }
    _action_frame_count[0]++;
    if (_action_frame_count[0] > 2*DEFAULT_MOUSE_SENSITIVITY)
        clearActionFrameCount();
    mouseMove(cursor_pos_x, cursor_pos_y);
    return MOUSE_ACTION_MOVE;
}

void MouseController::clearActionFrameCount()
{
    for (auto & c : _action_frame_count)
        c = 0;
}

void MouseController::mouseMove(const int &x, const int &y)
{
    _releaseMouse();
#ifdef __APPLE__
    mouse::CGEventRef mouse_move = mouse::CGEventCreateMouseEvent(NULL,
                                                                  mouse::kCGEventMouseMoved,
                                                                  mouse::CGPointMake(x, y),
                                                                  mouse::kCGMouseButtonLeft);
    mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_move);
    mouse::CFRelease(mouse_move);
#elif defined(_WIN_)
    // TODO MouseMoveEvent for windows
#else
    // TODO MouseMoveEvent using X11
#endif
}

void MouseController::mouseLeftClick(const int &x, const int &y)
{
    _releaseMouse();
#ifdef __APPLE__
    mouse::CGEventRef mouse_click = mouse::CGEventCreateMouseEvent(NULL,
                                                                   mouse::kCGEventLeftMouseDown,
                                                                   mouse::CGPointMake(x, y),
                                                                   mouse::kCGMouseButtonLeft);
    mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_click);
    mouse::CGEventSetType(mouse_click, mouse::kCGEventLeftMouseUp);
    mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_click);
    mouse::CFRelease(mouse_click);
#elif defined(_WIN_)
    // TODO MouseLeftClickEvent for windows
#else
    // TODO MouseLeftClickEvent using X11
#endif
}

void MouseController::mouseDoubleClick(const int &x, const int &y)
{
    _releaseMouse();
#ifdef __APPLE__
    mouse::CGEventRef mouse_click = mouse::CGEventCreateMouseEvent(NULL,
                                                                   mouse::kCGEventLeftMouseDown,
                                                                   mouse::CGPointMake(x, y),
                                                                   mouse::kCGMouseButtonLeft);
    mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_click);
    mouse::CGEventSetType(mouse_click, mouse::kCGEventLeftMouseUp);
    mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_click);
    mouse::CGEventSetIntegerValueField(mouse_click, mouse::kCGMouseEventClickState, 2);
    mouse::CGEventSetType(mouse_click, mouse::kCGEventLeftMouseDown);
    mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_click);
    mouse::CGEventSetType(mouse_click, mouse::kCGEventLeftMouseUp);
    mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_click);
    mouse::CFRelease(mouse_click);
#elif defined(_WIN_)
    // TODO MouseLeftClickEvent for windows
#else
    // TODO MouseLeftClickEvent using X11
#endif
}

void MouseController::mouseRightClick(const int &x, const int &y)
{
    _releaseMouse();
#ifdef __APPLE__
    mouse::CGEventRef mouse_click = mouse::CGEventCreateMouseEvent(NULL,
                                                                   mouse::kCGEventRightMouseDown,
                                                                   mouse::CGPointMake(x, y),
                                                                   mouse::kCGMouseButtonRight);

    mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_click);
    mouse::CGEventSetType(mouse_click, mouse::kCGEventRightMouseUp);
    mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_click);
    mouse::CFRelease(mouse_click);
#elif defined(_WIN_)
    // TODO MouseRightClickEvent for windows
#else
    // TODO MouseRightClickEvent using X11
#endif
}

void MouseController::mouseDrag(const int &x, const int &y)
{
#if defined(__APPLE__)
    mouse::CGEventRef mouse_drag;
    if (_has_released == true)
    {
        mouse_drag = mouse::CGEventCreateMouseEvent(NULL,
                                                    mouse::kCGEventLeftMouseDown,
                                                    mouse::CGPointMake(x, y),
                                                    mouse::kCGMouseButtonLeft);
    }
    else
    {
        mouse_drag = mouse::CGEventCreateMouseEvent(NULL,
                                                    mouse::kCGEventMouseMoved,
                                                    mouse::CGPointMake(x, y),
                                                    mouse::kCGMouseButtonLeft);
    }
    mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_drag);
    mouse::CFRelease(mouse_drag);
    _has_released = false;
    _last_drag_pos = std::make_pair(x,y);

#elif defined(_WIN_)
    // TODO MouseDragEvent for windows
#else
    // TODO MouseDragEvent using X11
#endif
}

void MouseController::_releaseMouse()
{
    if (_has_released == false)
    {
#if defined(__APPLE__)
        mouse::CGEventRef mouse_left_up = mouse::CGEventCreateMouseEvent(NULL,
                                                                         mouse::kCGEventLeftMouseUp,
                                                                         mouse::CGPointMake(_last_drag_pos.first, _last_drag_pos.second),
                                                                         mouse::kCGMouseButtonLeft);
        mouse::CGEventPost(mouse::kCGHIDEventTap, mouse_left_up);
        mouse::CFRelease(mouse_left_up);
        _has_released = true;
#elif defined(_WIN_)
        // TODO MouseLeftClickReleasaeEvent for windows
#else
        // TODO MouseLeftClickReleaseEvent using X11
#endif
        //        if (emit_signal == true)
        emit mouseReleased();
    }
}

void MouseController::setActionInterval(const int &interval_in_ms)
{
    _action_interval = interval_in_ms;
}

bool MouseController::hasReleased()
{
    return _has_released;
}
