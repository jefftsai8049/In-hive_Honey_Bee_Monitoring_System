#ifndef DATAPROCESSWINDOW_H
#define DATAPROCESSWINDOW_H

//STD LIB
#include <stdlib.h>
#include <string>
#include <sstream>

//QT LIB
#include <QMainWindow>
#include <QStringList>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QtConcurrent>
#include <QDateTime>

//by Tsai
#include "object_tracking.h"
#include "objecttrackingform.h"
#include "mdl.h"
#include "parameters.h"

//QCustomPlot LIB
#include <qcustomplot/qcustomplot.h>

//OpenCV
#include <opencv.hpp>

//#include <engine.h>

#include "whitelist.h"

#include "math.h"


struct weatherInfo
{
    QDateTime time;
    float inHiveTemp;
    float inHiveRH;
    float outHiveTemp;
    float outHiveRH;
    float pressure;
};

struct beeDailyInfo
{
    QString date;
    QStringList IDList;
    QVector<int> trajectoryCount;
    QVector< QVector<int> > motionPatternCount;
};

struct beeMotionPatternInfo
{
    QString date;
    QStringList IDList;
    QVector<int> trajectoryCount;
    QVector< QVector<int> > motionPatternCount;
};

struct inOutInfo
{
    QDateTime time;
    int direction;
    QString ID;
};

struct inOutDailyInfo
{
    QDate time;
    QVector<int> count;
    QVector<QString> ID;
};

struct frameDetected
{
    QDateTime time;
    QVector<QString> IDList;
    QVector<cv::Point> positionList;
    QVector<int> interactionID;

    void getInteractions(QVector<int> &interaction, const double &distanceThreshold);
};


namespace Ui {
class DataProcessWindow;
}

class DataProcessWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DataProcessWindow(QWidget *parent = 0);
    ~DataProcessWindow();


private slots:
    void on_data_preprocessing_pushButton_clicked();

    void on_actionOpen_Raw_Data_triggered();

    void receiveSystemLog(const QString &log);

    void receiveLoadDataFinish();

    void receiveProgress(const int &val);

    void receiveWhiteList(const QStringList &controlWhiteList,const QStringList &experimentWhiteList);

    void setObjectTrackingParameters(const objectTrackingParameters &params);

//    void pseudoColor(const cv::Mat &src,cv::Mat &dst);

    void on_actionOpen_Processed_Data_triggered();

    void on_trajectory_classify_pushButton_clicked();

    void on_actionObject_Tracking_triggered();

    void on_actionOpen_Sensor_Data_triggered();

    void on_actionWhite_List_triggered();

    void on_mdl_pushButton_clicked();

    void on_white_list_smoothing_pushButton_clicked();

    void on_test_pushButton_clicked();

    void on_distributed_area_pushButton_clicked();

    void on_c_value_spinBox_valueChanged(int arg1);

    void on_motion_pattern_filterpushButton_clicked();

    void on_actionOpen_In_Out_Data_triggered();

    void on_sub_group_distributed_area_pushButton_clicked();

    void on_actionDaily_Infomation_triggered();

    void on_actionDaily_Trajectory_Analysis_triggered();

    void on_actionHouly_Compare_triggered();

    void on_action2D_PCA_Plot_triggered();

    void on_action3D_Motion_Pattern_Plot_triggered();

    void on_open_script_pushButton_clicked();

    void on_save_script_pushButton_clicked();

    void on_interaction_matrix_pushButton_clicked();

    void on_trajectory_behavior_classifier_pushButton_clicked();

private:
    Ui::DataProcessWindow *ui;

    ObjectTrackingForm *OTS;

    object_tracking *OT;

    WhiteList *WL;

    QProcess *process;

    std::vector<track> path;

    QVector<trackPro> data;

    QVector<inOutInfo> inOutData;

    QVector<QString> inOutIDList;

    objectTrackingParameters OTParams;

    QStringList controlWhiteList;

    QStringList experimentWhiteList;

    QVector< cv::Scalar > colorTable;

    int cVal = 1;

    void loadWeatherData(const QStringList &fileNames,QVector<weatherInfo> &weatherData);

    //weather plot
    void getInHiveTemp(const QVector<weatherInfo> &weatherData,QVector<double> &x,QVector<double> &y);

    void getInHiveRH(const QVector<weatherInfo> &weatherData,QVector<double> &x,QVector<double> &y);

    void getOutHiveTemp(const QVector<weatherInfo> &weatherData,QVector<double> &x,QVector<double> &y);

    void getOutHiveRH(const QVector<weatherInfo> &weatherData,QVector<double> &x,QVector<double> &y);

    void getPressure(const QVector<weatherInfo> &weatherData,QVector<double> &x,QVector<double> &y);

    //bee info plot
    void getDailyBeeInfo(const QVector<trackPro> &data, QVector<QString> &x, QVector<double> &y);

    void plotBeeInfo(const QVector<trackPro> &data);

    //for fix length trajectory analysis
    void getGroupBeePatternRation(QVector<trackPro> &data, QVector<QStringList> &infoID, QVector< QVector<double> > &infoRatio);

    void getIndividualBeePatternRatio(QVector<trackPro> &data,QStringList &individualInfoID,QVector< QVector<double> > &individualInfoRatio);

    void getDailyInfo(QVector<beeDailyInfo> &beeInfo);

    //for behavior trajecotory analysis
    void getGroupBeePatternRation_behavior(QVector<trackPro> &data, QVector<QStringList> &infoID, QVector< QVector<double> > &infoRatio);

    void getIndividualBeePatternRatio_behavior(QVector<trackPro> &data,QStringList &individualInfoID,QVector< QVector<double> > &individualInfoRatio);

    //for output file
    void saveDailyInfoFile(const QString &fileName);

    //testing...
    void getTransitionMatrix(QVector<trackPro> &data, QStringList &individualInfoID, QVector<cv::Mat> &transition);

    void getDailyMotionPatternInfo(QVector<beeMotionPatternInfo> &dailyMotionInfo);

    void outBeeBehaviorInfo(QVector<trackPro> &data);

    void loadColorTable(const QString &fileName,QVector< cv::Scalar > &colorTable);

    QVector<double> movingAVG(const QVector<double> &raw,const int &length);

    QVector<int> trajectoryClassifier(const QVector<double> &raw,QVector<double> &distanceP2P_AVG, const double &staticThreshold, const double &loiteringThreshold);

    void OtsuMultiLevel(double &T1,double &T2,const std::vector<double> &val);


    //statistic
    double meanVarStd(const std::vector<double> &val, double &mean, double &std);

signals:
    void sendSystemLog(const QString &log);


};



#endif // DATAPROCESSWINDOW_H
