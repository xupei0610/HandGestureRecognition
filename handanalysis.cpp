#include "handanalysis.h"

HandAnalysis::HandAnalysis()
{
    _tracking_timer = new QTimer();
    _tracking_timer->setSingleShot(true);
    connect(_tracking_timer, SIGNAL(timeout()), this, SLOT(loseTracking()));
}

HandAnalysis::~HandAnalysis()
{
    delete _tracking_timer;
}

void HandAnalysis::loseTracking()
{
    _hand_centers.clear();
}

void HandAnalysis::detect(const cv::Mat & input_frame)
{
//    if (_is_tracking == true)
//    {

//    }
//    else
//    {

//    }

    cv::flip(input_frame, original_frame, 1);
    interested_frame = original_frame(_roi);
    // Skin Color Filter
    _skinColorDetect();
    // Contour Extraction and then Gesture Analysis
    auto contour_ids = _contourExtract();
    for (auto & i : contour_ids.first)
    {
        if (i==contour_ids.second)
        {   // Send the analysis result for tracked hand
            auto hand = _gestureRecognize(_contours[i]);
            if (hand.second)
            {
//                emit handPos(hand.first.first, hand.first.second);
//                emit fingersFound(hand.second);
                emit handSignal(hand.first.first, hand.first.second, hand.second);
            }
        }
        else
            _gestureRecognize(_contours[i]);
    }
    emit detected();

    cv::rectangle(original_frame, _roi, ColorGreen);
}

void HandAnalysis::_skinColorDetect()
{
    filtered_frame = interested_frame;
        cv::cvtColor(filtered_frame, filtered_frame, cv::COLOR_RGB2HSV);
        cv::inRange(filtered_frame,
                    _skin_color_lower_bound,
                    _skin_color_upper_bound,
                    filtered_frame);
        if (_erode)
            cv::erode(filtered_frame, filtered_frame, cv::Mat());
        if (_dilate)
            cv::dilate(filtered_frame, filtered_frame, cv::Mat());
        if (_median_blur)
            cv::medianBlur(filtered_frame, filtered_frame, 7);
//    cv::cvtColor(filtered_frame, filtered_frame,cv::COLOR_RGB2GRAY);
//    cv::threshold(filtered_frame, filtered_frame, 0, 255, (CV_THRESH_BINARY+CV_THRESH_OTSU));

}

const std::pair<std::vector<int>, int> HandAnalysis::_contourExtract()
{
    std::vector<int> contour_ids;
    int tracked_indx = -1;

    filtered_frame.copyTo(convexity_frame);
    interested_frame.copyTo(contour_frame);
    cv::findContours(convexity_frame, _contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE); //CV_CHAIN_APPROX_NONE
    cv::cvtColor(convexity_frame, convexity_frame, cv::COLOR_GRAY2RGB);

    // Find out big contours
    int current_area, largest_area = 0;
    size_t total_contours = _contours.size();
    for (size_t i = -1; ++i < total_contours;)
    {
        current_area = cv::contourArea(_contours[i]);
        if (current_area > DEFAULT_DETECTION_AREA)
        {
            contour_ids.push_back(i);
            if (current_area > largest_area)
                tracked_indx = i;
        }
    }

    for (auto & indx: contour_ids)
    {
        //cv::approxPolyDP(contours[indx], contours[indx], contours[indx].size()*0.0025, true);

        // Draw contour
        cv::drawContours(convexity_frame, _contours, indx, ColorRed);
        cv::drawContours(contour_frame, _contours, indx, ColorRed);

        // Find and Draw Convex Hull
        //        std::vector<std::vector<cv::Point> > hulls(1);
        //        cv::convexHull(_contours[indx], hulls[0], false);
        //        cv::drawContours(convexity_frame, hulls, -1, ColorGreen, 2);
        //        cv::drawContours(contour_frame, hulls, -1, ColorGreen, 2);

        //        if (hulls[0].size()>0)
        //        {
        //            // Draw Minimum Rotated Rectangle
        //            cv::RotatedRect rect = cv::minAreaRect(_contours[indx]);
        //            cv::Point2f rect_points[4];
        //            rect.points(rect_points);
        //            for (int j = -1; ++j < 4;)
        //            {
        //                cv::line(convexity_frame, rect_points[j], rect_points[(j+1)%4], cv::Scalar(255, 0, 0, 255));
        //                cv::line(contour_frame, rect_points[j], rect_points[(j+1)%4], cv::Scalar(255, 0, 0, 255));
        //            }
        //        }
    }
    return std::make_pair(contour_ids, tracked_indx);
}

std::pair<std::pair<int, int>, int> HandAnalysis::_gestureRecognize(const HandAnalysis::Contour & contour)
{
    int num_of_fingers = 0;
    std::pair<std::pair<int, int>, int> result;
    if (contour.empty())
        return result;

    // Find out Convexity Defects
    std::vector<std::vector<int> > hulls_indx(1);
    cv::convexHull(contour, hulls_indx[0], false);
    std::vector<cv::Vec4i> defects;
    cv::convexityDefects(contour, hulls_indx[0], defects);

    // Too few defects
    int total_defects = defects.size();
    if(total_defects < 3)
        return result;

    // Estiamte the Center of Hand

    // Step 0, draw the current center of defects
        cv::Point hand_center;
        for (int j = -1; ++j < total_defects;)
        {
            hand_center.x += contour[defects[j][0]].x + contour[defects[j][1]].x + contour[defects[j][2]].x;
            hand_center.y += contour[defects[j][0]].y + contour[defects[j][1]].y + contour[defects[j][2]].y;
        }
        hand_center.x /= total_defects*3;
        hand_center.y /= total_defects*3;
        cv::circle(convexity_frame, hand_center, 5, ColorGreen);
        cv::circle(contour_frame, hand_center, 5, ColorGreen);

        // First, sort defect points accoording to the distance between each of them to the center of hand
        std::vector<std::pair<int, float> > dist;
        for (int j = -1; ++j < total_defects;)
        {
            for (int k = -1; ++k < 3;)
            {
                dist.push_back(std::make_pair(3*j+k,
                                              (contour[defects[j][k]].x - hand_center.x)*(contour[defects[j][k]].x - hand_center.x)
                        + (contour[defects[j][k]].y - hand_center.y)*(contour[defects[j][k]].y - hand_center.y)
                        ));
            }

        }
        std::sort(dist.begin(), dist.end(),
                  [](const std::pair<int, float> & a, const std::pair<int, float> & b) -> bool
        {
            return a.second > b.second;
        });

        // Then, use three points close to the center of hand to construct and circle region as the region of palm
        cv::Point p1, p2, p3;
        float mag_2, cof_23, cof_12, det, center_x, center_y, radius;
        for (int j = dist.size(); --j > 1;)
        {
            p1 = contour[defects[(dist[j].first-dist[j].first%3)/3][dist[j].first%3]];
            p2 = contour[defects[(dist[j-1].first-dist[j-1].first%3)/3][dist[j-1].first%3]];
            p3 = contour[defects[(dist[j-2].first-dist[j-2].first%3)/3][dist[j-2].first%3]];

            det = (p1.x-p2.x)*(p2.y-p3.y) - (p2.x-p3.x)*(p1.y-p2.y);
            if (std::abs(det) > 0.0000001)
            {
                mag_2 = p2.x*p2.x + p2.y*p2.y;
                cof_23 = p1.x*p1.x+p1.y*p1.y-mag_2;
                cof_12 = mag_2-p3.x*p3.x-p3.y*p3.y;
                center_x = (cof_23*(p2.y-p3.y)-cof_12*(p1.y-p2.y)) /det /2;
                center_y = (cof_12*(p1.x-p2.x)-cof_23*(p2.x-p3.x)) /det /2;
                radius = (p2.x-center_x)*(p2.x-center_x) + (p2.y-center_y)*(p2.y-center_y);
                if (radius > 0)
                    break;
            }
        }
        // Fail to construct a circle region
        if (radius <= 0)
            return result;

        // Finally, use the average of the last ten positions of the palm detected as the current position of hand.
        _hand_centers.push_back(std::make_pair(cv::Point(center_x, center_y), radius));
        if (_hand_centers.size()<11)
            return result;
        center_x = 0;
        center_y = 0;
        radius = 0;
        for (int j = _hand_centers.size(); --j>-1;)
        {
            center_x += _hand_centers[j].first.x;
            center_y += _hand_centers[j].first.y;
            radius += _hand_centers[j].second;
        }
        center_x /= _hand_centers.size();
        center_y /= _hand_centers.size();
        radius /= _hand_centers.size();
        // Draw the confident region of the palm and its center
        cv::circle(convexity_frame,
                   cv::Point(center_x, center_y),
                   3,
                   cv::Scalar(255, 0, 0, 255)
                   );
        cv::circle(convexity_frame,
                   cv::Point(center_x, center_y),
                   std::sqrt(radius),
                   cv::Scalar(255, 0, 0, 255)
                   );
        cv::circle(contour_frame,
                   cv::Point(std::move(center_x), std::move(center_y)),
                   3,
                   cv::Scalar(255, 0, 0, 255)
                   );
        cv::circle(contour_frame,
                   cv::Point(std::move(center_x), std::move(center_y)),
                   std::sqrt(std::move(radius)),
                   cv::Scalar(255, 0, 0, 255)
                   );
        _hand_centers.erase(_hand_centers.begin());

        // Setup a timer for tracking,
        // if it takes too much time before detecting hand again
        // clear the previously stored positions of hand
        _tracking_timer->start(DEFAULT_KEEP_TRACKING_DURATION);

    //    // Count Number of Fingers
    //    float dist_1, dist_2, len_1, len_2;
    //    for (int j = -1; ++j < total_defects;)
    //    {
    //        dist_1 = (contour[defects[j][0]].x-center_x)*(contour[defects[j][0]].x-center_x)
    //                +(contour[defects[j][0]].y-center_y)*(contour[defects[j][0]].y-center_y);
    //        dist_2 = (contour[defects[j][2]].x-center_x)*(contour[defects[j][2]].x-center_x)
    //                +(contour[defects[j][2]].y-center_y)*(contour[defects[j][2]].y-center_y);
    //        len_1 = defects[j][3]/256.0;
    //        len_1 *= len_1;
    //        len_2 = (contour[defects[j][2]].x-contour[defects[j][0]].x)*(contour[defects[j][2]].x-contour[defects[j][0]].x)
    //                +(contour[defects[j][2]].y-contour[defects[j][0]].y)*(contour[defects[j][2]].y-contour[defects[j][0]].y);

    //        if (len_1 <= 9*radius && dist_1 >= 0.16*radius &&
    //                len_1 >= 100 && len_2 >= 100 &&
    //                (len_1/len_2 >= 0.64 || len_2/len_1 >= 0.64) &&
    //                (dist_1/dist_2 <= 0.64 || dist_2/dist_1 <= 0.64) &&
    //                ((dist_1 >= 0.01*radius && dist_1 <= 1.69*radius) || (dist_2 >= 0.01*radius && dist_2 <= 1.69*radius))
    //                )
    //        {
    //            num_of_fingers++;

    //            cv::circle(convexity_frame, contour[defects[j][1]], 4, ColorRed, 1);
    //            cv::circle(convexity_frame, contour[defects[j][2]], 4, ColorRed, 3);
    //            cv::line(convexity_frame, contour[defects[j][1]], contour[defects[j][2]], ColorGreen, 2);

    //            cv::circle(contour_frame, contour[defects[j][1]], 4, ColorRed, 1);
    //            cv::circle(contour_frame, contour[defects[j][2]], 4, ColorRed, 3);
    //            cv::line(contour_frame, contour[defects[j][1]], contour[defects[j][2]], ColorGreen, 2);
    //        }
    //    }

    auto blob_area = cv::minAreaRect(contour);
    cv::circle(contour_frame, blob_area.center, 5, ColorRed);
    for (int j = -1; ++j < total_defects;)
    {

        float a = (contour[defects[j][1]].x - contour[defects[j][0]].x) * (contour[defects[j][1]].x - contour[defects[j][0]].x) +
                (contour[defects[j][1]].y - contour[defects[j][0]].y) * (contour[defects[j][1]].y - contour[defects[j][0]].y);                         ;

        float b = (contour[defects[j][2]].x - contour[defects[j][0]].x) * (contour[defects[j][2]].x - contour[defects[j][0]].x) +
                (contour[defects[j][2]].y - contour[defects[j][0]].y) * (contour[defects[j][2]].y - contour[defects[j][0]].y);                         ;

        float c = (contour[defects[j][1]].x - contour[defects[j][2]].x) * (contour[defects[j][1]].x - contour[defects[j][2]].x) +
                (contour[defects[j][1]].y - contour[defects[j][2]].y) * (contour[defects[j][1]].y - contour[defects[j][2]].y);                         ;

        if (std::acos((b + c - a)/(2*b*c))*57 <= 90 )
        {

            if ((contour[defects[j][0]].y < blob_area.center.y || contour[defects[j][2]].y < blob_area.center.y) &&
                    contour[defects[j][0]].y < contour[defects[j][2]].y)
            {

                float dist_1 = contour[defects[j][0]].x-contour[defects[j][2]].x;
                dist_1 *= dist_1;
                float dist_2 = contour[defects[j][0]].y-contour[defects[j][2]].y;
                dist_2 *= dist_2;

                if (dist_1 + dist_2 > float(blob_area.size.height)*blob_area.size.height/36)
                {
                    num_of_fingers++;
                    cv::circle(convexity_frame, contour[defects[j][0]], 4, ColorRed, 1);
                    cv::circle(convexity_frame, contour[defects[j][2]], 4, ColorRed, 3);
                    cv::line(convexity_frame, contour[defects[j][0]], contour[defects[j][2]], ColorGreen, 2);

                    cv::circle(contour_frame, contour[defects[j][0]], 4, ColorRed, 1);
                    cv::circle(contour_frame, contour[defects[j][2]], 4, ColorRed, 3);
                    cv::line(contour_frame, contour[defects[j][0]], contour[defects[j][2]], ColorGreen, 2);
                }
            }
        }
    }
    cv::Point ext_top   = *std::min_element(contour.begin(), contour.end(),
                          [](const cv::Point& lhs, const cv::Point& rhs) {
                              return lhs.y < rhs.y;
                      });
    cv::circle(convexity_frame, ext_top, 2, ColorBlue, 2);

//    result.first = std::make_pair(blob_area.center.x, blob_area.center.y);
//    result.first = std::make_pair(center_x, center_y);
    result.first = std::make_pair(ext_top.x, ext_top.y);
    result.second = num_of_fingers;
    return result;
}

const cv::Mat & HandAnalysis::getOriginalFrame()
{
    return original_frame;
}

const cv::Mat & HandAnalysis::getInterestedFrame()
{
    return interested_frame;
}

const cv::Mat & HandAnalysis::getFilteredFrame()
{
    return filtered_frame;
}

const cv::Mat & HandAnalysis::getContourFrame()
{
    return contour_frame;
}

const cv::Mat & HandAnalysis::getConvexityFrame()
{
    return convexity_frame;
}

void HandAnalysis::setROI(const int & start_x, const int & start_y, const int & end_x, const int & end_y)
{
    _roi = cv::Rect(start_x, start_y, end_x-start_x, end_y-start_y);
    _tracking_timer->stop();
    loseTracking();
}

void HandAnalysis::setROI(const cv::Rect & ROI)
{
    _roi = ROI;
    _tracking_timer->stop();
    loseTracking();
}

void HandAnalysis::setSkinColorLowerBound(const int & H, const int & S, const int & V)
{
    _skin_color_lower_bound = cv::Scalar(H, S, V);
}

void HandAnalysis::setSkinColorUpperBound(const int & H, const int & S, const int & V)
{
    _skin_color_upper_bound = cv::Scalar(H, S, V);
}

void HandAnalysis::setErode(const bool & val)
{
    _erode = val;
}

void HandAnalysis::setDilate(const bool & val)
{
    _dilate = val;
}

void HandAnalysis::setMedianBlur(const bool & val)
{
    _median_blur = val;
}

