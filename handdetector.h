#ifndef HANDDETECTION_H
#define HANDDETECTION_H

#include <vector>
#include <QObject>
#include <QTimer>
#include <opencv2/opencv.hpp>

using namespace cv;
#ifdef USE_GPU
using namespace cv::cuda;
#endif

#define DEFAULT_SKIN_COLOR_MIN_H 30
#define DEFAULT_SKIN_COLOR_MAX_H 180
#define DEFAULT_SKIN_COLOR_MIN_S 0
#define DEFAULT_SKIN_COLOR_MAX_S 255
#define DEFAULT_SKIN_COLOR_MIN_V 100
#define DEFAULT_SKIN_COLOR_MAX_V 255
#define DEFAULT_DETECTION_AREA 5000 // only detect contours whose area is bigger than this value
#define DEFAULT_MAXMIUM_MOVEMENT_OFFSET 15 // the maximum movement distance
                                           // if the tracked point between two frames
                                           // will be considered as the same point
#define DEFAULT_LOSE_TRACKING_INTERVAL 500 // ms
                                           // how long the detector think it lose tracking
class HandDetector : public QObject
{
    Q_OBJECT

public:

    HandDetector();
    ~HandDetector();

    // Try to detect hands in region of interesting
    bool detect(const Mat& input_frame);
    // Obtain the last tracked point
    const Point & getTrackedPoint();
    // Obtain the points of finger tops
    const std::vector<Point> & getFingerPoints();

    // Set region of interesting
    void setROI(const Rect &);

    // Set background for background substractor
    void setBackgroundImage(const Mat&);

    const Mat& getOriginalFrame();
    const Mat& getInterestedFrame();
    const Mat& getFilteredFrame();
    const Mat& getContourFrame();
    const Mat& getConvexityFrame();

    // BGR Color
    const Scalar ColorGreen = Scalar(47 ,255,173,255);
    const Scalar ColorBlue  = Scalar(255,  0,  0,255);
    const Scalar ColorRed   = Scalar(0  ,  0,255,255);
    const Scalar ColorGray  = Scalar(110,110,110,255);
    const Scalar ColorBlack = Scalar(0  ,  0,  0,255);
    const Scalar ColorWhite = Scalar(255,255,255,255);

    inline static double squaredEuclideanDistance(Point p1, Point p2);

 public slots:
    // set the tracking to be lost
    void lost_tracking();
    // Set region of interesting
    void setROI(const int & start_x, const int & start_y, const int & end_x, const int & end_y);
    // Set skin color detection range
    void setSkinColorLowerBound(const int & H, const int & S, const int & V);
    void setSkinColorUpperBound(const int & H, const int & S, const int & V);
    // Set Minimum Detection Area
    void setDetectionArea(const int &);

protected:
    Rect _ROI;              // region of interesting on the input frame

    Mat _morph_kernel = getStructuringElement(MORPH_ELLIPSE, Size(9,9));

#ifdef USE_GPU
    // filter implemented by gpu
    Ptr<cuda::Filter> _gaussian_filter;
    Ptr<cuda::Filter> _open_filter;
    Ptr<cuda::Filter> _close_filter;
#endif

    Mat _original_frame;     // original input frame
    Mat _interested_frame;   // interesting region of the original input frame
    Mat _filtered_frame;     // interesting region after skin color detection
    Mat _contour_frame;      // contour found at the interesting region
    Mat _convexity_frame;    // skeleton forming by connecting fingers and the center of hand

    bool _is_tracking = false; // keeping tracking or not
    QTimer * _losing_tracking = nullptr;
    std::vector<Point> _finger_points; // finger points detected
    Point _tracked_point_pos; // the point tracked as the cursor

    void _detect_hand();  // detect hand region and update the point of giners
    bool _track_finger(); // track fingers and update the tracked point's pos

    Ptr<BackgroundSubtractor> _background_subtractor;
    Mat _bg;
    bool _has_set_bg = false;

private:
    Scalar _skin_color_lower_bound = Scalar(DEFAULT_SKIN_COLOR_MIN_H,
                                                    DEFAULT_SKIN_COLOR_MIN_S,
                                                    DEFAULT_SKIN_COLOR_MIN_V);
    Scalar _skin_color_upper_bound = Scalar(DEFAULT_SKIN_COLOR_MAX_H,
                                                    DEFAULT_SKIN_COLOR_MAX_S,
                                                    DEFAULT_SKIN_COLOR_MAX_V);
    int _detection_area = DEFAULT_DETECTION_AREA;

};

#endif // HANDDETECTION_H
