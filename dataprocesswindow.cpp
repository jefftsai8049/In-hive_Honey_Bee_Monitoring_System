#include "dataprocesswindow.h"
#include "ui_dataprocesswindow.h"

DataProcessWindow::DataProcessWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataProcessWindow)
{
    ui->setupUi(this);
    OT = new object_tracking;
    OTS = new ObjectTrackingForm;
    connect(this,SIGNAL(sendSystemLog(QString)),this,SLOT(receiveSystemLog(QString)));

    connect(OT,SIGNAL(sendSystemLog(QString)),this,SLOT(receiveSystemLog(QString)));
    connect(OT,SIGNAL(sendLoadRawDataFinish()),this,SLOT(receiveLoadDataFinish()));
    connect(OT,SIGNAL(sendProgress(int)),this,SLOT(receiveProgress(int)));

    connect(OTS,SIGNAL(setObjectTrackingParameters(objectTrackingParameters)),this,SLOT(setObjectTrackingParameters(objectTrackingParameters)));
    OTS->requestObjectTrackingParameters();
    OT->setPathSegmentSize(this->OTParams.segmentSize);
}

DataProcessWindow::~DataProcessWindow()
{
    delete ui;
}


void DataProcessWindow::on_data_preprocessing_pushButton_clicked()
{
    QFuture<void> rawDataProcessing = QtConcurrent::run(OT,&object_tracking::rawDataPreprocessing,&this->path,&this->data);
    rawDataProcessing.waitForFinished();
    this->path.clear();

}

void DataProcessWindow::receiveSystemLog(const QString &log)
{
    ui->system_log_textBrowser->append(log);
}

void DataProcessWindow::receiveLoadDataFinish()
{
    ui->data_preprocessing_pushButton->setEnabled(true);
}

void DataProcessWindow::receiveProgress(const int &val)
{
    ui->progressBar->setValue(val);
}

void DataProcessWindow::setObjectTrackingParameters(const objectTrackingParameters &params)
{
    OTParams = params;
    OT->setPathSegmentSize(this->OTParams.segmentSize);
}

void DataProcessWindow::on_actionOpen_Raw_Data_triggered()
{
    ui->data_preprocessing_pushButton->setEnabled(false);
    QStringList fileNames;
    fileNames = QFileDialog::getOpenFileNames(this,"Open Data File","","Data Files (*.csv)");
    if(fileNames.size() == 0)
        return;
    QFuture<void> loadDataFunction = QtConcurrent::run(OT,&object_tracking::loadDataTrack,fileNames,&this->path);
}

void DataProcessWindow::on_actionOpen_Processed_Data_triggered()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,"Open Data File","","Data Files (*.csv)");

    if(fileName.size() == 0)
        return;
    QFuture<void> loadDataTrackPro = QtConcurrent::run(OT,&object_tracking::loadDataTrackPro,fileName,&this->data);
    //OT->loadDataTrackPro(fileName,&this->data);
    loadDataTrackPro.waitForFinished();
    //qDebug() << this->data.size();

    ui->trajectory_classify_pushButton->setEnabled(true);

}

void DataProcessWindow::on_trajectory_classify_pushButton_clicked()
{
    OT->tracjectoryClassify(this->data,this->OTParams);
}

void DataProcessWindow::on_actionObject_Tracking_triggered()
{
    OTS->show();
}

void DataProcessWindow::on_actionOpen_Sensor_Data_triggered()
{
    QStringList fileNames;
    fileNames = QFileDialog::getOpenFileNames(this,"Open Sensor File","","Data Files (*.csv)");

    QVector<weatherInfo> weatherData;
    try
    {
        this->loadWeatherData(fileNames,weatherData);
    }
    catch(const QString &errMsg)
    {
        ui->system_log_textBrowser->insertPlainText(errMsg);
    }
    //set title font size
    QFont font;
    font.setPointSize(14);

    //
    ui->rh_info_widget->plotLayout()->insertRow(0);
    QCPPlotTitle *rhTitle = new QCPPlotTitle(ui->rh_info_widget, "Humidity In-Out Bee Hive");
    rhTitle->setFont(font);
    ui->rh_info_widget->plotLayout()->addElement(0, 0, rhTitle);

    QVector<double> x1,y1;
    this->getInHiveRH(weatherData,x1,y1);
    ui->rh_info_widget->addGraph();
    ui->rh_info_widget->graph(0)->setData(x1,y1);
    ui->rh_info_widget->graph(0)->setPen(QPen(Qt::red,2));
    ui->rh_info_widget->graph(0)->setName("In-Hive");


    QVector<double> x2,y2;
    this->getOutHiveRH(weatherData,x2,y2);
    ui->rh_info_widget->addGraph();
    ui->rh_info_widget->graph(1)->setData(x2,y2);
    ui->rh_info_widget->graph(1)->setPen(QPen(Qt::blue,2));
    ui->rh_info_widget->graph(1)->setName("Out-Hive");

    ui->rh_info_widget->xAxis->rescale();
    ui->rh_info_widget->yAxis->rescale();
    ui->rh_info_widget->yAxis->setVisible(true);
    ui->rh_info_widget->yAxis->setLabel("Humidity (%)");

    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setAutoTickStep(false);
    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickStep(60*10*6*6); // 4 day tickstep
    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelType(QCPAxis::ltDateTime);
    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeSpec(Qt::LocalTime);
    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeFormat("MM-dd hh:mm");
    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelRotation(30);

    ui->temp_info_widget->plotLayout()->insertRow(0);
    QCPPlotTitle *tempTitle = new QCPPlotTitle(ui->temp_info_widget, "Temperture In-Out Bee Hive");
    tempTitle->setFont(font);
    ui->temp_info_widget->plotLayout()->addElement(0, 0, tempTitle);

    QVector<double> x3,y3;
    this->getInHiveTemp(weatherData,x3,y3);
    ui->temp_info_widget->addGraph(ui->temp_info_widget->xAxis,ui->temp_info_widget->yAxis);
    ui->temp_info_widget->graph(0)->setData(x3,y3);
    ui->temp_info_widget->graph(0)->setPen(QPen(QColor(Qt::red),2));
    ui->temp_info_widget->graph(0)->setName("In-Hive");

    QVector<double> x4,y4;
    this->getOutHiveTemp(weatherData,x4,y4);
    ui->temp_info_widget->addGraph(ui->temp_info_widget->xAxis,ui->temp_info_widget->yAxis);
    ui->temp_info_widget->graph(1)->setData(x4,y4);
    ui->temp_info_widget->graph(1)->setPen(QPen(QColor(Qt::blue),2));
    ui->temp_info_widget->graph(1)->setName("Out-Hive");

    ui->temp_info_widget->xAxis->rescale();
    ui->temp_info_widget->yAxis->rescale();
    ui->temp_info_widget->yAxis->setVisible(true);
    ui->temp_info_widget->yAxis->setLabel("Temperture (C)");

    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setAutoTickStep(false);
    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickStep(60*10*6*6); // 4 day tickstep
    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelType(QCPAxis::ltDateTime);
    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeSpec(Qt::LocalTime);
    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeFormat("MM-dd hh:mm");
    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelRotation(30);


    ui->pressure_info_widget->plotLayout()->insertRow(0);
    QCPPlotTitle *pressureTitle = new QCPPlotTitle(ui->pressure_info_widget, "Air Pressure");
    pressureTitle->setFont(font);
    ui->pressure_info_widget->plotLayout()->addElement(0, 0, pressureTitle);

    QVector<double> x5,y5;
    this->getPressure(weatherData,x5,y5);
    ui->pressure_info_widget->addGraph(ui->pressure_info_widget->xAxis,ui->pressure_info_widget->yAxis);
    ui->pressure_info_widget->graph(0)->setData(x5,y5);
    ui->pressure_info_widget->graph(0)->setPen(QPen(QColor(Qt::black),2));
    ui->pressure_info_widget->graph(0)->setName("Air Pressure");

    ui->pressure_info_widget->xAxis->rescale();
    ui->pressure_info_widget->yAxis->rescale();
    ui->pressure_info_widget->yAxis->setVisible(true);
    ui->pressure_info_widget->yAxis->setLabel("Air Pressure (hPa)");

    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setAutoTickStep(false);
    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickStep(60*10*6*6); // 4 day tickstep
    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelType(QCPAxis::ltDateTime);
    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeSpec(Qt::LocalTime);
    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeFormat("MM-dd hh:mm");
    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelRotation(30);

    ui->pressure_info_widget->legend->setVisible(true);
    ui->pressure_info_widget->replot();

    ui->temp_info_widget->legend->setVisible(true);
    ui->temp_info_widget->replot();

    ui->rh_info_widget->legend->setVisible(true);
    ui->rh_info_widget->replot();


}

void DataProcessWindow::loadWeatherData(const QStringList &fileNames, QVector<weatherInfo> &weatherData)
{
    weatherData.clear();
    for(int i = 0; i < fileNames.size(); i++)
    {
        QFile file(fileNames.at(i));
        if(!file.exists())
        {
            throw fileNames.at(i)+QString(" file not exist");
            return;
        }

        file.open(QIODevice::ReadOnly);
        while(!file.atEnd())
        {
            QString msg = file.readLine();
            msg = msg.trimmed();
            QStringList data = msg.split(",");
            if(data.at(0)!="")
            {
                weatherInfo wData;

                wData.time = QDateTime::fromString(data.at(0),"yyyy-MM-dd_hh-mm-ss-zzz");
                wData.inHiveTemp = data.at(1).toFloat();
                wData.inHiveRH = data.at(2).toFloat();
                wData.outHiveTemp = data.at(3).toFloat();
                wData.outHiveRH = data.at(4).toFloat();
                wData.pressure = data.at(5).toFloat();

                weatherData.push_back(wData);
            }
        }
    }
}

void DataProcessWindow::getInHiveTemp(const QVector<weatherInfo> &weatherData, QVector<double> &x, QVector<double> &y)
{
    x.clear();
    y.clear();

    x.resize(weatherData.size());
    y.resize(weatherData.size());

    for(int i = 0; i < weatherData.size(); i++)
    {
        x[i] = weatherData.at(i).time.toTime_t();
        y[i] = weatherData.at(i).inHiveTemp;
    }
}

void DataProcessWindow::getInHiveRH(const QVector<weatherInfo> &weatherData, QVector<double> &x, QVector<double> &y)
{
    x.clear();
    y.clear();

    x.resize(weatherData.size());
    y.resize(weatherData.size());

    for(int i = 0; i < weatherData.size(); i++)
    {
        x[i] = weatherData.at(i).time.toTime_t();
        y[i] = weatherData.at(i).inHiveRH;
    }
}

void DataProcessWindow::getOutHiveTemp(const QVector<weatherInfo> &weatherData, QVector<double> &x, QVector<double> &y)
{
    x.clear();
    y.clear();

    x.resize(weatherData.size());
    y.resize(weatherData.size());

    for(int i = 0; i < weatherData.size(); i++)
    {
        x[i] = weatherData.at(i).time.toTime_t();
        y[i] = weatherData.at(i).outHiveTemp;
    }
}

void DataProcessWindow::getOutHiveRH(const QVector<weatherInfo> &weatherData, QVector<double> &x, QVector<double> &y)
{
    x.clear();
    y.clear();

    x.resize(weatherData.size());
    y.resize(weatherData.size());

    for(int i = 0; i < weatherData.size(); i++)
    {
        x[i] = weatherData.at(i).time.toTime_t();
        y[i] = weatherData.at(i).outHiveRH;
    }
}

void DataProcessWindow::getPressure(const QVector<weatherInfo> &weatherData, QVector<double> &x, QVector<double> &y)
{
    x.clear();
    y.clear();

    x.resize(weatherData.size());
    y.resize(weatherData.size());

    for(int i = 0; i < weatherData.size(); i++)
    {
        x[i] = weatherData.at(i).time.toTime_t();
        y[i] = weatherData.at(i).pressure;
    }
}
