#include "handdetector.h"

HandDetector::HandDetector()
{
#ifdef USE_GPU
    _background_subtractor = cuda::createBackgroundSubtractorMOG2(500, 16, false);
    _open_filter = cuda::createMorphologyFilter(MORPH_OPEN, CV_8UC1, _morph_kernel);
    _close_filter= cuda::createMorphologyFilter(MORPH_CLOSE,CV_8UC1, _morph_kernel);
    _gaussian_filter = cuda::createGaussianFilter(CV_8UC1, CV_8UC1, Size(3,3), 0.8);
#else
    _background_subtractor = createBackgroundSubtractorMOG2(500, 16, false);
#endif
    _losing_tracking = new QTimer();
    _losing_tracking->setSingleShot(true);
    connect(_losing_tracking, SIGNAL(timeout()), this, SLOT(lost_tracking()));
}

HandDetector::~HandDetector()
{
    if (_losing_tracking != nullptr)
        delete _losing_tracking;
}

void HandDetector::lost_tracking()
{
    _is_tracking = false;
}

void HandDetector::setBackgroundImage(const Mat& input_frame)
{
#ifdef USE_GPU
    _background_subtractor->apply(input_frame, GpuMat(), 1);
#else
    _background_subtractor->apply(input_frame, _bg, 1);
#endif
    _has_set_bg = true;
}

bool HandDetector::detect(const Mat& input_frame)
{
    _finger_points.clear();
    cv::flip(input_frame, _original_frame, 1);
    _detect_hand();
    return _track_finger();
}

void HandDetector::_detect_hand()
{
    _original_frame(_ROI).copyTo(_interested_frame);

    // Skin Color Detection and Preprocessing
#ifdef USE_GPU
    GpuMat tmp_gpumat;
    if (_has_set_bg == true)
    {
        _background_subtractor->apply(GpuMat(_interested_frame), tmp_gpumat, 0);
    }
    else
    {
        _filtered_frame = _interested_frame;  // copy implicitly later
        cv::cvtColor(_filtered_frame, _filtered_frame, COLOR_RGB2HSV);
        inRange(_filtered_frame, _skin_color_lower_bound, _skin_color_upper_bound, _filtered_frame);
        tmp_gpumat.upload(_filtered_frame);
    }
    _gaussian_filter->apply(tmp_gpumat, tmp_gpumat);
    cuda::threshold(tmp_gpumat, tmp_gpumat, 10, 255, THRESH_BINARY);
    _open_filter->apply(tmp_gpumat, tmp_gpumat);
    _close_filter->apply(tmp_gpumat, tmp_gpumat);
    tmp_gpumat.download(_filtered_frame);
#else
    if (_has_set_bg == true)
    {
        _background_subtractor->apply(_interested_frame, _bg, 0);
        _interested_frame.copyTo(_filtered_frame, _bg);
    }
    else
    {
        _interested_frame.copyTo(_filtered_frame);
    }
    cvtColor(_filtered_frame, _filtered_frame, COLOR_RGB2HSV);
    inRange(_filtered_frame, _skin_color_lower_bound, _skin_color_upper_bound, _filtered_frame);
    GaussianBlur(_filtered_frame, _filtered_frame, Size(3,3), 0.8);
    threshold(_filtered_frame, _filtered_frame, 10, 255, THRESH_BINARY);
    morphologyEx(_filtered_frame, _filtered_frame, MORPH_OPEN, _morph_kernel);
    morphologyEx(_filtered_frame, _filtered_frame, MORPH_CLOSE, _morph_kernel);
#endif

    // Contour Extraction
    std::vector<std::vector<Point> > contours;
    double contour_area, largest_contour = 0;
    int indx = -1;
    _interested_frame.copyTo(_contour_frame);
    findContours(_filtered_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    // find out the largest contour
    for (int i = static_cast<int>(contours.size()); --i>-1;)
    {
        contour_area = contourArea(contours[i]);
        if (contour_area > _detection_area && contour_area > largest_contour )
        {
            largest_contour = contour_area;
            indx = i;
        }
    }

    // Convexity Defects Detection applied on the largest contour region
    _convexity_frame = Mat(_contour_frame.rows, _contour_frame.cols, CV_8UC4);
    _convexity_frame.setTo(ColorWhite);
    if (indx > -1)
    {
        // Smooth contour
        approxPolyDP(contours[indx], contours[indx], 10, true);
        drawContours(_contour_frame, contours, indx, ColorRed, 2);
        drawContours(_convexity_frame, contours, indx, ColorGray, -1);
        drawContours(_convexity_frame, contours, indx, ColorBlack, 2);

        // Rectangle Region of the contour
        auto bounding_rect = boundingRect(contours[indx]);
        rectangle(_contour_frame, bounding_rect, ColorGreen, 2);

        // Convex Hull
        std::vector<std::vector<Point> > hulls(1);
        convexHull(contours[indx], hulls[0], false);
        drawContours(_contour_frame, hulls, -1, ColorBlue, 2);
        std::vector<std::vector<int> > hulls_indx(1);
        convexHull(contours[indx], hulls_indx[0], false);

        // Convexity Defects
        std::vector<Vec4i> defects;
        convexityDefects(contours[indx], hulls_indx[0], defects);
        float dist1, dist2, angle;

        // Detect Fingers
        // Find out the top most point as the pos to control cursor
        _finger_points.push_back(*std::min_element(contours[indx].begin(),
                                                   contours[indx].end(),
                                                   [](const Point& lhs, const Point& rhs) -> bool{
            return lhs.y < rhs.y;
        }));
        for (const auto & defect : defects)
        {
            // Filter defects
            dist1 = squaredEuclideanDistance(contours[indx][defect[0]], contours[indx][defect[2]]);
            if (dist1 > 100 && dist1 < 30000)   // keep those whose start and end points are too close or too far
            {
                dist2 = squaredEuclideanDistance(contours[indx][defect[1]], contours[indx][defect[2]]);
                if (dist2 > 100 && dist2 < 30000)
                {
                    angle = std::acos(
                                (
                                    (contours[indx][defect[0]].x - contours[indx][defect[2]].x)*(contours[indx][defect[1]].x - contours[indx][defect[2]].x)
                            +(contours[indx][defect[0]].x - contours[indx][defect[2]].x)*(contours[indx][defect[1]].x - contours[indx][defect[2]].y)
                            ) / std::sqrt(dist1 * dist2)
                            );
                    if (angle > 0.2618 && angle < 2.1415926535) // 15 degree to 180 degree
                    {
                        bool flag1 = true, flag2 = true;
                        for (const auto & p: _finger_points) // discard points who are too close
                        {
                            if (flag1 == true && squaredEuclideanDistance(contours[indx][defect[0]], p) < 900)
                                flag1 = false;

                            if (flag2 == true)
                            {
                                if (squaredEuclideanDistance(contours[indx][defect[1]], p) < 900)
                                    flag2 = false;
                            }
                            else if (flag1 == false)
                                break;

                        }

                        // One of the two points will be discarded if they are close.
                        // Then the left one point is considered the point of a finger
                        // if it is enough far from others who have been considered as points of fingers
                        if (flag1 == true )
                        {
                            if (flag2 == true)
                            {
                                if (squaredEuclideanDistance(contours[indx][defect[1]], contours[indx][defect[0]]) < 1000)
                                {
                                    if (contours[indx][defect[0]].y > contours[indx][defect[1]].y)
                                        _finger_points.push_back(contours[indx][defect[1]]);
                                    else
                                        _finger_points.push_back(contours[indx][defect[0]]);
                                }
                                else
                                {
                                    _finger_points.push_back(contours[indx][defect[0]]);
                                    _finger_points.push_back(contours[indx][defect[1]]);
                                }
                            }
                            else
                                _finger_points.push_back(contours[indx][defect[0]]);
                        }
                        else if (flag2 == true)
                            _finger_points.push_back(contours[indx][defect[1]]);

                    }
                }
            }
        }

        // Estimate Hand Center via gravity
        auto mom = moments(contours[indx], true);
        Point center(mom.m10/mom.m00, mom.m01/mom.m00);
        circle(_convexity_frame, center, 5, ColorGreen, -1);

        // top of fingers should be above the center
        if (_finger_points[0].y > center.y)
        {
            _finger_points.clear();
        }
        else
        {
            auto it_end = _finger_points.end();
            _finger_points.erase(std::remove_if(_finger_points.begin(), it_end,
                                                [center](const Point & p)->bool
            {
                return p.y > center.y;
            }), it_end);
            // Draw line between finger top and hand center
            for (const auto & fp : _finger_points)
            {
                circle(_convexity_frame, fp, 10, ColorRed, -1);
                line(_convexity_frame, center, fp, ColorBlue, 2);
            }
        }

    }

    rectangle(_original_frame, _ROI, ColorGreen, 2);
}

bool HandDetector::_track_finger()
{
    if (_finger_points.empty())
    {
        if (!_losing_tracking->isActive())
            _losing_tracking->start(DEFAULT_LOSE_TRACKING_INTERVAL);
        return false;
    }

    // Find out the top most point as the currently tracked point
    int indx = 0;
    for (size_t i = _finger_points.size(); --i>0;)
    {
        if (_finger_points[i].y < _finger_points[indx].y)
            indx = i;
    }

    bool flag = false;
    if (_is_tracking == true)
    {   // if still we have not lost tracking,
        // we check if the currently tracked point is too far away from the previously tracked point
        if (_finger_points[indx].x > _tracked_point_pos.x - DEFAULT_MAXMIUM_MOVEMENT_OFFSET
                && _finger_points[indx].x < _tracked_point_pos.x + DEFAULT_MAXMIUM_MOVEMENT_OFFSET
                && _finger_points[indx].y > _tracked_point_pos.y - DEFAULT_MAXMIUM_MOVEMENT_OFFSET
                && _finger_points[indx].y < _tracked_point_pos.y + DEFAULT_MAXMIUM_MOVEMENT_OFFSET)
        {   // if the tracked point did not move too far away,
            // we think the currecntly and previously tracked points are the same point
            flag = true;
        }
        else
        {   // we try to rectify the currently tracked point
            // by checking if any tracked point is not too far away from the previously tracked point
            for (size_t i = _finger_points.size(); --i>0;)
            {
                if (_finger_points[indx].x > _tracked_point_pos.x - DEFAULT_MAXMIUM_MOVEMENT_OFFSET
                        && _finger_points[indx].x < _tracked_point_pos.x + DEFAULT_MAXMIUM_MOVEMENT_OFFSET
                        && _finger_points[indx].y > _tracked_point_pos.y - DEFAULT_MAXMIUM_MOVEMENT_OFFSET
                        && _finger_points[indx].y < _tracked_point_pos.y + DEFAULT_MAXMIUM_MOVEMENT_OFFSET)
                {   // find a suitable point, take this point as the currently tracked point
                    indx = i;
                    flag = true;
                    break;
                }
            }
        }
    }
    if (flag == false)
    {  // if we have lost tracking or no suitable tracked point found,
        // then we directly update the tracked point to the current point
        _tracked_point_pos.x = _finger_points[indx].x;
        _tracked_point_pos.y = _finger_points[indx].y;
    }
    else
    {  // if it seems that we still keep tracking
        // we update the tracked point's pos
        // by the average of the currently and previously tracked points' pos
        _tracked_point_pos.x = static_cast<float>(_finger_points[indx].x + _tracked_point_pos.x)/2;
        _tracked_point_pos.y = static_cast<float>(_finger_points[indx].y + _tracked_point_pos.y)/2;
    }

    // Tracked something
    _losing_tracking->stop();
    circle(_convexity_frame, _tracked_point_pos, 12, ColorBlue, 2);

    return true;
}

const Point & HandDetector::getTrackedPoint()
{
    return _tracked_point_pos;
}

const std::vector<Point> & HandDetector::getFingerPoints()
{
    return _finger_points;
}

const Mat& HandDetector::getOriginalFrame()
{
    return _original_frame;
}

const Mat& HandDetector::getInterestedFrame()
{
    return _interested_frame;
}

const Mat& HandDetector::getFilteredFrame()
{
    return _filtered_frame;
}

const Mat& HandDetector::getContourFrame()
{
    return _contour_frame;
}

const Mat& HandDetector::getConvexityFrame()
{
    return _convexity_frame;
}

void HandDetector::setROI(const int & start_x, const int & start_y, const int & end_x, const int & end_y)
{
    _ROI = Rect(start_x, start_y, end_x-start_x, end_y-start_y);
}

void HandDetector::setROI(const Rect & roi)
{
    _ROI = roi;
}

void HandDetector::setSkinColorLowerBound(const int & H, const int & S, const int & V)
{
    _skin_color_lower_bound = Scalar(H, S, V);
}

void HandDetector::setSkinColorUpperBound(const int & H, const int & S, const int & V)
{
    _skin_color_upper_bound = Scalar(H, S, V);
}

void HandDetector::setDetectionArea(const int & area)
{
    _detection_area = area;
}

inline double HandDetector::squaredEuclideanDistance(Point p1, Point p2)
{
    return (p1.x - p2.x)*(p1.x-p2.x) + (p1.y - p2.y)*(p1.y-p2.y);
}
