#ifndef TRAJECTORY_TRACKING_H
#define TRAJECTORY_TRACKING_H

#include <QThread>
#include <QObject>
#include <QDebug>
#include <QTime>
#include <QTimer>

#include <stdlib.h>
#include <math.h>

#include <omp.h>
#include <opencv.hpp>
#ifndef NO_OCL
#include <core/ocl.hpp>
#endif
#include "tag_recognition.h"
#include "math_function.h"
#include "object_tracking.h"

#define imgSizeX 1200
#define imgSizeY 1600

#define VIDEOTIME (30*60)
#define HOUGH_CIRCLE_RESIZE 2

class trajectory_tracking : public QThread
{
    Q_OBJECT
public:
    explicit trajectory_tracking(QObject *parent = 0);
    ~trajectory_tracking();

    void setImageShiftOriginPoint(std::vector<cv::Point> originPoint);

    cv::Mat imageShift(std::vector<cv::Mat> stitchFrame,std::vector<cv::Point> originPoint);

    cv::Mat imageShiftLoaded(std::vector<cv::Mat> stitchFrame);

#ifndef NO_OCL
    void initOCL();
#endif
    void setVideoName(std::vector<std::string> videoName);

    void setHoughCircleParameters(const int &dp,const int &minDist,const int &para_1,const int &para_2,const int &minRadius,const int &maxRadius);

    void setShowImage(const bool &status);

    void setSVMModelFileName(const std::string &fileName);

    void setPCAModelFileName(const std::string &fileName);

    void setManualStitchingFileName(const std::string &fileName);

    void setTagBinaryThreshold(const double &value);

    void setPCAandHOG(const bool &PCAS,const bool &HOGS);

    void stopStitch();

signals:

    void finish();

    void sendFPS(const double &fps);

    void sendImage(const cv::Mat &src);

    void sendSystemLog(const QString& msg);

    void sendProcessingProgress(const int &percentage);

public slots:

private slots:
    void receiveSystemLog(const QString &log);

private:
    tag_recognition *TR;

    object_tracking *OT;

    std::vector<cv::Point> originPoint;

    std::vector<std::string> videoName;

    std::vector<cv::Mat> frame;


    bool stopped;

    bool showImage = true;

    QTimer *saveClock;

    //Hough Circle Parameters
    int dp;

    int minDist;

    int para_1;

    int para_2;

    int minRadius;

    int maxRadius;

    //Contour Parameters

    int contourParam1;

    int contourParam2;


    std::string SVMModelFileName;

    std::string PCAModelFileName;


    void run();

    cv::Mat imageCutBlack(cv::Mat src);

    void circleResize(std::vector<cv::Vec3f> &circles);

};

#endif // TRAJECTORY_TRACKING_H
