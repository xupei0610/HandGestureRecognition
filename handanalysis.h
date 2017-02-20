#ifndef HANDTRACKER_H
#define HANDTRACKER_H

#include <list>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>

class HandAnalysis
{
public:

    void analyze(const cv::Mat & input_frame);
    void detect(const cv::Mat & input_frame,
                const cv::Rect & roi,
                const cv::Scalar & color_lower_bound,
                const cv::Scalar & color_upper_bound,
                const bool & erode,
                const bool & dilate,
                const bool & median_blur);
    bool isTracking();
    int fingersFound();

    cv::Mat & getOriginalFrame();
    cv::Mat & getContourFrame();
    cv::Mat & getConvexHullFrame();


    const cv::Scalar ColorGreen = cv::Scalar(47,255,173,255);


    const static int LowerBoundH = 30;
    const static int LowerBoundS = 0;
    const static int LowerBoundV = 100;
    const static int UpperBoundH = 180;
    const static int UpperBoundS = 255;
    const static int UpperBoundV = 255;

private:
    bool is_tracking = false;
    int _num_of_fingers;
    std::vector<std::pair<cv::Point, double> > _hand_centers;

    cv::Mat original_frame;
    cv::Mat contour_frame;
    cv::Mat convexhull_frame;
    cv::Mat convexity_frame;

    void _skinDectetor(const cv::Mat &);

};

#endif // HANDTRACKING_H
