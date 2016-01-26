#include "mdl.h"

mdl::mdl(trackPro *data, const int maxLength)
{
    trajectory = data;

    int startIndex = 0;
    int length = 1;
    while(startIndex+length <= trajectory->size)
    {
        int currentIndex = startIndex+length;
        double costNoParVal = costNoPar(startIndex,currentIndex);
        double costParVal = costPar(startIndex,currentIndex);

        if(costParVal > costNoParVal || length > maxLength)
        {
            criticalPoint.append(currentIndex);
            startIndex = currentIndex-1;
            length = 1;
        }
        else
        {
            length++;
        }
    }
    trajectory->criticalPointIndex = criticalPoint;
}

double mdl::costNoPar(const int startIndex, const int currentIndex)
{
    double eucliadianDistance = 0;
    for(int i = startIndex; i < currentIndex-startIndex;i++)
    {
        eucliadianDistance += distanceEucliadian(trajectory->position[i],trajectory->position[i+1]);
    }
    double LofH = log2(eucliadianDistance);

    //assume LofDH is zero

    return LofH;
}

double mdl::costPar(const int startIndex, const int currentIndex)
{
    double eucliadianDistance = distanceEucliadian(trajectory->position[startIndex],trajectory->position[currentIndex]);
    double LofH = log2(eucliadianDistance);

    double distanceV = 0;
    double distanceA = 0;
    for(int i = startIndex; i < currentIndex-startIndex; i++)
    {
        distanceV += distancePerpendicular(trajectory->position[startIndex],trajectory->position[currentIndex],trajectory->position[i],trajectory->position[i+1]);
        distanceA += distanceAngle(trajectory->position[startIndex],trajectory->position[currentIndex],trajectory->position[i],trajectory->position[i+1]);
    }
    double LofDH = log2(distanceV)+log2(distanceA);

    return LofH+LofDH;
}

double mdl::distancePerpendicular(const cv::Point pt1_s, const cv::Point pt1_e, const cv::Point pt2_s, const cv::Point pt2_e)
{
    int L1v = abs(pt2_s.y-pt1_s.y);
    int L2v = abs(pt2_e.y-pt1_e.y);

    return (pow(L1v,2)+pow(L2v,2))/(L1v+L2v);
}

double mdl::distanceAngle(const cv::Point pt1_s, const cv::Point pt1_e, const cv::Point pt2_s, const cv::Point pt2_e)
{
    int LeDistance = cv::norm(pt2_e-pt2_s);
    int LsDistance = cv::norm(pt1_e-pt1_s);

    cv::Point Le = pt2_e-pt2_s;
    cv::Point Ls = pt1_e-pt2_e;

    double dotVal = Le.x*Ls.x+Le.y*Ls.y;
    double cosVal = dotVal/(LeDistance*LsDistance);
    double angle = acos(cosVal);

    if(cosVal > 0)
        return LeDistance*sin(angle);
    else
        return LeDistance;

}

