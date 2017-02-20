#include "handanalysis.h"

void HandAnalysis::analyze(const cv::Mat & input_frame)
{
    original_frame = input_frame;

    //    _skinDectetor(original_frame); // Update ycrcbFrame

}

bool HandAnalysis::isTracking()
{
    return is_tracking;
}


void HandAnalysis::detect(const cv::Mat & input_frame,
                          const cv::Rect & roi,
                          const cv::Scalar & color_lower_bound,
                          const cv::Scalar & color_upper_bound,
                          const bool & erode,
                          const bool & dilate,
                          const bool & median_blur)
{
    cv::flip(input_frame, original_frame, 1);

//    original_frame(roi).copyTo(contour_frame);
//    contour_frame.copyTo(convexhull_frame);
    contour_frame = original_frame(roi);
    convexhull_frame = contour_frame;
    cv::cvtColor(contour_frame, contour_frame, cv::COLOR_RGB2HSV);
    cv::inRange(contour_frame, color_lower_bound, color_upper_bound, contour_frame);

    if (erode)
        cv::erode(contour_frame, contour_frame, cv::Mat());
    if (dilate)
        cv::dilate(contour_frame, contour_frame, cv::Mat());
    if (median_blur)
        cv::medianBlur(contour_frame, contour_frame, 7);

    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(contour_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    cv::cvtColor(contour_frame, contour_frame, cv::COLOR_GRAY2RGB);

    for (size_t i = -1; ++i < contours.size();)
    {
        if (cv::contourArea(contours[i]) > 5000)
        {
            // Draw contour on the contour frame
            cv::drawContours(contour_frame, contours, i, cv::Scalar(0, 0, 255, 255));

            cv::Mat c_contour(contours[i]);

            // Find and Draw Convex Hull
            std::vector<std::vector<cv::Point> > hulls(1);
            cv::convexHull(c_contour, hulls[0], false);
            cv::drawContours(convexhull_frame, hulls, -1, cv::Scalar(0, 255, 0, 255), 2);

            if (hulls[0].size()>0)
            {
                // Draw Minimum Rotated Rectangle
                cv::RotatedRect rect = cv::minAreaRect(c_contour);
                cv::Point2f rect_points[4];
                rect.points(rect_points);
                for (int j = -1; ++j < 4;)
                    cv::line(convexhull_frame, rect_points[j], rect_points[(j+1)%4], cv::Scalar(255, 0, 0, 255));

                // Find Convexity Defects
                std::vector<std::vector<int> > hulls_indx(1);
                cv::convexHull(c_contour, hulls_indx[0], false);
                std::vector<cv::Vec4i> defects;
                cv::convexityDefects(contours[i], hulls_indx[0], defects);

                // Estiamte the Center of Hand
                int total_defects = defects.size();
                if(total_defects > 2)
                {
                    cv::Point hand_center;
                    std::vector<cv::Point> hand_points;
                    for (int j = -1; ++j < total_defects;)
                    {
                        hand_center.x += contours[i][defects[j][0]].x + contours[i][defects[j][1]].x + contours[i][defects[j][2]].x;
                        hand_center.y += contours[i][defects[j][0]].y + contours[i][defects[j][1]].y + contours[i][defects[j][2]].y;
                    }
                    hand_center.x /= total_defects*3;
                    hand_center.y /= total_defects*3;

                    cv::circle(convexhull_frame, hand_center, 5, cv::Scalar(0, 255, 0, 255));

                    std::vector<std::pair<int, double> > dist;
                    for (int j = -1; ++j < total_defects;)
                    {
                        for (int k = -1; ++k < 3;)
                        {
                            dist.push_back(std::make_pair(3*j+k,
                                                          (contours[i][defects[j][k]].x - hand_center.x)*(contours[i][defects[j][k]].x - hand_center.x)
                                    + (contours[i][defects[j][k]].y - hand_center.y)*(contours[i][defects[j][k]].y - hand_center.y)
                                    ));
                        }

                    }

                    std::sort(dist.begin(), dist.end(),
                              [](const std::pair<int, double> & a, const std::pair<int, double> & b) -> bool
                    {
                        return a.second > b.second;
                    });

                    cv::Point p1, p2, p3;
                    double mag_2, cof_23, cof_12, det, center_x, center_y, radius;
                    for (int j = dist.size(); --j > 1;)
                    {
                        p1 = contours[i][defects[(dist[j].first-dist[j].first%3)/3][dist[j].first%3]];
                        p2 = contours[i][defects[(dist[j-1].first-dist[j-1].first%3)/3][dist[j-1].first%3]];
                        p3 = contours[i][defects[(dist[j-2].first-dist[j-2].first%3)/3][dist[j-2].first%3]];

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
                    if (radius > 0)
                    {
                        _hand_centers.push_back(std::make_pair(cv::Point(center_x, center_y), radius));
                        if (_hand_centers.size()>10)
                            _hand_centers.erase(_hand_centers.begin());
                        center_x = 0;
                        center_y = 0;
                        for (int j = _hand_centers.size(); --j>-1;)
                        {
                            center_x += _hand_centers[j].first.x;
                            center_y += _hand_centers[j].first.y;
                            radius += _hand_centers[j].second;
                        }
                        center_x /= _hand_centers.size();
                        center_y /= _hand_centers.size();

                        radius /= _hand_centers.size();

                        cv::circle(convexhull_frame,
                                   cv::Point(std::move(center_x), std::move(center_y)),
                                   3,
                                   cv::Scalar(255, 0, 0, 255)
                                   );
                        cv::circle(convexhull_frame,
                                   cv::Point(std::move(center_x), std::move(center_y)),
                                   std::sqrt(std::move(radius)),
                                   cv::Scalar(255, 0, 0, 255)
                                   );

                        _num_of_fingers = 0;
                        double dist_1, dist_2, len_1, len_2;
                        for (int j = -1; ++j < total_defects;)
                        {

                            dist_1 = (contours[i][defects[j][0]].x-center_x)*(contours[i][defects[j][0]].x-center_x)
                                    +(contours[i][defects[j][0]].y-center_y)*(contours[i][defects[j][0]].y-center_y);
                            dist_2 = (contours[i][defects[j][2]].x-center_x)*(contours[i][defects[j][2]].x-center_x)
                                    +(contours[i][defects[j][2]].y-center_y)*(contours[i][defects[j][2]].y-center_y);
                            len_1 = defects[j][3]/256.0;
                            len_1 *= len_1;
                            len_2 = (contours[i][defects[j][2]].x-contours[i][defects[j][0]].x)*(contours[i][defects[j][2]].x-contours[i][defects[j][0]].x)
                                    +(contours[i][defects[j][2]].y-contours[i][defects[j][0]].y)*(contours[i][defects[j][2]].y-contours[i][defects[j][0]].y);

                            if (len_1 <= 9*radius && dist_1 >= 0.16*radius &&
                                    len_1 >= 100 && len_2 >= 100 &&
                                    (len_1/len_2 >= 0.64 || len_2/len_1 >= 0.64) &&
                                    (dist_1/dist_2 <= 0.64 || dist_2/dist_1 <= 0.64) &&
                                    ((dist_1 >= 0.01*radius && dist_1 <= 1.69*radius) || (dist_2 >= 0.01*radius && dist_2 <= 1.69*radius))
                                    )
                            {


                                cv::circle(convexhull_frame, contours[i][defects[j][1]], 4, cv::Scalar(0, 0, 255, 255), 1);
                                cv::circle(convexhull_frame, contours[i][defects[j][2]], 4, cv::Scalar(0, 0, 255, 255), 3);
                                cv::line(convexhull_frame, contours[i][defects[j][1]], contours[i][defects[j][2]], cv::Scalar(0, 255, 0, 255), 2);
                                _num_of_fingers++;
                            }

                        }
                    }

                }

            }
        }
        cv::rectangle(original_frame, roi, ColorGreen);
    }
}

int HandAnalysis::fingersFound()
{
    return _num_of_fingers;
}

cv::Mat & HandAnalysis::getOriginalFrame()
{
    return original_frame;
}

cv::Mat & HandAnalysis::getContourFrame()
{
    return contour_frame;
}

cv::Mat & HandAnalysis::getConvexHullFrame()
{
    return convexhull_frame;
}

//cv::Mat & HandAnalysis::getOriginalFrame()
//{
//    return originalFrame;
//}

//cv::Mat & HandAnalysis::getYCrCbFrame()
//{
//    return ycrcbFrame;
//}

//void HandAnalysis::_skinDectetor(const cv::Mat & inputFrame)
//{
//    cv::cvtColor(inputFrame, ycrcbFrame, cv::COLOR_RGB2HSV);
//    cv::inRange(ycrcbFrame, DefaultSkinColorLowerBound, DefaultSkinColorUpperBound, ycrcbFrame);
//}

