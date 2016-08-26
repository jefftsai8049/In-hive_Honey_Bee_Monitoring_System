#ifndef OBJECT_TRACKING_H
#define OBJECT_TRACKING_H

#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QVector>
#include <QFileInfo>
#include <QDataStream>
#include <QDir>

#include <opencv.hpp>
#include <stdlib.h>

#include "math_function.h"

#include "parameters.h"

enum pattern{
    NO_MOVE,
    LOITERING,
    FORWARD_MOVE,
    RIGHT_MOVE,
    LEFT_MOVE,
    WAGGLE,
    INTERACTION,
    FORAGING,
    OTHER,
    MOVING
};

struct objectTrackingParameters
{
    int thresholdNoMove;
    int thresholdLoitering;
    int thresholdDirection;
    int segmentSize;
};

//before preprocessing
struct track
{
    //raw data format
    std::vector<char> w1;
    std::vector<char> w2;
    std::vector<cv::Point> position;
    std::vector<QDateTime> time;

    //functions (raw data)
    int size();
    cv::Point lastPosition();
    cv::Point futurePosition();
    double howClose(const cv::Vec3f& circle);

};

//after preprocessing
struct trackPro
{
    QString ID;
    QDateTime startTime;
    QDateTime endTime;
    int size;
    QVector<cv::Point> position;

    //for fix length pattern classifier
    QVector<int> pattern;
    QVector<int> pattern3D;
    QVector<double> getPatternCount();

    //MDL
    QVector<int> criticalPointIndex;
    cv::Mat getCriticalPointPlot(const cv::Mat &src);

    //for behavior length pattern classifier
    QVector<int> trajectoryPattern;
    QVector<double> distanceP2P;
    QVector<double> distanceP2P_AVG;
    QVector<double> getMovingDistanceP2P();
    QVector<double> getPatternCount_behavior();
    cv::Rect getROI();

    //draw trajecory plot
    cv::Mat getTrajectoryPlot(const cv::Mat &src);
    cv::Mat getTrajectoryPlotPart(const cv::Mat &src, int from, int end);


    double getTrajectoryMovingDistance();
    double getTrajectoryMovingVelocity();
    double getStaticTime();
    double getLoiteringTime();
    double getMovingTime();
    double getDetectedTime();

};


class object_tracking : public QObject
{
    Q_OBJECT
public:
    explicit object_tracking(QObject *parent = 0,const QDateTime fileTime = QDateTime());
    ~object_tracking();


    void compute(const QDateTime &time, const std::vector<cv::Vec3f>& circles, const std::vector<std::string>& w1, const std::vector<std::string>& w2);

    void setImageRange(const cv::Size &imageRange);

    void lastPath(std::vector< std::vector<cv::Point> >& path);

    void drawPath(cv::Mat &src);

    void savePath();

    void saveAllPath();


    //for trackPro

    QString voting(track path);

    void trajectoryFilter(QVector<cv::Point> &path);

    QVector<cv::Point> interpolation(const std::vector<cv::Point> &position, const std::vector<QDateTime> &time);

    void saveTrackPro(const QVector<trackPro> &path,const QString &fileName);

    void loadDataTrack(const QStringList &fileNames, std::vector<track> *path);

    void rawDataPreprocessing(const std::vector<track> *path, QVector<trackPro> *TPVector);

    void loadDataTrackPro(const QString &fileName, QVector<trackPro> *path);

    //for tracjectory classify

    void tracjectoryWhiteList(QVector<trackPro> &data,const QStringList &whiteList);

    void trajectoryClassify(QVector<trackPro> &path,const objectTrackingParameters params);

    void trajectoryClassify3D(QVector<trackPro> &path,const objectTrackingParameters params);

    QString tracjectoryName(const char &pattern);

    void setPathSegmentSize(const int &size);

//    void calculatePatternCount(const trackPro &data, QVector<int> &count);

    //for debug

    void drawPathPattern(const QVector<cv::Point> &path);

signals:

    void sendSystemLog(const QString &log);

    void sendLoadRawDataFinish();

    void sendProgress(const int &progress);

public slots:



private:

    QDateTime nowTime;

    QDateTime startTime;

    cv::Size range;

    std::vector<track> path;

    QDir outDataDir = QDir("out/processing_data");

    int segmentSize;

    int minTimeStep(const std::vector<QDateTime> &time);

    cv::Point mean(const QVector<cv::Point> &motion);

    QVector<float> variance(const QVector<cv::Point> &motion);

    int direction(const QVector<cv::Point> &motion,const objectTrackingParameters params);


};
#endif // OBJECT_TRACKING_H
