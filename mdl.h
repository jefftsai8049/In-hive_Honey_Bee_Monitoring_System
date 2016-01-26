#ifndef MDL_H
#define MDL_H

#include <opencv.hpp>

#include <object_tracking.h>

class mdl
{
public:
    mdl(trackPro *data, const int maxLength);

    QVector<int> getCriticalPoint(){return criticalPoint;}

private:
    trackPro *trajectory;

    QVector<int> criticalPoint;

    double costNoPar(const int startIndex,const int currentIndex);
    double costPar(const int startIndex,const int currentIndex);

    double distancePerpendicular(const cv::Point pt1_s, const cv::Point pt1_e, const cv::Point pt2_s, const cv::Point pt2_e);
    double distanceAngle(const cv::Point pt1_s,const cv::Point pt1_e,const cv::Point pt2_s,const cv::Point pt2_e);
    double distanceEucliadian(const cv::Point pt1,const cv::Point pt2){return cv::norm(pt2-pt1);}
};

#endif // MDL_H
