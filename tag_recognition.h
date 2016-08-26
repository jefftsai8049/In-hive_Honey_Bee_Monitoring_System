#ifndef TAG_RECOGNITION_H
#define TAG_RECOGNITION_H

//#define tagSize 24


#include <QObject>
#include <QDebug>
#include <QTime>
#include <QFileInfo>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QMap>

#include <opencv.hpp>
#ifdef IN_UNIX
#include "omp.h"
#endif
#include <core/ocl.hpp>

struct tagRecognitionParameters
{
    bool withHOG;
    bool withPCA;
    double CValPowUpper;
    double CValPowLower;
    double CValPow;
    int PCARemains;

};


class tag_recognition : public QObject
{
    Q_OBJECT
public:
    explicit tag_recognition(QObject *parent = 0);
    ~tag_recognition();

    //for tag image processing

    void tagImgProc(cv::Mat src, cv::Mat &word1, cv::Mat &word2);

    void wordImage2Data(cv::Mat &src);

    void wordImage2DataHOG(cv::Mat &src);

    int wordRecognition(cv::Mat &src);

    int wordMapping(const int &result);


    //for training model and load model

    void trainAllStep(const QString trainPathName,const QString testPathName,const tagRecognitionParameters params);

    bool loadSVMModel(const std::string &fileName);

    void loadTrainData(const QString path, cv::Mat &trainData, cv::Mat &trainLabel, const bool HOG);

    void loadTestData(const QString path, std::vector<cv::Mat> &testData, std::vector<int> &testLabel, const bool HOG);

    bool loadPCAModel(const std::string &fileName);

    void trainModel(const cv::Mat &trainData, const cv::Mat &trainLabel, const std::vector<cv::Mat> &testData, const std::vector<int> &testLabel, tagRecognitionParameters params);

    //for setting parameters

    void setTagBinaryThreshold(const int &value);

    void setPCAandHOG(const bool &PCAS,const bool &HOGS);

    void setTextSystem(const QString &textSys);

    int binaryThreshold;
signals:
    void sendSystemLog(const QString &msg);

public slots:


private:

    void findBlobs(const cv::Mat &binary, std::vector< std::vector<cv::Point2f> > &blobs);

    std::vector<cv::Point2f> findCircleBlobs(std::vector< std::vector<cv::Point2f> > &blobs);

    float calcualteCOV(std::vector<cv::Point2f> points);

    cv::Point2f calcualteMeanPoint(std::vector<cv::Point2f> val);

    float calcualteMean(std::vector<float> val);

    float calculateVar(std::vector<float> val);

    void sortblobs(std::vector< std::vector<cv::Point2f> > &blobs);

    void sortblobsSize(std::vector< std::vector<cv::Point2f> > &blobs);

    std::vector<std::vector<cv::Point2f> > maskRemoveBlobs(cv::Mat &src,std::vector<std::vector<cv::Point2f> > blobs);

    std::vector<std::vector<cv::Point2f> > removeImpossibleBlobs(std::vector<std::vector<cv::Point2f> > blobs);

    std::vector<std::vector<cv::Point2f> > removeImpossibleBlobsCOV(std::vector<std::vector<cv::Point2f> > blobs);

    float findRotateAngle(std::vector<cv::Point2f> blobsCenter, cv::Point2f &imgCenter);

    void drawBlob(cv::Mat &dst, std::vector<std::vector<cv::Point2f> > blobs);

    void drawBlobMask(cv::Mat &dst,std::vector<std::vector<cv::Point2f> > blobs);

    void findBlobCenter(std::vector< std::vector<cv::Point2f> > blobs,std::vector<cv::Point2f> &blobCenter);

    void cutWords(cv::Mat wordsMask, cv::Mat rawDst, cv::Mat &word1, cv::Mat &word2);

    cv::Ptr<cv::ml::SVM> SVMModel;

    cv::PCA pca;

    cv::Ptr<cv::ml::SVM> trainSVMModel;

    cv::PCA *trainPCA;

    bool HOGStatus;

    bool PCAStatus;

    //Contour Parameters

    int contourParam1;

    int contourParam2;

    //Text System

    QString textSys;

    QMap<int,int> SUTM_map;

    QMap<int,int> MUTM_map;

    QMap<int,int> Test_map;
};

#endif // TAG_RECOGNITION_H
