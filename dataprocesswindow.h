#ifndef DATAPROCESSWINDOW_H
#define DATAPROCESSWINDOW_H

//QT LIB
#include <QMainWindow>
#include <QStringList>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QtConcurrent>
#include <QDateTime>

//Tsai LIB
#include "object_tracking.h"
#include "objecttrackingform.h"

//QCustomPlot LIB
#include <qcustomplot/qcustomplot.h>

//OpenCV
#include <opencv.hpp>


struct weatherInfo
{
    QDateTime time;
    float inHiveTemp;
    float inHiveRH;
    float outHiveTemp;
    float outHiveRH;
    float pressure;
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

    void setObjectTrackingParameters(const objectTrackingParameters &params);

    void on_actionOpen_Processed_Data_triggered();

    void on_trajectory_classify_pushButton_clicked();

    void on_actionObject_Tracking_triggered();

    void on_actionOpen_Sensor_Data_triggered();

private:
    Ui::DataProcessWindow *ui;

    ObjectTrackingForm *OTS;

    object_tracking *OT;

    std::vector<track> path;

    QVector<trackPro> data;

    objectTrackingParameters OTParams;

    void loadWeatherData(const QStringList &fileNames,QVector<weatherInfo> &weatherData);

    void getInHiveTemp(const QVector<weatherInfo> &weatherData,QVector<double> &x,QVector<double> &y);

    void getInHiveRH(const QVector<weatherInfo> &weatherData,QVector<double> &x,QVector<double> &y);

    void getOutHiveTemp(const QVector<weatherInfo> &weatherData,QVector<double> &x,QVector<double> &y);

    void getOutHiveRH(const QVector<weatherInfo> &weatherData,QVector<double> &x,QVector<double> &y);

    void getPressure(const QVector<weatherInfo> &weatherData,QVector<double> &x,QVector<double> &y);


signals:
    void sendSystemLog(const QString &log);


};



#endif // DATAPROCESSWINDOW_H
