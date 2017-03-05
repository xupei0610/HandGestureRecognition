#ifndef HANDTRACKER_H
#define HANDTRACKER_H

#include <vector>

#include <QObject>
#include <QTimer>
#include <opencv2/opencv.hpp>

#define DEFAULT_SKIN_COLOR_MIN_H 30
#define DEFAULT_SKIN_COLOR_MAX_H 180
#define DEFAULT_SKIN_COLOR_MIN_S 0
#define DEFAULT_SKIN_COLOR_MAX_S 255
#define DEFAULT_SKIN_COLOR_MIN_V 100
#define DEFAULT_SKIN_COLOR_MAX_V 255
#define DEFAULT_CONDUCT_ERODE true
#define DEFAULT_CONDUCT_DILATE true
#define DEFAULT_CONDUCT_MEDIAN_BLUR true
#define DEFAULT_KEEP_TRACKING_DURATION 2000 // milliseconds
#define DEFAULT_DETECTION_AREA 5000 // only detect contours whose area is bigger than this value

class HandAnalysis : public QObject
{
    Q_OBJECT

signals:
    void detected();
    void fingersFound(int);
    void handPos(int x, int y);
    void handSignal(int x, int y, int num_of_fingers);

public:
    typedef std::vector<cv::Point> Contour;

    HandAnalysis();
    ~HandAnalysis();

    // Try to detect hands in region of interesting
    void detect(const cv::Mat & input_frame);
    // Set region of interesting
    void setROI(const cv::Rect &);

    const cv::Mat & getOriginalFrame();
    const cv::Mat & getInterestedFrame();
    const cv::Mat & getFilteredFrame();
    const cv::Mat & getContourFrame();
    const cv::Mat & getConvexityFrame();

    // BGR Color
    const cv::Scalar ColorGreen = cv::Scalar(47,255,173,255);
    const cv::Scalar ColorBlue  = cv::Scalar(255, 0,  0,255);
    const cv::Scalar ColorRed   = cv::Scalar(0  , 0,255,255);

 public slots:
    // Set region of interesting
    void setROI(const int & start_x, const int & start_y, const int & end_x, const int & end_y);
    // Set skin color detection range
    void setSkinColorLowerBound(const int & H, const int & S, const int & V);
    void setSkinColorUpperBound(const int & H, const int & S, const int & V);
    // Perform erode before extraction contour
    void setErode(const bool &);
    // Perform dilate before extraction contour
    void setDilate(const bool &);
    // Perform median blur before extraction contour
    void setMedianBlur(const bool &);
    // Clear tracking data if lose tracking
    void loseTracking();

private:

    std::vector<std::pair<cv::Point, double> > _hand_centers;
    QTimer * _tracking_timer;

    cv::Rect _roi;
    cv::Scalar _skin_color_lower_bound = cv::Scalar(DEFAULT_SKIN_COLOR_MIN_H,
                                                    DEFAULT_SKIN_COLOR_MIN_S,
                                                    DEFAULT_SKIN_COLOR_MIN_V);
    cv::Scalar _skin_color_upper_bound = cv::Scalar(DEFAULT_SKIN_COLOR_MAX_H,
                                                    DEFAULT_SKIN_COLOR_MAX_S,
                                                    DEFAULT_SKIN_COLOR_MAX_V);
    bool _erode       = DEFAULT_CONDUCT_ERODE;
    bool _dilate      = DEFAULT_CONDUCT_DILATE;
    bool _median_blur = DEFAULT_CONDUCT_MEDIAN_BLUR;

    cv::Mat original_frame;
    cv::Mat interested_frame;
    cv::Mat contour_frame;
    cv::Mat filtered_frame;
    cv::Mat convexity_frame;

    bool _is_tracking = false;

    std::vector<Contour> _contours;
    void _skinColorDetect();
    const std::pair<std::vector<int>, int> _contourExtract();
    std::pair<std::pair<int,int>, int> _gestureRecognize(const Contour & contour);

};

#endif // HANDTRACKING_H
