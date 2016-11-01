#include "dataprocesswindow.h"
#include "ui_dataprocesswindow.h"

QStringList SUTM;

DataProcessWindow::DataProcessWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataProcessWindow)
{
    ui->setupUi(this);
    //for analyze trajectory
    OT = new object_tracking;
    //for setting OT parameters UI form
    OTS = new ObjectTrackingForm;
    //for setting ID white list UI
    WL = new WhiteList;

    //connect system log recorder and show
    connect(this,SIGNAL(sendSystemLog(QString)),this,SLOT(receiveSystemLog(QString)));
    //connect OT class system log to log recorder and show
    connect(OT,SIGNAL(sendSystemLog(QString)),this,SLOT(receiveSystemLog(QString)));
    //
    connect(OT,SIGNAL(sendLoadRawDataFinish()),this,SLOT(receiveLoadDataFinish()));
    //connect processing progress bar to ui
    connect(OT,SIGNAL(sendProgress(int)),this,SLOT(receiveProgress(int)));
    //connect the OT setting UI and this UI to sned the object tracking parameters
    connect(OTS,SIGNAL(setObjectTrackingParameters(objectTrackingParameters)),this,SLOT(setObjectTrackingParameters(objectTrackingParameters)));
    //connect the whitelist UI and this UI to sned the white list
    connect(WL,SIGNAL(sendWhiteList(QStringList,QStringList)),this,SLOT(receiveWhiteList(QStringList,QStringList)));


    //request object tracking parameters from the OTS UI
    OTS->requestObjectTrackingParameters();
    //set up trajectory segmentation size
    OT->setPathSegmentSize(this->OTParams.segmentSize);


    SUTM << "A" << "B" << "C" << "E" << "F" << "G" << "H" << "K" << "L" << "O" << "P" << "R" << "S" << "T" << "U" << "Y" << "Z";

    //for drawing trajectory color table
    loadColorTable("model/color_table.csv",this->colorTable);

    //new a QProcess for running matlab script
    process = new QProcess(this);
}

DataProcessWindow::~DataProcessWindow()
{
    delete ui;
}


void DataProcessWindow::on_data_preprocessing_pushButton_clicked()
{
    //set button enable
    ui->data_preprocessing_pushButton->setEnabled(false);

    //process raw data
    QFuture<void> rawDataProcessing = QtConcurrent::run(OT,&object_tracking::rawDataPreprocessing,&this->path,&this->data);
    rawDataProcessing.waitForFinished();
    this->path.clear();

    //set next button enable
    ui->white_list_smoothing_pushButton->setEnabled(true);


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

void DataProcessWindow::receiveWhiteList(const QStringList &controlWhiteList, const QStringList &experimentWhiteList)
{
    //show white list on ui
    this->controlWhiteList = controlWhiteList;
    this->experimentWhiteList = experimentWhiteList;
    QString msg = "White List : ";
    for(int i = 0; i < this->controlWhiteList.size(); i++)
        msg.append(this->controlWhiteList.at(i));
    for(int i = 0; i < this->experimentWhiteList.size(); i++)
        msg.append(this->experimentWhiteList.at(i));
    ui->current_white_list_label->setText(msg);
}

void DataProcessWindow::setObjectTrackingParameters(const objectTrackingParameters &params)
{
    //receive object trajectory parameters from OTS ui
    OTParams = params;
    OT->setPathSegmentSize(this->OTParams.segmentSize);
    //send system log
    emit sendSystemLog("Object tracking parameters set!");
}

void DataProcessWindow::on_actionOpen_Raw_Data_triggered()
{
    //set button
    ui->data_preprocessing_pushButton->setEnabled(false);
    ui->white_list_smoothing_pushButton->setEnabled(false);
    ui->trajectory_classify_pushButton->setEnabled(false);
    ui->distributed_area_pushButton->setEnabled(false);
    ui->mdl_pushButton->setEnabled(false);
    ui->test_pushButton->setEnabled(false);
    ui->motion_pattern_filterpushButton->setEnabled(false);

    //load raw processing data
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
    loadDataTrackPro.waitForFinished();

    ui->trajectory_classify_pushButton->setEnabled(true);
    ui->white_list_smoothing_pushButton->setEnabled(true);

}

void DataProcessWindow::on_trajectory_classify_pushButton_clicked()
{
    //pattern classification
    OT->trajectoryClassify(this->data,this->OTParams);


    //group pattern count and ratio
    QVector<QStringList> infoID;
    infoID << this->controlWhiteList << this->experimentWhiteList;
    QVector< QVector<double> > infoRatio;
    this->getGroupBeePatternRation(this->data,infoID,infoRatio);

    //individual pattern ratio
    QStringList individualInfoID;
    QVector< QVector<double> > individualInfoRatio;
    this->getIndividualBeePatternRatio(this->data,individualInfoID,individualInfoRatio);


    cv::Mat PCAData;
    PCAData.create(individualInfoRatio.size(),PATTERN_TYPES,CV_64FC1);
    for(int i = 0; i < individualInfoRatio.size(); i++)
    {
        for(int j = 0; j < PATTERN_TYPES; j++)
        {
            PCAData.at<double>(i,j) = individualInfoRatio[i][j];
        }
    }

    //plot 2D-PCA
    int dims = 2;

    cv::PCA *PCA_2D;
    PCA_2D = new cv::PCA(PCAData,cv::Mat(),cv::PCA::DATA_AS_ROW,dims);
    PCAData = PCA_2D->project(PCAData);
    cv::normalize(PCAData,PCAData,0,1,cv::NORM_MINMAX);

    //check folder exist or not
    QDir outInfo("out/bee_info");
    if(!outInfo.exists())
    {
        outInfo.cdUp();
        outInfo.mkdir("bee_info");
        outInfo.cd("bee_info");
    }
    QFile beePCAFile(outInfo.absolutePath()+"/"+"individual_PCA.csv");
    beePCAFile.open(QIODevice::ReadWrite);

    QTextStream outPCA(&beePCAFile);
    for(int i = 0; i < individualInfoRatio.size(); i++)
    {
        outPCA << individualInfoID[i] << ",";
        for(int j = 0; j < dims; j++)
        {
            outPCA << PCAData.at<double>(i,j) << ",";
        }
        outPCA << "\n";
    }
    beePCAFile.close();

    QFile beeInfoFile(outInfo.absolutePath()+"/"+"individual_info.csv");
    beeInfoFile.open(QIODevice::ReadWrite);

    QTextStream out(&beeInfoFile);

    for(int i = 0; i < individualInfoRatio.size(); i++)
    {
        out << individualInfoID[i] << ",";
        for(int j = 0; j < PATTERN_TYPES; j++)
        {
            out << individualInfoRatio[i][j] << ",";
        }
        out << "\n";
    }
    beeInfoFile.close();

    QVector<beeDailyInfo> beeInfo;
    this->getDailyInfo(beeInfo);

    QFile motionPatternCountFile(outInfo.absolutePath()+"/"+"motion_pattern_count.csv");
    motionPatternCountFile.open(QIODevice::ReadWrite);
    QTextStream motionOut(&motionPatternCountFile);
    for(int i = 0; i < beeInfo.size();i++)
    {

        motionOut << beeInfo[i].date << "\n";
        for(int j = 0; j < beeInfo[i].IDList.size();j++)
        {
            motionOut << beeInfo[i].IDList[j] << ",";
            for(int k = 0 ; k < beeInfo[i].motionPatternCount[j].size(); k++)
            {
                motionOut << beeInfo[i].motionPatternCount[j][k];
                if(k != beeInfo[i].motionPatternCount[j].size()-1)
                    motionOut << ",";
            }
            motionOut << "\n";
        }
    }
    motionPatternCountFile.close();

    //get trajectory distance

    //    qDebug() << outInfo.absolutePath()+"/trajectory_distance.csv";
    QFile file;
    file.setFileName(outInfo.absolutePath()+"/trajectory_info.csv");
    file.open(QIODevice::WriteOnly);
    QTextStream outTra(&file);

    outTra << "ID,StartTime,Trajectory_distance,Trajectory_velocity,Detected_time,"
           << "Static_count,Loitering_count,Moving_forward_count,Moving_CW_count,Moving_CCW_count,Waggle_count,Count_sum,"
           << "Static_ratio,Loitering_ratio,Moving_ratio,Group\n";

    for(int i = 0; i < this->data.size(); i++)
    {
        if(this->data[i].getTrajectoryMovingVelocity() < 250 &&this->data[i].position.size() > 12)
        {
            outTra << this->data[i].ID << ",";
            outTra << this->data[i].startTime.toString("yyyy-MM-dd-hh-mm-ss") << ",";
            outTra << this->data[i].getTrajectoryMovingDistance() << ",";
            outTra << this->data[i].getTrajectoryMovingVelocity() << ",";
            outTra << this->data[i].getDetectedTime() << ",";

            QVector<double> patternCount = this->data[i].getPatternCount();
            outTra << patternCount[0] << ",";
            outTra << patternCount[1] << ",";
            outTra << patternCount[2] << ",";
            outTra << patternCount[3] << ",";
            outTra << patternCount[4] << ",";
            outTra << patternCount[5] << ",";

            double patternCountSum = patternCount[0]+patternCount[1]+patternCount[2]+patternCount[3]+patternCount[4]+patternCount[5];
            outTra << patternCountSum << ",";
            outTra << patternCount[0]/patternCountSum << ",";
            outTra << patternCount[1]/patternCountSum << ",";
            outTra << (patternCount[2]+patternCount[3]+patternCount[4]+patternCount[5])/patternCountSum << ",";

            if(this->controlWhiteList.indexOf(QString(this->data[i].ID[0])) > -1)
                outTra << 0;
            else if(this->experimentWhiteList.indexOf(QString(this->data[i].ID[0])) > -1)
                outTra << 1;
            else
                outTra << -1;

            outTra << "\n";
        }
    }
    file.close();

    this->outBeeBehaviorInfo(this->data);

}

void DataProcessWindow::on_actionObject_Tracking_triggered()
{
    //show object tracking setting UI
    OTS->show();
}

void DataProcessWindow::on_actionOpen_Sensor_Data_triggered()
{
    //load sensor data
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

    //insert title and set font
    ui->rh_info_widget->plotLayout()->insertRow(0);
    QCPPlotTitle *rhTitle = new QCPPlotTitle(ui->rh_info_widget, "Humidity In-Out Bee Hive");
    rhTitle->setFont(font);
    ui->rh_info_widget->plotLayout()->addElement(0, 0, rhTitle);

    ui->temp_info_widget->plotLayout()->insertRow(0);
    QCPPlotTitle *tempTitle = new QCPPlotTitle(ui->temp_info_widget, "Temperture In-Out Bee Hive");
    tempTitle->setFont(font);
    ui->temp_info_widget->plotLayout()->addElement(0, 0, tempTitle);

    ui->pressure_info_widget->plotLayout()->insertRow(0);
    QCPPlotTitle *pressureTitle = new QCPPlotTitle(ui->pressure_info_widget, "Air Pressure");
    pressureTitle->setFont(font);
    ui->pressure_info_widget->plotLayout()->addElement(0, 0, pressureTitle);

    //add humidity graph
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

    //add temperture graph
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

    //add pressure graph
    QVector<double> x5,y5;
    this->getPressure(weatherData,x5,y5);
    ui->pressure_info_widget->addGraph(ui->pressure_info_widget->xAxis,ui->pressure_info_widget->yAxis);
    ui->pressure_info_widget->graph(0)->setData(x5,y5);
    ui->pressure_info_widget->graph(0)->setPen(QPen(QColor(Qt::black),2));
    ui->pressure_info_widget->graph(0)->setName("Air Pressure");


    //set scale and label
    ui->rh_info_widget->xAxis->rescale();
    ui->rh_info_widget->yAxis->rescale();
    ui->rh_info_widget->yAxis->setVisible(true);
    ui->rh_info_widget->yAxis->setLabel("Humidity (%)");

    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setAutoTickStep(false);
    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickStep(60*10*6*12); // 4 day tickstep
    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelType(QCPAxis::ltDateTime);
    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeSpec(Qt::LocalTime);
    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeFormat("MM-dd hh:mm");
    ui->rh_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelRotation(30);

    ui->temp_info_widget->xAxis->rescale();
    ui->temp_info_widget->yAxis->rescale();
    ui->temp_info_widget->yAxis->setVisible(true);
    ui->temp_info_widget->yAxis->setLabel("Temperture (C)");

    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setAutoTickStep(false);
    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickStep(60*10*6*12); // 4 day tickstep
    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelType(QCPAxis::ltDateTime);
    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeSpec(Qt::LocalTime);
    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeFormat("MM-dd hh:mm");
    ui->temp_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelRotation(30);

    ui->pressure_info_widget->xAxis->rescale();
    ui->pressure_info_widget->yAxis->rescale();
    ui->pressure_info_widget->yAxis->setVisible(true);
    ui->pressure_info_widget->yAxis->setLabel("Air Pressure (hPa)");

    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setAutoTickStep(false);
    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickStep(60*10*6*12); // 4 day tickstep
    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelType(QCPAxis::ltDateTime);
    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeSpec(Qt::LocalTime);
    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setDateTimeFormat("MM-dd hh:mm");
    ui->pressure_info_widget->axisRect()->axis(QCPAxis::atBottom)->setTickLabelRotation(30);

    //replot
    //ui->pressure_info_widget->legend->setVisible(true);
    ui->pressure_info_widget->replot();

    //ui->temp_info_widget->legend->setVisible(true);
    ui->temp_info_widget->replot();

    //ui->rh_info_widget->legend->setVisible(true);
    ui->rh_info_widget->replot();

    //save chart
    QDir outChart("out/chart");
    if(!outChart.exists())
    {
        outChart.cdUp();
        outChart.mkdir("chart");
        outChart.cd("chart");

    }
    ui->pressure_info_widget->savePng(outChart.absolutePath()+"/"+"pressure_info.png");
    ui->rh_info_widget->savePng(outChart.absolutePath()+"/"+"humidity_info.png");
    ui->temp_info_widget->savePng(outChart.absolutePath()+"/"+"temperture_info.png");
}

void DataProcessWindow::loadWeatherData(const QStringList &fileNames, QVector<weatherInfo> &weatherData)
{
    //load weather data
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
    //transform the weatherinfo data to plotable format
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
    //transform the weatherinfo data to plotable format
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
    //transform the weatherinfo data to plotable format
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
    //transform the weatherinfo data to plotable format
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
    //transform the weatherinfo data to plotable format
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

void DataProcessWindow::getDailyBeeInfo(const QVector<trackPro> &data, QVector<QString> &x, QVector<double> &y)
{
    //transform the weatherinfo data to plotable format
    x.clear();
    y.clear();

    double dayCount = 0;
    QStringList day;
    for(int i = 0 ; i < data.size(); i++)
    {
        if(!day.contains(data.at(i).startTime.toString("dd")))
        {
            day << data.at(i).startTime.toString("dd");
            x.append(data.at(i).startTime.toString("MM-dd"));
            y.append(dayCount);
            dayCount = 0;
        }
        dayCount++;
    }
    y.append(dayCount);
    y.remove(0);
}

void DataProcessWindow::plotBeeInfo(const QVector<trackPro> &data)
{
    //set title font size
    QFont font;
    font.setPointSize(14);

    //clear plot
    //    ui->bee_info_widget->plotLayout()->clear();

    //insert title and set font
    ui->bee_info_widget->plotLayout()->insertRow(0);
    QCPPlotTitle *beeTitle = new QCPPlotTitle(ui->bee_info_widget, "Daily Effectively Track");
    beeTitle->setFont(font);
    ui->bee_info_widget->plotLayout()->addElement(0, 0, beeTitle);

    //get data
    QVector<QString> x;
    QVector<double> y;
    QVector<double> tick;

    this->getDailyBeeInfo(data,x,y);
    for(int i = 0; i < x.size(); i++)
    {
        tick.append(i+1);
    }
    //qDebug() <<x << y<< tick;

    //set bar
    QCPBars *dailyInfo = new QCPBars(ui->bee_info_widget->xAxis,ui->bee_info_widget->yAxis);
    ui->bee_info_widget->addPlottable(dailyInfo);
    dailyInfo->setData(tick,y);

    //set color and width
    QPen pen;
    pen.setWidth(1.2);
    pen.setColor(QColor(255,131,0));
    dailyInfo->setPen(pen);
    dailyInfo->setBrush(QColor(255, 131, 0, 50));

    //set x axis
    ui->bee_info_widget->xAxis->setAutoTicks(false);
    ui->bee_info_widget->xAxis->setAutoTickLabels(false);
    ui->bee_info_widget->xAxis->setTickVector(tick);
    ui->bee_info_widget->xAxis->setTickVectorLabels(x);
    ui->bee_info_widget->xAxis->setTickLabelRotation(60);
    ui->bee_info_widget->xAxis->setSubTickCount(0);
    ui->bee_info_widget->xAxis->setTickLength(0, 4);
    ui->bee_info_widget->xAxis->grid()->setVisible(false);
    ui->bee_info_widget->xAxis->setRange(0,y.size()+1);
    ui->bee_info_widget->xAxis->setLabel("Date");

    //set y axis
    double yMax = 0;
    for(int i = 0; i < y.size();i++)
    {
        if(y.at(i) > yMax)
            yMax = y.at(i);
    }
    ui->bee_info_widget->yAxis->setRange(0,yMax);
    ui->bee_info_widget->yAxis->grid()->setVisible(true);
    ui->bee_info_widget->yAxis->setLabel("Number of tracks");
    ui->bee_info_widget->replot();

    //save chart
    QDir outChart("out/chart");
    if(!outChart.exists())
    {
        outChart.cdUp();
        outChart.mkdir("chart");
        outChart.cd("chart");

    }
    ui->bee_info_widget->savePng(outChart.absolutePath()+"/"+"daily_effectively_track.png");
}

void DataProcessWindow::getGroupBeePatternRation(QVector<trackPro> &data, QVector<QStringList> &infoID, QVector<QVector<double> > &infoRatio)
{
    QVector< QVector<double> > group(infoID.size());

    for(int i = 0; i < infoID.size();i++)
    {
        group[i].resize(PATTERN_TYPES);

    }
    for(int i = 0; i < data.size();i++)
    {
        int gID;
        for(int j = 0; j < infoID.size();j++)
        {
            if(infoID[j].contains(QString(data[i].ID[0])))
            {
                gID = j;
                break;
            }
        }
        for(int j = 0; j < PATTERN_TYPES; j++)
            group[gID][j] += data[i].getPatternCount()[j];
    }

    QVector<double> sum(infoID.size());
    for(int i = 0; i < infoID.size(); i++)
    {
        for(int j = 0; j < PATTERN_TYPES;j++)
        {
            sum[i] += group[i][j];

        }
    }
    for(int i = 0; i < infoID.size(); i++)
    {
        for(int j = 0; j < PATTERN_TYPES;j++)
        {
            group[i][j] /= sum[i];
        }
    }
    infoRatio = group;
}

void DataProcessWindow::getIndividualBeePatternRatio(QVector<trackPro> &data,QStringList &individualInfoID,QVector< QVector<double> > &individualInfoRatio)
{
    //individual bee pattern count and ratio
    for(int i = 0; i < data.size();i++)
    {
        QString ID = data.at(i).ID;
        QVector<double> count = data[i].getPatternCount();

        int index = individualInfoID.indexOf(ID);
        if(index == -1)
        {
            individualInfoID.append(ID);
            individualInfoRatio.append(count);
        }
        else
        {
            for(int j = 0; j < count.size(); j++)
            {
                individualInfoRatio[index][j] += count[j];
            }
        }
    }
    //sort
    for(int i = 0; i < individualInfoID.size()-1; i++)
    {
        for(int j = i; j < individualInfoID.size(); j++)
        {
            if(individualInfoID[i][0]>individualInfoID[j][0])
            {
                individualInfoID.swap(i,j);
                QVector<double> temp = individualInfoRatio[i];
                individualInfoRatio[i] = individualInfoRatio[j];
                individualInfoRatio[j] = temp;
            }
            else if(individualInfoID[i][0]==individualInfoID[j][0])
            {
                if(individualInfoID[i][1]>individualInfoID[j][1])
                {
                    individualInfoID.swap(i,j);
                    QVector<double> temp = individualInfoRatio[i];
                    individualInfoRatio[i] = individualInfoRatio[j];
                    individualInfoRatio[j] = temp;
                }
            }
        }
    }
    //get ratio
    for(int i = 0; i < individualInfoID.size(); i++)
    {
        double sum = 0;
        for(int j = 0; j < individualInfoRatio[i].size(); j++)
        {
            sum += individualInfoRatio[i][j];
        }
        if(sum < BEE_TRACK_COUNT)
        {
            individualInfoID.removeAt(i);
            individualInfoRatio.remove(i);
            i--;
        }
        else{
            for(int j = 0; j < individualInfoRatio[i].size(); j++)
            {
                individualInfoRatio[i][j] /= sum;
            }
        }
    }
    //remove impossible data
    for(int i = 0; i < individualInfoID.size(); i++)
    {
        if(!SUTM.contains(QString(individualInfoID[i][1])))
        {
            individualInfoID.removeAt(i);
            individualInfoRatio.remove(i);
            i--;
        }
    }

}

void DataProcessWindow::getDailyInfo(QVector<beeDailyInfo> &beeInfo)
{
    beeInfo.clear();
    //    QVector<beeDailyInfo> beeInfo;
    for(int i = 0;i < this->data.size();i++)
    {
        //check day in list or not
        bool isExist = 0;
        int day = 0;
        QString beeDate = this->data[i].startTime.toString("yyyy-MM-dd");
        for(int j = 0; j < beeInfo.size();j++)
        {
            if(beeDate == beeInfo[j].date)
            {
                isExist = 1;
                day = j;
                break;
            }
        }
        if(!isExist)
        {
            beeDailyInfo dailyInfo;
            dailyInfo.date = beeDate;
            beeInfo.append(dailyInfo);
            isExist = 1;
            day = beeInfo.size()-1;
        }
        //check ID in list or not
        bool isInList = 0;
        int IDNumber;
        for(int j = 0; j < beeInfo[day].IDList.size();j++)
        {
            if(beeInfo[day].IDList[j] == this->data[i].ID)
            {
                isInList = 1;
                IDNumber = j;
                break;
            }

        }
        if(!isInList)
        {
            beeInfo[day].IDList.append(this->data[i].ID);
            beeInfo[day].trajectoryCount.append(0);
            QVector<int> mpc(6);
            beeInfo[day].motionPatternCount.append(mpc);
            IDNumber = beeInfo[day].IDList.size()-1;
        }
        //qDebug() << this->data[i].ID << this->data[i].pattern.size() << this->data[i].pattern;
        for(int k = 0;k < this->data[i].pattern.size();k++)
        {
            beeInfo[day].motionPatternCount[IDNumber][this->data[i].pattern[k]]++;
            //qDebug() << this->data[i].pattern[k];
        }
        beeInfo[day].trajectoryCount[IDNumber]++;
    }
}

void DataProcessWindow::getGroupBeePatternRation_behavior(QVector<trackPro> &data, QVector<QStringList> &infoID, QVector<QVector<double> > &infoRatio)
{
    QVector< QVector<double> > group(infoID.size());

    for(int i = 0; i < infoID.size();i++)
    {
        group[i].resize(PATTERN_TYPES_BEHAVIOR);

    }
    for(int i = 0; i < data.size();i++)
    {
        int gID;
        for(int j = 0; j < infoID.size();j++)
        {
            if(infoID[j].contains(QString(data[i].ID[0])))
            {
                gID = j;
                break;
            }
        }
        for(int j = 0; j < PATTERN_TYPES_BEHAVIOR; j++)
            group[gID][j] += data[i].getPatternCount_behavior()[j];
    }

    QVector<double> sum(infoID.size());
    for(int i = 0; i < infoID.size(); i++)
    {
        for(int j = 0; j < PATTERN_TYPES_BEHAVIOR;j++)
        {
            sum[i] += group[i][j];

        }
    }
    for(int i = 0; i < infoID.size(); i++)
    {
        for(int j = 0; j < PATTERN_TYPES_BEHAVIOR;j++)
        {
            group[i][j] /= sum[i];
        }
    }
    infoRatio = group;
}

void DataProcessWindow::getIndividualBeePatternRatio_behavior(QVector<trackPro> &data, QStringList &individualInfoID, QVector<QVector<double> > &individualInfoRatio)
{
    //individual bee pattern count and ratio
    for(int i = 0; i < data.size();i++)
    {
        QString ID = data.at(i).ID;
        QVector<double> count = data[i].getPatternCount_behavior();

        int index = individualInfoID.indexOf(ID);
        if(index == -1)
        {
            individualInfoID.append(ID);
            individualInfoRatio.append(count);
        }
        else
        {
            for(int j = 0; j < count.size(); j++)
            {
                individualInfoRatio[index][j] += count[j];
            }
        }
    }
    //sort
    for(int i = 0; i < individualInfoID.size()-1; i++)
    {
        for(int j = i; j < individualInfoID.size(); j++)
        {
            if(individualInfoID[i][0]>individualInfoID[j][0])
            {
                individualInfoID.swap(i,j);
                QVector<double> temp = individualInfoRatio[i];
                individualInfoRatio[i] = individualInfoRatio[j];
                individualInfoRatio[j] = temp;
            }
            else if(individualInfoID[i][0]==individualInfoID[j][0])
            {
                if(individualInfoID[i][1]>individualInfoID[j][1])
                {
                    individualInfoID.swap(i,j);
                    QVector<double> temp = individualInfoRatio[i];
                    individualInfoRatio[i] = individualInfoRatio[j];
                    individualInfoRatio[j] = temp;
                }
            }
        }
    }
    //get ratio
    for(int i = 0; i < individualInfoID.size(); i++)
    {
        double sum = 0;
        for(int j = 0; j < individualInfoRatio[i].size(); j++)
        {
            sum += individualInfoRatio[i][j];
        }
        if(sum < BEE_TRACK_COUNT)
        {
            individualInfoID.removeAt(i);
            individualInfoRatio.remove(i);
            i--;
        }
        else{
            for(int j = 0; j < individualInfoRatio[i].size(); j++)
            {
                individualInfoRatio[i][j] /= sum;
            }
        }
    }
    //remove impossible data
    for(int i = 0; i < individualInfoID.size(); i++)
    {
        if(!SUTM.contains(QString(individualInfoID[i][1])))
        {
            individualInfoID.removeAt(i);
            individualInfoRatio.remove(i);
            i--;
        }
    }
}

void DataProcessWindow::saveDailyInfoFile(const QString &fileName)
{

}

void DataProcessWindow::getTransitionMatrix(QVector<trackPro> &data, QStringList &individualInfoID, QVector<cv::Mat> &transition)
{
    //    for(int i = 0; i < data.size(); i++)
    //    {
    //        if()

    //    }
    //transition.create(PATTERN_TYPES,PATTERN_TYPES,CV_8UC1);
}

void DataProcessWindow::getDailyMotionPatternInfo(QVector<beeMotionPatternInfo> &dailyMotionInfo)
{
    //    dailyMotionInfo.clear();
    //    for(int i = 0; i < this->data.size(); i++)
    //    {
    //        bool isExist = 0;
    //        int day = 0;
    //        QString beeDate = this->data[i].startTime.toString("yyyy-MM-dd");
    //        for(int j = 0; j < dailyMotionInfo.size();j++)
    //        {
    //            if(beeDate == beeInfo[j].date)
    //            {
    //                isExist = 1;
    //                day = j;
    //                break;
    //            }
    //        }

    //        for(int j = 0; j < dailyMotionInfo.size();j++)
    //        {
    //            if(this->data[i].startTime.toString("yyyy-MM-dd") == dailyMotionInfo[j].date )
    //            {
    //                break;
    //            }
    //        }
    //    }
}

void DataProcessWindow::outBeeBehaviorInfo(QVector<trackPro> &data)
{
    QDir outInfo("out/bee_info");
    if(!outInfo.exists())
    {
        outInfo.cdUp();
        outInfo.mkdir("bee_info");
        outInfo.cd("bee_info");
    }
    QFile beeBehaviorFile(outInfo.absolutePath()+"/"+"individual_behavior.csv");
    beeBehaviorFile.open(QIODevice::ReadWrite);

    QTextStream out(&beeBehaviorFile);

    QStringList beeIDList;
    QVector< QVector<double> > beePatternRatio;
    this->getIndividualBeePatternRatio(data,beeIDList,beePatternRatio);
    QVector< QVector<double> > beeBehavior;
    for(int i = 0; i < beeIDList.size(); i++)
    {
        QVector<double> behavior(13);
        for(int j = 0; j < 6; j++)
        {
            behavior[j] = beePatternRatio[i][j];

        }
        beeBehavior.append(behavior);
    }

    for(int i = 0; i < data.size();i++)
    {
        int ID = beeIDList.indexOf(data[i].ID);
        if(ID > -1)
        {

            //trajectory count
            beeBehavior[ID][12]++;
            //velocity
            beeBehavior[ID][6] += data[i].getTrajectoryMovingVelocity();
            //distance
            beeBehavior[ID][7] += data[i].getTrajectoryMovingDistance();
            //static
            beeBehavior[ID][8] += data[i].getStaticTime();
            //loitering
            beeBehavior[ID][9] += data[i].getLoiteringTime();
            //moving
            beeBehavior[ID][10] += data[i].getMovingTime();
            //time
            beeBehavior[ID][11] += data[i].getDetectedTime();
        }
        else
        {
            //            qDebug() << data[i].ID;
        }
    }

    for(int i = 0; i < beeBehavior.size(); i++)
    {
        //velocity
        beeBehavior[i][6] /= beeBehavior[i][12];
        //distance
        beeBehavior[i][7] /= beeBehavior[i][12];
        //static
        beeBehavior[i][8] /= beeBehavior[i][12];
        //loitering
        beeBehavior[i][9] /= beeBehavior[i][12];
        //moving
        beeBehavior[i][10] /= beeBehavior[i][12];
        //time
        beeBehavior[i][11] /= beeBehavior[i][12];
    }

    out << "ID,Group,MotionRatioStatic,MotionRatioLoitering,MotionRatioMovingForward,MotionRatioMovingCW,MotionRatioMovingCCW,Waggle,"
        << "Velocity,Distance,AvgLoiteringTime,AvgStaticTime,AvgMovingTime,DetectedTime,TrajectoryCount";
    out << "\n";

    for(int i = 0; i < beeIDList.size(); i++)
    {
        out << beeIDList[i] << ",";
        int group = controlWhiteList.indexOf(QString(beeIDList[i][0]));
        if(group > -1)
            out << 0 << ",";
        else
        {
            if(group = experimentWhiteList.indexOf(QString(beeIDList[i][0])) >-1)
                out << 1 << ",";
            else

                out << -1 << ",";
        }
        for(int j = 0; j < beeBehavior[i].size();j++)
        {
            out << beeBehavior[i][j];
            if(j!=beeBehavior[i].size()-1)
                out << ",";
        }
        if(i!=beeIDList.size()-1)
            out << "\n";
    }

    beeBehaviorFile.close();

}

void DataProcessWindow::loadColorTable(const QString &fileName, QVector<cv::Scalar> &colorTable)
{
    colorTable.clear();

    QFile file;
    file.setFileName(fileName);
    if(!file.exists())
    {
        emit sendSystemLog("color table load failed!");
        return;
    }
    else
        emit sendSystemLog("color table load success!");

    file.open(QIODevice::ReadOnly);

    while(!file.atEnd())
    {
        QString line = file.readLine().trimmed();
        QStringList str = line.split(',');
        //        qDebug() << str;
        if(str.size() > 2){
            cv::Scalar color = cv::Scalar(str[0].toInt(),str[1].toInt(),str[2].toInt());
            colorTable.append(color);
        }
    }

}

QVector<double> DataProcessWindow::movingAVG(const QVector<double> &raw, const int &length)
{
    QVector<double> weigth;
    for(int i = 0; i < length;i++)
    {
        double val = 1.0/(double)length;
        weigth.append(val);
    }
    QVector<double> out;
    for(int i = 0; i < raw.size()-length+1; i++)
    {
        double sum = 0;
        for(int j = 0; j < length; j++)
        {
            sum += raw[i+j]*weigth[j];
        }
        //sum /= length;
        out.append(sum);
    }
    return out;
}

QVector<int> DataProcessWindow::trajectoryClassifier(const QVector<double> &raw, QVector<double> &distanceP2P_AVG, const double &staticThreshold, const double &loiteringThreshold)
{
    distanceP2P_AVG = this->movingAVG(raw,24);

    QVector<int> out;
    for(int i = 0; i < raw.size()+1;i++)
    {
        if(i < distanceP2P_AVG.size())
        {
            if(distanceP2P_AVG[i] < staticThreshold)
            {
                out.append(0);
            }
            else if(distanceP2P_AVG[i] >= staticThreshold && distanceP2P_AVG[i] < loiteringThreshold)
            {
                out.append(1);
            }
            else if(distanceP2P_AVG[i] >= loiteringThreshold)
            {
                out.append(2);
            }
        }
        else
        {
            out.append(out[out.size()-1]);
        }
    }


    for(int i = 2; i < out.size()-3;i++)
    {
        QVector<int> vote(3);
        for(int j = -2; j < 3;j++)
        {
            //            qDebug() << out[i+j];
            vote[out[i+j]]++;
        }
        //        qDebug() << vote;
        int winner = -1;
        int max = 0;
        for(int k = 0; k < vote.size();k++)
        {

            if(vote[k] > max)
            {
                max = vote[k];
                winner = k;
            }
        }

        out[i] = winner;
    }

    return out;
}

void DataProcessWindow::OtsuMultiLevel(double &T1, double &T2, const std::vector<double> &val)
{
    int bins = 256;

    //total number of pixels
    int N = val.size();


    //find max
    double maxVal = 10;

    //accumulate image histogram and total number of pixels
    std::vector<double> histogram(bins);
    double deltaVal = maxVal/(double)bins;
    for(int k = 0; k < val.size(); k++)
    {
        histogram[(int)(val[k]/deltaVal)]++;
    }

    double W0K, W1K, W2K, M0, M1, M2, currVarB, maxBetweenVar, M0K, M1K, M2K, MT;

    T1 = 0;
    T2 = 0;

    W0K = 0;
    W1K = 0;

    M0K = 0;
    M1K = 0;

    MT = 0;
    maxBetweenVar = 0;
    for (int k = 0; k < bins; k++) {
        MT += k * (histogram[k] / (double) N);
    }


    for (int t1 = 0; t1 < bins-2; t1++) {
        W0K += histogram[t1] / (double) N; //Pi
        M0K += t1 * (histogram[t1] / (double) N); //i * Pi
        M0 = M0K / W0K; //(i * Pi)/Pi

        W1K = 0;
        M1K = 0;

        for (int t2 = t1 + 1; t2 < bins-1; t2++) {
            W1K += histogram[t2] / (double) N; //Pi
            M1K += t2 * (histogram[t2] / (double) N); //i * Pi
            M1 = M1K / W1K; //(i * Pi)/Pi

            W2K = 1 - (W0K + W1K);
            M2K = MT - (M0K + M1K);

            if (W2K <= 0) break;

            M2 = M2K / W2K;

            currVarB = W0K * (M0 - MT) * (M0 - MT) + W1K * (M1 - MT) * (M1 - MT) + W2K * (M2 - MT) * (M2 - MT);

            if (maxBetweenVar < currVarB) {
                maxBetweenVar = currVarB;
                T1 = t1;
                T2 = t2;
            }
        }
    }


}

double DataProcessWindow::meanVarStd(const std::vector<double> &val, double &mean, double &std)
{
    mean = 0;
    for(int m = 0; m < val.size();m++)
    {
        mean += val[m]/(double)val.size();
    }
    std = 0;
    for(int n = 0; n < val.size();n++)
    {
        std += pow(val[n]-mean,2);
    }
    std /= (double)val.size();

    double var = std;
    std = sqrt(std);

    return var;
}

void DataProcessWindow::on_actionWhite_List_triggered()
{
    WL->show();
}

void DataProcessWindow::on_mdl_pushButton_clicked()
{
    for(int i = 0; i < this->data.size(); i++)
    {
        mdl mdlProcess(&this->data[i],500);
        qDebug() << this->data[i].ID << i << this->data.size() << this->data[i].size << mdlProcess.getCriticalPoint();
        cv::Mat src = cv::Mat::zeros(1600,3600,CV_8UC3);
        cv::Mat dstTrajectory = this->data[i].getTrajectoryPlot(src);
        cv::Mat dstCritical = this->data[i].getCriticalPointPlot(dstTrajectory);
        cv::resize(dstCritical,dstCritical,cv::Size(1800,800));
        cv::imshow("dst",dstCritical);
        cv::waitKey(1000);
    }

}

void DataProcessWindow::on_white_list_smoothing_pushButton_clicked()
{
    //if without setting whitelist
    if((this->controlWhiteList+this->experimentWhiteList).isEmpty())
        WL->exec();
    if((this->controlWhiteList+this->experimentWhiteList).isEmpty())
        return;

    //set button
    ui->white_list_smoothing_pushButton->setEnabled(false);

    //white list filter
    emit sendSystemLog("before white list filter : "+QString::number(this->data.size()));
    QStringList whiteList = this->controlWhiteList+this->experimentWhiteList;
    OT->tracjectoryWhiteList(this->data,whiteList);
    emit sendSystemLog("after white list filter : "+QString::number(this->data.size()));

    //get daily info
    QVector<beeDailyInfo> beeInfo;
    getDailyInfo(beeInfo);

    //get outlier list
    QStringList outlierDate;
    QVector< QStringList > outlierList;
    for(int i = 0; i < beeInfo.size(); i++)
    {
        outlierDate.append(beeInfo[i].date);
        QStringList outlier;
        for(int j = 0; j < beeInfo[i].trajectoryCount.size();j++)
        {
            if(beeInfo[i].trajectoryCount[j] < 5)
                outlier.append(beeInfo[i].IDList[j]);
        }
        outlierList.append(outlier);

    }
    //remove some outlier
    for(int i = 0; i < this->data.size(); i++)
    {
        int whichDay;
        for(int j = 0; j < outlierDate.size(); j++)
        {
            if(outlierDate[j] == this->data[i].startTime.toString("yyyy-MM-dd"))
            {
                whichDay = j;
                break;
            }
        }
        for(int j = 0; j < outlierList[whichDay].size(); j++)
        {
            if(this->data[i].ID == outlierList[whichDay][j])
            {
                this->data.remove(i);
                i--;
                break;
            }
        }
    }

    emit sendSystemLog("after remove outlier : "+QString::number(this->data.size()));

    //remove wrong day data
    if(ui->day_selected_checkBox->isChecked())
    {
        QDateTime controlStratDate = ui->control_start_dateEdit->dateTime();
        QDateTime experimentStratDate = ui->experiment_start_dateEdit->dateTime();
        for(int i = 0; i < this->data.size();i++)
        {
            if(this->controlWhiteList.indexOf(QString(this->data[i].ID[0])) > -1)
            {
                if(this->data[i].startTime.daysTo(controlStratDate) > 0)
                {
                    this->data.remove(i);
                    i--;
                    //                break;
                }
            }
            else if(this->experimentWhiteList.indexOf(QString(this->data[i].ID[0])) > -1)
            {
                if(this->data[i].startTime.daysTo(experimentStratDate) > 0)
                {
                    this->data.remove(i);
                    i--;
                    //                break;
                }
            }
        }
        emit sendSystemLog("after remove wrong day data : "+QString::number(this->data.size()));
    }

    //get daily info
    getDailyInfo(beeInfo);

    //plot chart
    this->plotBeeInfo(this->data);

    //save daily info
    QDir outInfo("out/bee_info");
    if(!outInfo.exists())
    {
        outInfo.cdUp();
        outInfo.mkdir("bee_info");
        outInfo.cd("bee_info");
    }
    QFile dailyNumberFile(outInfo.absolutePath()+"/dailyInfo.csv");
    dailyNumberFile.open(QIODevice::WriteOnly);
    QTextStream out(&dailyNumberFile);
    for(int i = 0; i < beeInfo.size(); i++)
    {
        out << beeInfo[i].date << "\n";
        for(int j = 0; j < beeInfo[i].IDList.size();j++)
        {
            out << beeInfo[i].IDList[j];
            if(j != beeInfo[i].IDList.size()-1)
                out << ",";
        }
        out << "\n";
        for(int j = 0; j < beeInfo[i].IDList.size();j++)
        {
            out << beeInfo[i].trajectoryCount[j];
            if(j != beeInfo[i].trajectoryCount.size()-1)
                out << ",";
        }
        out << "\n";
    }
    dailyNumberFile.close();
    emit sendSystemLog("Daily Info File Saved!");

    //set button
    ui->trajectory_classify_pushButton->setEnabled(true);
    ui->distributed_area_pushButton->setEnabled(true);
    ui->mdl_pushButton->setEnabled(true);
    ui->test_pushButton->setEnabled(true);
    ui->sub_group_distributed_area_pushButton->setEnabled(true);
    //    ui->test2_pushButton->setEnabled(true);
    ui->trajectory_behavior_classifier_pushButton->setEnabled(true);
    ui->interaction_matrix_pushButton->setEnabled(true);



}

void DataProcessWindow::on_test_pushButton_clicked()
{
    QFile file;
    file.setFileName("trajectory_distance2.csv");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);

    QDateTime date = this->data[0].startTime;;
    for(int i = 0; i < this->data.size(); i++)
    {
        for(int j = 0; j < this->data[i].distanceP2P.size();j++)
        {
            out << this->data[i].distanceP2P[j] << ",";
            //        if(this->data[i].)
        }
        if(date.date() != this->data[i].startTime.date())
        {
            date = this->data[i].startTime;
            out << "\n";
        }


    }
    file.close();

    date = this->data[0].startTime;
    //    QDateTime date = this->data[0].startTime;
    int i = 0;
    while(1){



        std::vector<double> val;

        //pick up data day by day
        while(date.date() == this->data[i].startTime.date())
        {


            for(int j = 0 ; j < this->data[i].distanceP2P_AVG.size(); j++)
                val.push_back(this->data[i].distanceP2P_AVG[j]);

            i++;

            if(i>=this->data.size())
                break;
        }

        //        qDebug() << "check1";
        //calculate mean and std
        double mean,std,var;
        var = meanVarStd(val,mean,std);

        //remove extreme value
        double upperLimit = mean+3*var;
        double lowerLimit = mean-3*var;

        for(int k = 0; k < val.size(); k++)
        {
            //            qDebug() << "kick" << k;
            if(val[k] > upperLimit || val[k] < lowerLimit)
            {
                val.erase(val.begin()+k);
                k--;
            }

        }

        //find max
        double maxVal = 0;
        for(int k = 0; k < val.size(); k++)
        {
            if(val[k] > maxVal)
                maxVal = val[k];
        }

        //do histogram
        int bins = 255;
        std::vector<double> histogram(bins);
        double deltaVal = maxVal/(double)bins;
        for(int k = 0; k < val.size(); k++)
        {
            histogram[(int)(val[k]/deltaVal)]++;
        }


        //Otsu's multi-level threshold
        double OtsuVar;
        int T1Result = 0;
        int T2Result = 0;
        for(int T1 = 0; T1 < bins-2;T1++)
        {
            for(int T2 = T1+2; T2 < bins-1;T2++)
            {
                std::vector<double> T1Val(histogram.begin(),histogram.begin()+T1);
                std::vector<double> T2Val(histogram.begin()+T1+1,histogram.begin()+T2);
                std::vector<double> T3Val(histogram.begin()+T2+1,histogram.end());

                double P1 = 0;
                for(int k = 0;k < T1;k++)
                {
                    P1 += histogram[k]/val.size();
                }


                double P2 = 0;
                for(int k = T1;k < T2-T1;k++)
                {
                    P2 += histogram[k]/val.size();
                }


                double P3 = 0;
                for(int k = T2;k < bins-T2;k++)
                {
                    P3 += histogram[k]/val.size();
                }


                double m,s;
                double nowOtsuVal = (T1-0)/255.0*meanVarStd(T1Val,m,s)+(T2-T1)/255.0*meanVarStd(T2Val,m,s)+(255-T2)/255.0*meanVarStd(T3Val,m,s);
                if(T1 == 0 && T2 == 1)
                {
                    OtsuVar = nowOtsuVal;
                    T1Result = T1;
                    T2Result = T2;
                }

                if(OtsuVar < nowOtsuVal)
                {
                    OtsuVar = nowOtsuVal;
                    T1Result = T1;
                    T2Result = T2;
                }

            }
        }
        double T1,T2;
        this->OtsuMultiLevel(T1,T2,val);
        qDebug() << 10.0*((double)T1/255.0) << 10.0*((double)T2/255.0);
        if(i>=this->data.size())
            break;

        date = this->data[i].startTime;

    }
}

void DataProcessWindow::on_distributed_area_pushButton_clicked()
{
    //check data is exist or not
    if(!data.isEmpty())
    {
        //for saving raw image and trajectory image
        cv::Mat srcControl;
        srcControl = srcControl.zeros(IMAGE_SIZE_Y,IMAGE_SIZE_X*3,CV_8UC1);
        cv::Mat srcExperiment;
        srcExperiment = srcExperiment.zeros(IMAGE_SIZE_Y,IMAGE_SIZE_X*3,CV_8UC1);
        cv::Mat traControl;
        traControl = traControl.zeros(IMAGE_SIZE_Y,IMAGE_SIZE_X*3,CV_8UC3);
        cv::Mat traExperiment;
        traExperiment = traExperiment.zeros(IMAGE_SIZE_Y,IMAGE_SIZE_X*3,CV_8UC3);

        //trajectory count
        double controlCount = 0;
        double experimentCount = 0;

        //scan each trajectory
        for(int i = 0;i < this->data.size();i++)
        {
            //get two ID character (ascii word)
            char firstChr = (char)this->data[i].ID.toStdString()[0]-65;
            char lastChr = (char)this->data[i].ID.toStdString()[1]-65;
            //choose color form color table
            cv::Scalar lineColor = this->colorTable[firstChr*17+lastChr];

            //if tag ID in control group white list
            if(this->controlWhiteList.indexOf(this->data[i].ID.at(0)) > -1)
            {
                for(int j = 0;j < this->data[i].position.size(); j++)
                {
                    //
                    if(this->data[i].position[j].x >= 3600 ||this->data[i].position[j].y >= 1600)
                    {
                        //qDebug() << j<< this->data[i].position[j].x << this->data[i].position[j].y;

                    }
                    else
                    {
                        controlCount++;
                        //                        if(srcControl.at<uchar>(data[i].position[j].y,data[i].position[j].x) < 256-cVal)
                        srcControl.at<uchar>(this->data[i].position[j]) += cVal;
                        if(j < this->data[i].position.size()-1)
                            cv::line(traControl,this->data[i].position[j],this->data[i].position[j+1],lineColor,3);
                    }
                }
            }
            else if(this->experimentWhiteList.indexOf(this->data[i].ID.at(0)) > -1)
            {

                //qDebug() << "experiment "<< data[i].ID;
                for(int j = 0;j < this->data[i].position.size(); j++)
                {
                    if(this->data[i].position[j].x >= 3600 ||this->data[i].position[j].y >= 1600)
                    {
                        //qDebug() << j << this->data[i].position[j].x << this->data[i].position[j].y;
                    }
                    else
                    {
                        experimentCount++;
                        //                        if(srcExperiment.at<uchar>(data[i].position[j].y,data[i].position[j].x) < 256-cVal)
                        srcExperiment.at<uchar>(this->data[i].position[j])+= cVal;
                        if(j < this->data[i].position.size()-1)
                            cv::line(traExperiment,this->data[i].position[j],this->data[i].position[j+1],lineColor,3);
                    }
                }
            }
        }
        //        //        cv::normalize(srcControl,srcControl,0,255,cv::NORM_MINMAX);
        cv::resize(srcControl,srcControl,cv::Size(srcControl.cols/2,srcControl.rows/2));
        //        //        cv::normalize(srcExperiment,srcExperiment,0,255,cv::NORM_MINMAX);
        cv::resize(srcExperiment,srcExperiment,cv::Size(srcExperiment.cols/2,srcExperiment.rows/2));


        cv::Mat dstControl,dstExperiment;
        cv::applyColorMap(srcControl,dstControl,cv::COLORMAP_JET);
        cv::applyColorMap(srcExperiment,dstExperiment,cv::COLORMAP_JET);

        cv::imshow("dstControl",dstControl);
        cv::imshow("dstExperiment",dstExperiment);
        QDir outInfo("out/bee_info");
        if(!outInfo.exists())
        {
            outInfo.cdUp();
            outInfo.mkdir("bee_info");
            outInfo.cd("bee_info");
        }
        cv::String dstName;
        dstName = (outInfo.absolutePath()+"/dstControl.jpg").toStdString();
        cv::imwrite(dstName,dstControl);
        dstName = (outInfo.absolutePath()+"/dstExperiment.jpg").toStdString();
        cv::imwrite(dstName,dstExperiment);

        dstName = (outInfo.absolutePath()+"/traControl.jpg").toStdString();
        cv::imwrite(dstName,traControl);
        dstName = (outInfo.absolutePath()+"/traExperiment.jpg").toStdString();
        cv::imwrite(dstName,traExperiment);

    }
}

void DataProcessWindow::on_c_value_spinBox_valueChanged(int arg1)
{
    cVal = arg1;
}

void DataProcessWindow::on_motion_pattern_filterpushButton_clicked()
{
    //motion pattern filter

    //check file exist or not
    QFile inFile;
    inFile.setFileName("out/bee_info/motion_pattern_count.csv");
    inFile.open(QIODevice::ReadOnly);
    if(!inFile.exists())
    {
        emit sendSystemLog(inFile.fileName()+" not exist!");
        return;
    }

    //open new file
    QFile outFile;
    outFile.setFileName("out/bee_info/motion_pattern_count_filter.csv");
    outFile.open(QIODevice::WriteOnly);


    QVector<double> sumList;
    sumList.clear();
    QVector<double> SRList;
    SRList.clear();
    QStringList strList;
    strList.clear();

    int last = 2;
    while(!inFile.atEnd()||last==1)
    {
        QString str = inFile.readLine();
        QStringList data = str.trimmed().split(',');
        if(data.size() > 1)
        {

            data.removeFirst();
            QVector<int> val(data.size());
            for(int i = 0;i < val.size();i++)
            {
                val[i] = data[i].toInt();
            }

            //sum
            double sum = 0;
            for(int i = 0; i < val.size();i++)
            {
                sum+=(double)val[i]/12.0/60.0/60.0;
            }

            //static motion ratio
            double SR = (double)val[0]/sum;
            sumList.append(sum);
            SRList.append(SR);
            strList.append(str);

        }
        else
        {

            QVectorIterator<double> isumList(sumList);
            double sum = 0;
            while(isumList.hasNext()){
                sum += isumList.next();
            }

            double mean = sum/ (double)sumList.count();
            double var = 0;
            for(int i = 0;i < sumList.count();i++)
            {
                var += (mean-sumList[i])*(mean-sumList[i]);

            }
            var /= (double)sumList.count();

            for(int i = 0;i < sumList.count();i++)
            {
                bool c1 = (sumList[i] > mean+2*var);
                bool c2 = (sumList[i] < mean-2*var);
                if(c1+c2)
                {
                    qDebug() << sumList[i] << mean+2*var << mean-2*var;
                    sumList.remove(i);
                    SRList.remove(i);
                    strList.removeAt(i);
                    i--;
                }
            }
            qDebug() << strList.count();
            for(int i = 0;i < strList.count();i++)
            {
                outFile.write(strList[i].toStdString().c_str());
            }
            outFile.write(str.toStdString().c_str());
            sumList.clear();
            SRList.clear();
            strList.clear();

        }
        if(last == 1)
        {
            break;
            qDebug() << str << str.size();
        }
        if(inFile.atEnd())
        {
            last = 1;
        }

    }
    inFile.close();
    outFile.close();
}

void DataProcessWindow::on_actionOpen_In_Out_Data_triggered()
{

    //load in-out data
    QStringList fileNames = QFileDialog::getOpenFileNames(this,"Select one or more files to open","","Text Files (*.txt)");

    if(fileNames.isEmpty())
        return;
    inOutData.clear();
    for(int i = 0; i < fileNames.size(); i++)
    {
        QFile file;
        file.setFileName(fileNames[i]);
        file.open(QIODevice::ReadOnly);
        emit sendSystemLog(fileNames[i]);

        while(!file.atEnd())
        {

            QString dateStr = file.readLine().trimmed();
            QString IDDirStr = file.readLine().trimmed();

            inOutInfo info;
            info.time = QDateTime::fromString(dateStr,"yyyy-MM-dd hh:mm:ss");
            QStringList IDDir = IDDirStr.split(' ');
            info.ID = IDDir[0];
            info.direction = 0;
            if(IDDir[2] == "in")
                info.direction = 1;

            inOutData.append(info);
        }

    }

    //print in-out data
    QVector<inOutDailyInfo> dailyInfo;
    for(int i = 0; i < inOutData.size(); i++)
    {
        if((this->controlWhiteList.indexOf(QString(inOutData[i].ID[0])) > -1 && SUTM.contains(QString(inOutData[i].ID[1])))
                || (this->experimentWhiteList.indexOf(QString(inOutData[i].ID[0])) > -1 && SUTM.contains(QString(inOutData[i].ID[1]))))
        {

            bool inDate = 0;
            int dateID = 0;
            for(int j = 0; j< dailyInfo.size(); j++)
            {
                if(inOutData[i].time.date() == dailyInfo[j].time)
                {
                    inDate = 1;
                    dateID = j;
                    break;
                }
            }

            if(!inDate)
            {
                inOutDailyInfo dInfo;
                dInfo.time = inOutData[i].time.date();
                dailyInfo.append(dInfo);
                inDate = 1;
                dateID = dailyInfo.size()-1;
            }

            bool inID = 0;
            int IDID = 0;
            for(int j = 0; j < dailyInfo[dateID].ID.size(); j++)
            {
                if(inOutData[i].ID == dailyInfo[dateID].ID[j])
                {
                    inID = 1;
                    IDID = j;
                    break;
                }
            }
            if(!inID)
            {
                dailyInfo[dateID].ID.append(inOutData[i].ID);
                dailyInfo[dateID].count.append(0);
                inID = 1;
                IDID = dailyInfo[dateID].count.size()-1;

            }
            dailyInfo[dateID].count[IDID]++;
        }
        else
        {
            inOutData.remove(i);
            i--;
        }
    }

    QString msg;
    QTextStream out(&msg);
    for(int i = 0; i< dailyInfo.size(); i++)
    {
        qDebug() << dailyInfo[i].time.toString("yyyy-MM-dd");
        out << dailyInfo[i].time.toString("yyyy-MM-dd") << "\n";
        for(int j = 0; j < dailyInfo[i].ID.size(); j++)
        {
            out << dailyInfo[i].ID[j] << "\t" << dailyInfo[i].count[j] << "\n";
        }
    }




    inOutIDList.clear();
    for(int i = 0; i < inOutData.size(); i++)
    {
        if(!inOutIDList.contains(inOutData[i].ID))
            inOutIDList.append(inOutData[i].ID);
    }

    for(int i = 0; i < inOutIDList.size()-1; i++)
    {
        for(int j = i; j < inOutIDList.size(); j++)
        {
            if(inOutIDList[i] >= inOutIDList[j])
            {
                QString temp = inOutIDList[i];
                inOutIDList[i] = inOutIDList[j];
                inOutIDList[j] = temp;
            }
        }
    }
    for(int i = 0; i < inOutIDList.size(); i++)
    {
        out << "'" << inOutIDList[i] << "'" << ";";
    }

    ui->in_out_info_textBrowser->append(msg);
}


void frameDetected::getInteractions(QVector<int> &interaction, const double &distanceThreshold)
{
    //calculate highly interaction honeybee

    interaction.clear();
    for(int m = 0; m < this->IDList.size()-1; m++)
    {
        for(int n = m+1; n < this->IDList.size(); n++)
        {
            double distance = sqrt(pow(this->positionList[m].x-this->positionList[n].x,2)+pow(this->positionList[m].y-this->positionList[n].y,2));
            if(distance < distanceThreshold)
            {
                interaction.append(m);
                interaction.append(n);
            }
        }
    }
    this->interactionID = interaction;

}

void DataProcessWindow::on_sub_group_distributed_area_pushButton_clicked()
{
    if(!data.isEmpty())
    {
        QString targetStr = ui->target_list_lineEdit->text();
        targetStr.replace("'","");


        QStringList targetList;
        targetList = targetStr.split(";");
        qDebug() << targetList;
        //        targetList  << "AA" << "AC" << "AG" << "AL" << "AP" << "AR" << "AT" << "AV" << "BB" << "BF"
        //                    << "BH" << "BO" << "BP" << "BP" << "BR" << "BT" << "BY" << "BZ" << "CF" << "CH"
        //                    << "CR" << "CP" << "CT" << "CG" << "CZ" << "EB" << "EG" << "DA" << "FB" << "FE"
        //                    << "FF" << "FC" << "FG" << "FH" << "FL" << "FK" << "FR" << "FT" << "FY" << "GA"
        //                    << "GB" << "GC" << "GF" << "GK" << "GP" << "GO" << "GT" << "GZ";

        cv::Mat srcControl;
        srcControl = srcControl.zeros(IMAGE_SIZE_Y,IMAGE_SIZE_X*3,CV_8UC1);
        cv::Mat srcExperiment;
        srcExperiment = srcExperiment.zeros(IMAGE_SIZE_Y,IMAGE_SIZE_X*3,CV_8UC1);
        cv::Mat traControl;
        traControl = traControl.zeros(IMAGE_SIZE_Y,IMAGE_SIZE_X*3,CV_8UC3);
        cv::Mat traExperiment;
        traExperiment = traExperiment.zeros(IMAGE_SIZE_Y,IMAGE_SIZE_X*3,CV_8UC3);

        double controlCount = 0;
        double experimentCount = 0;

        for(int i = 0;i < this->data.size();i++)
        {
            char firstChr = (char)this->data[i].ID.toStdString()[0]-65;
            char lastChr = (char)this->data[i].ID.toStdString()[1]-65;
            cv::Scalar lineColor = this->colorTable[firstChr*17+lastChr];
            if(targetList.indexOf(this->data[i].ID) > -1)
            {

                for(int j = 0;j < this->data[i].position.size(); j++)
                {
                    if(this->data[i].position[j].x >= 3600 ||this->data[i].position[j].y >= 1600)
                    {

                    }
                    else
                    {
                        controlCount++;
                        srcControl.at<uchar>(this->data[i].position[j]) += cVal;
                        if(j < this->data[i].position.size()-1)
                            cv::line(traControl,this->data[i].position[j],this->data[i].position[j+1],lineColor,3);
                    }
                }
            }
            else
            {

                for(int j = 0;j < this->data[i].position.size(); j++)
                {
                    if(this->data[i].position[j].x >= 3600 ||this->data[i].position[j].y >= 1600)
                    {
                        //qDebug() << j << this->data[i].position[j].x << this->data[i].position[j].y;
                    }
                    else
                    {
                        experimentCount++;
                        //                        if(srcExperiment.at<uchar>(data[i].position[j].y,data[i].position[j].x) < 256-cVal)
                        srcExperiment.at<uchar>(this->data[i].position[j])+= cVal;
                        if(j < this->data[i].position.size()-1)
                            cv::line(traExperiment,this->data[i].position[j],this->data[i].position[j+1],lineColor,3);
                    }
                }
            }
        }
        cv::resize(srcControl,srcControl,cv::Size(srcControl.cols/2,srcControl.rows/2));
        cv::resize(srcExperiment,srcExperiment,cv::Size(srcExperiment.cols/2,srcExperiment.rows/2));


        cv::Mat dstControl,dstExperiment;
        cv::applyColorMap(srcControl,dstControl,cv::COLORMAP_JET);
        cv::applyColorMap(srcExperiment,dstExperiment,cv::COLORMAP_JET);

        cv::imshow("dstControl",dstControl);
        cv::imshow("dstExperiment",dstExperiment);
        QDir outInfo("out/bee_info");
        if(!outInfo.exists())
        {
            outInfo.cdUp();
            outInfo.mkdir("bee_info");
            outInfo.cd("bee_info");
        }
        cv::String dstName;
        dstName = (outInfo.absolutePath()+"/dstControl.jpg").toStdString();
        cv::imwrite(dstName,dstControl);
        dstName = (outInfo.absolutePath()+"/dstExperiment.jpg").toStdString();
        cv::imwrite(dstName,dstExperiment);

        dstName = (outInfo.absolutePath()+"/traControl.jpg").toStdString();
        cv::imwrite(dstName,traControl);
        dstName = (outInfo.absolutePath()+"/traExperiment.jpg").toStdString();
        cv::imwrite(dstName,traExperiment);

    }
}

void DataProcessWindow::on_actionDaily_Infomation_triggered()
{
    QFile file;
    file.setFileName("out/bee_info/dailyInfo.csv");
    if(!file.exists())
    {
        emit sendSystemLog("File not found!");
        return;
    }

    QString fileName = "'../out/bee_info/dailyInfo.csv'";

    QString defaultCMD = "matlab -nodesktop -nosplash -r ";
    QString changeCMD = "cd analysis;";
    QString parameterCMD = "fileName="+fileName+";";
    QString functionCMD = "dailyInfo;";

    QString controlLegend = "controlLegendName='"+ui->control_legend_lineEdit->text()+"';";
    QString experimentLegend = "experimentLegendName='"+ui->experiment_legend_lineEdit->text()+"';";

    QString outputCMD = defaultCMD+'"'+changeCMD+parameterCMD+controlLegend+experimentLegend+functionCMD+'"';


    process->execute(outputCMD);
}

void DataProcessWindow::on_actionDaily_Trajectory_Analysis_triggered()
{
    QFile file;
    file.setFileName("out/bee_info/trajectory_info.csv");
    if(!file.exists())
    {
        emit sendSystemLog("File not found!");
        return;
    }
    if(this->controlWhiteList.isEmpty()||this->experimentWhiteList.isEmpty())
    {
        emit sendSystemLog("Whitelist no ID");
        return;
    }


    QString fileName = "'../out/bee_info/trajectory_info.csv'";

    QString defaultCMD = "matlab -nodesktop -nosplash -r ";
    QString changeCMD = "cd analysis;";
    QString parameterCMD = "fileName="+fileName+";";
    QString functionCMD = "trajectory_analysis;";

    QString controlGroupIDs = "groupAID={";
    for(int i=0; i < this->controlWhiteList.size();i++)
    {
        controlGroupIDs = controlGroupIDs+"'"+this->controlWhiteList[i]+"'";
        if(i!=this->controlWhiteList.size()-1)
            controlGroupIDs = controlGroupIDs+",";
    }
    controlGroupIDs = controlGroupIDs+"};";

    QString experimentGroupIDs = "groupBID={";
    for(int i=0; i < this->experimentWhiteList.size();i++)
    {
        experimentGroupIDs = experimentGroupIDs+"'"+this->experimentWhiteList[i]+"'";
        if(i!=this->experimentWhiteList.size()-1)
            experimentGroupIDs = experimentGroupIDs+",";
    }
    experimentGroupIDs = experimentGroupIDs+"};";

    QString controlLegend = "controlLegendName='"+ui->control_legend_lineEdit->text()+"';";
    QString experimentLegend = "experimentLegendName='"+ui->experiment_legend_lineEdit->text()+"';";

    QString outputCMD = defaultCMD+'"'+changeCMD+parameterCMD+controlLegend+experimentLegend+controlGroupIDs+experimentGroupIDs+functionCMD+'"';

    process->execute(outputCMD);
}

void DataProcessWindow::on_actionHouly_Compare_triggered()
{
    QFile file;
    file.setFileName("out/bee_info/trajectory_info.csv");
    if(!file.exists())
    {
        emit sendSystemLog("File not found!");
        return;
    }

    QString fileName = "'../out/bee_info/trajectory_info.csv'";

    QString defaultCMD = "matlab -nodesktop -nosplash -r ";
    QString changeCMD = "cd analysis;";
    QString parameterCMD = "fileName="+fileName+";";
    QString functionCMD = "hours_compare;";

    QString controlLegend = "controlLegendName='"+ui->control_legend_lineEdit->text()+"';";
    QString experimentLegend = "experimentLegendName='"+ui->experiment_legend_lineEdit->text()+"';";

    QString outputCMD = defaultCMD+'"'+changeCMD+parameterCMD+controlLegend+experimentLegend+functionCMD+'"';

    process->execute(outputCMD);

}

void DataProcessWindow::on_action2D_PCA_Plot_triggered()
{

    QFile file;
    file.setFileName("out/bee_info/individual_PCA.csv");
    if(!file.exists())
    {
        emit sendSystemLog("File not found!");
        return;
    }
    QString fileName = "'../out/bee_info/individual_PCA.csv'";

    QString defaultCMD = "matlab -nodesktop -nosplash -r ";
    QString changeCMD = "cd analysis;";
    QString parameterCMD = "fileName="+fileName+";";
    QString functionCMD = "main;";

    QString controlLegend = "controlLegendName='"+ui->control_legend_lineEdit->text()+"';";
    QString experimentLegend = "experimentLegendName='"+ui->experiment_legend_lineEdit->text()+"';";

    QString outputCMD = defaultCMD+'"'+changeCMD+parameterCMD+controlLegend+experimentLegend+functionCMD+'"';

    process->execute(outputCMD);
}

void DataProcessWindow::on_action3D_Motion_Pattern_Plot_triggered()
{
    QFile file;
    file.setFileName("out/bee_info/individual_info.csv");
    if(!file.exists())
    {
        emit sendSystemLog("File not found!");
        return;
    }
    QString fileName = "'../out/bee_info/individual_info.csv'";

    QString defaultCMD = "matlab -nodesktop -nosplash -r ";
    QString changeCMD = "cd analysis;";
    QString parameterCMD = "fileName="+fileName+";";
    QString functionCMD = "PCA_3D;";

    QString controlLegend = "controlLegendName='"+ui->control_legend_lineEdit->text()+"';";
    QString experimentLegend = "experimentLegendName='"+ui->experiment_legend_lineEdit->text()+"';";

    QString outputCMD = defaultCMD+'"'+changeCMD+parameterCMD+controlLegend+experimentLegend+functionCMD+'"';

    process->execute(outputCMD);
}

void DataProcessWindow::on_open_script_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName();
    QFile file;
    file.setFileName(fileName);
    if(!file.exists())
    {
        emit sendSystemLog("File no found!");
        return;
    }

    file.open(QIODevice::ReadOnly);
    QString script;
    script = file.readAll();
    ui->script_plainTextEdit->setPlainText(script);
    ui->script_label->setText(fileName);
}

void DataProcessWindow::on_save_script_pushButton_clicked()
{
    //    QString script = ui->script_plainTextEdit
}

void DataProcessWindow::on_interaction_matrix_pushButton_clicked()
{
    QTime t;
    t.start();
    //calculate frame by frame detected honeybee
    QVector<frameDetected> frames;

    //start time scan
    QDateTime startTime = this->data[0].startTime;
    for(int i = 1; i < this->data.size(); i++)
    {
        static int count = 0;
        if(startTime.msecsTo(this->data[i].startTime) < 0)
            startTime = this->data[i].startTime;
        else
            count++;
        if(count > 1000)
            break;
    }
    qDebug () << "1" << t.elapsed();
    QDateTime endTime = this->data[this->data.size()-1].endTime;
    double timeDelta = startTime.msecsTo(endTime);
    double frameSize = timeDelta/1000.0*12.0;
    frames.resize((int)frameSize+1);

    for(int i = 0; i < (int)frameSize+1; i++)
    {
        frames[i].time = startTime.addMSecs(1000.0/12.0*i);
    }

    int lastPosition = 0;
    for(int i = 0; i < this->data.size(); i++)
    {
        double progress = (double)i/(double)this->data.size()*100.0;
        //        if((int)progress%10 == 0)
        qDebug() << progress;


        int frameID = 0;
        int j;
        if(lastPosition-this->data[i].size < 0)
            j = 0;
        else
            j = lastPosition-this->data[i].size;

        int timeGap = this->data[i].startTime.msecsTo(frames[j].time);
        if(timeGap > 1000.0/12.0)
            j += timeGap/(1000.0/12.0);


        for(;j < frames.size()-1; j++)
        {
            int timeGapb = this->data[i].startTime.msecsTo(frames[j].time);
            int timeGapa = this->data[i].startTime.msecsTo(frames[j+1].time);
            if(abs(timeGapa) < 1000.0/12.0/2.0)
            {
                frameID = j;
                lastPosition = frameID;
                break;
            }
            else if(abs(timeGapb) < 1000.0/12.0/2.0)
            {
                frameID = j+1;
                lastPosition = frameID;
                break;
            }
            else
            {
                if(timeGapb < 0 && timeGapa > 0)
                {
                    frameID = j;
                    lastPosition = frameID;
                    break;
                }
            }

        }
        //        qDebug () << "2" << t.elapsed();
        //        qDebug() << i << frameID << this->data[i].size << frames.size() << frames[frameID].time <<endTime;
        for(int j = 0; j < this->data[i].size;j++)
        {
            if(j+frameID < frames.size())
            {
                if(!frames[frameID+j].IDList.contains(this->data[i].ID))
                {
                    frames[frameID+j].IDList.append(this->data[i].ID);
                    frames[frameID+j].positionList.append(this->data[i].position[j]);
                }
            }
        }
        //        qDebug () << "2.1" << t.elapsed();
    }

    qDebug () << "3" << t.elapsed();
    for(int i = 0; i < frames.size(); i++)
    {
        QVector<int> interaction;
        frames[i].getInteractions(interaction,100);
        //        if(interaction.size() > 0)
        //            qDebug() << interaction << frames[i].IDList;
    }

    qDebug () << "4" << t.elapsed();
    //plot interaction
    QStringList IDList;
    for(int i = 0; i < frames.size();i++)
    {
        for(int j = 0; j < frames[i].IDList.size(); j++)
        {
            if(!IDList.contains(frames[i].IDList[j]))
            {
                IDList.append(frames[i].IDList[j]);
            }
        }
    }
    qDebug () << "5" << t.elapsed();
    cv::Mat interactionMat;
    interactionMat = cv::Mat::zeros(IDList.size(),IDList.size(),CV_8UC1);

    for(int i = 0; i < frames.size();i++)
    {
        //        if(frames.size()/i*100%10 == 0)
        //                qDebug() << i <<  frames.size();

        for(int j = 0; j < frames[i].interactionID.size(); j+=2)
        {
            QString IDA = frames[i].IDList[frames[i].interactionID[j]];
            QString IDB = frames[i].IDList[frames[i].interactionID[j+1]];


            int m = IDList.indexOf(IDA);
            int n = IDList.indexOf(IDB);
            if(m>n)
            {
                int temp = m;
                m = n;
                n = temp;
            }
            if(interactionMat.at<uchar>(m,n) < 255)
                interactionMat.at<uchar>(m,n)++;
        }
    }
    //sort mat
    QStringList IDListRow = IDList;
    QStringList IDListCol = IDList;
    for(int i = 0; i < interactionMat.rows;i++)
    {
        for(int m = i+1; m < interactionMat.cols-1; m++)
        {
            for(int n = m+1; n < interactionMat.cols;n++)
            {
                int sumM = 0;
                int sumN = 0;
                for(int k = 0; k < interactionMat.cols; k++)
                {
                    sumM += interactionMat.at<uchar>(k,m);
                    sumN += interactionMat.at<uchar>(k,n);
                }
                if(sumM < sumN)
                {
                    for(int k = 0; k < interactionMat.cols; k++)
                    {
                        uchar temp = interactionMat.at<uchar>(k,m);
                        interactionMat.at<uchar>(k,m) = interactionMat.at<uchar>(k,n);
                        interactionMat.at<uchar>(k,n) = temp;
                    }
                    IDListCol.swap(m,n);
                }
            }
        }
    }

    for(int i = 0; i < interactionMat.cols;i++)
    {
        for(int m = i+1; m < interactionMat.rows-1; m++)
        {
            for(int n = m+1; n < interactionMat.rows;n++)
            {
                int sumM = 0;
                int sumN = 0;
                for(int k = 0; k < interactionMat.rows; k++)
                {
                    sumM += interactionMat.at<uchar>(m,k);
                    sumN += interactionMat.at<uchar>(n,k);
                }
                if(sumM < sumN)
                {
                    for(int k = 0; k < interactionMat.rows; k++)
                    {
                        uchar temp = interactionMat.at<uchar>(m,k);
                        interactionMat.at<uchar>(m,k) = interactionMat.at<uchar>(n,k);
                        interactionMat.at<uchar>(n,k) = temp;
                    }
                    IDListRow.swap(m,n);
                }
            }
        }
    }



    qDebug () << "6" << t.elapsed();
    int scaleSize = 20;
    cv::Mat interactionMatBig;
    cv::resize(interactionMat,interactionMatBig,cv::Size(interactionMat.cols*scaleSize,interactionMat.rows*scaleSize),0,0,cv::INTERSECT_NONE);


    cv::copyMakeBorder(interactionMatBig,interactionMatBig,scaleSize,0,scaleSize,0,cv::BORDER_CONSTANT,cv::Scalar(20));
    cv::applyColorMap(interactionMatBig,interactionMatBig,cv::COLORMAP_JET);

    if(!inOutIDList.empty())
    {
        for(int i = 0; i < IDList.size();i++)
        {
            if(inOutIDList.contains(IDListCol[i]))
            {
                cv::putText(interactionMatBig,IDListCol[i].toStdString(),cv::Point(scaleSize*(i+1),scaleSize/2),cv::FONT_HERSHEY_PLAIN,0.8,cv::Scalar(0,0,255));
            }

            else
            {
                cv::putText(interactionMatBig,IDListCol[i].toStdString(),cv::Point(scaleSize*(i+1),scaleSize/2),cv::FONT_HERSHEY_PLAIN,0.8,cv::Scalar(255,255,255));
            }

            if(inOutIDList.contains(IDListRow[i]))
            {
                cv::putText(interactionMatBig,IDListRow[i].toStdString(),cv::Point(0,scaleSize*(i+1)+scaleSize/4*3),cv::FONT_HERSHEY_PLAIN,0.8,cv::Scalar(0,0,255));
            }
            else
            {
                cv::putText(interactionMatBig,IDListRow[i].toStdString(),cv::Point(0,scaleSize*(i+1)+scaleSize/4*3),cv::FONT_HERSHEY_PLAIN,0.8,cv::Scalar(255,255,255));
            }
        }
    }
    else
    {
        for(int i = 0; i < IDList.size();i++)
        {
            cv::putText(interactionMatBig,IDListCol[i].toStdString(),cv::Point(scaleSize*(i+1),scaleSize/2),cv::FONT_HERSHEY_PLAIN,0.8,cv::Scalar(255,255,255));
            cv::putText(interactionMatBig,IDListRow[i].toStdString(),cv::Point(0,scaleSize*(i+1)+scaleSize/4*3),cv::FONT_HERSHEY_PLAIN,0.8,cv::Scalar(255,255,255));
        }
    }
    qDebug () << "7" << t.elapsed();
    cv::imshow("interaction",interactionMatBig);
    cv::imwrite("out/bee_info/interaction.jpg",interactionMatBig);

}

void DataProcessWindow::on_trajectory_behavior_classifier_pushButton_clicked()
{

    static int i = 1;
    i++;
#ifdef DEBUG_BEHAVIOR_CLASSIFIER
    while(1)
    {
        if(this->data[i].size > 200)
            break;
        i++;
        if(i >=  this->data.size()-1)
            return;
    }
#endif
#ifndef DEBUG_BEHAVIOR_CLASSIFIER
    for(int i = 0; i < this->data.size(); i++)
    {
#endif
        this->data[i].distanceP2P = this->data[i].getMovingDistanceP2P();
        this->data[i].trajectoryPattern = trajectoryClassifier(this->data[i].distanceP2P,this->data[i].distanceP2P_AVG,1.0,2.0);
#ifdef DEBUG_BEHAVIOR_CLASSIFIER
        qDebug() << this->data[i].ID << i << this->data.size() << this->data[i].size;
        qDebug() << this->data[i].distanceP2P;
        qDebug() << this->data[i].trajectoryPattern;
#endif

#ifdef DEBUG_BEHAVIOR_CLASSIFIER

        cv::Rect ROI = this->data[i].getROI();
#ifdef RECORD_BEHAVIOR_CLASSIFIER
        cv::VideoWriter out("trajectory.avi",cv::VideoWriter::fourcc('X','V','I','D'),12.0,ROI.size());
#endif
        cv::Mat src = cv::Mat::zeros(1600,3600,CV_8UC3);
        for(int j = 0; j < this->data[i].position.size();j++)
        {
            cv::Mat dstTrajectory = this->data[i].getTrajectoryPlotPart(src,1,j);
            cv::Mat show = dstTrajectory(ROI);
#ifdef RECORD_BEHAVIOR_CLASSIFIER
            out.write(show);
#endif
            cv::imshow("dst",show);
            cv::waitKey(83);
        }
#ifdef RECORD_BEHAVIOR_CLASSIFIER
        out.release();
#endif
#ifdef RECORD_BEHAVIOR_CLASSIFIER
        src = cv::Mat::zeros(1600,3600,CV_8UC3);
        for(int j = 0; j < this->data[i].trajectoryPattern.size();j++)
        {
            static bool isStart = 0;
            static bool isEnd = 0;
            static int start = 0;
            static int end = 0;
            static int nowPattern = -1;
            if(this->data[i].trajectoryPattern[j] != nowPattern)
            {
                if(isStart == 0)
                {
                    nowPattern = this->data[i].trajectoryPattern[j];
                    start = j;
                    isStart = 1;

                }
                else
                {
                    end = j-1;
                    isEnd = 1;
                }
            }
            if(isEnd)
            {
                cv::Mat subTrajectory = this->data[i].getTrajectoryPlotPart(src,start,end);
                cv::Mat show = subTrajectory(ROI);
                cv::imshow("show sub",show);
                cv::imwrite(QString::number(start).toStdString()+"_"+QString::number(nowPattern).toStdString()+".jpg",show);
                isStart = 0;
                isEnd = 0;
                j--;
            }
        }


#endif
#endif
#ifdef DEBUG_BEHAVIOR_CLASSIFIER
        qDebug() << "end";
#endif
#ifndef DEBUG_BEHAVIOR_CLASSIFIER
    }
#endif

    //pattern classification already do

    //    OT->trajectoryClassify(this->data,this->OTParams);
    //    OT->trajectoryClassify3D(this->data,this->OTParams);


    //group pattern count and ratio
    QVector<QStringList> infoID;
    infoID << this->controlWhiteList << this->experimentWhiteList;
    QVector< QVector<double> > infoRatio;
    this->getGroupBeePatternRation_behavior(this->data,infoID,infoRatio);


    //individual pattern ratio
    QStringList individualInfoID;
    QVector< QVector<double> > individualInfoRatio;
    this->getIndividualBeePatternRatio_behavior(this->data,individualInfoID,individualInfoRatio);

    //get transition matrix


    cv::Mat PCAData;
    PCAData.create(individualInfoRatio.size(),PATTERN_TYPES_BEHAVIOR,CV_64FC1);
    for(int i = 0; i < individualInfoRatio.size(); i++)
    {
        for(int j = 0; j < PATTERN_TYPES_BEHAVIOR; j++)
        {
            PCAData.at<double>(i,j) = individualInfoRatio[i][j];
        }
    }

    //plot 2D-PCA
    int dims = 2;

    cv::PCA *PCA_2D;
    PCA_2D = new cv::PCA(PCAData,cv::Mat(),cv::PCA::DATA_AS_ROW,dims);
    PCAData = PCA_2D->project(PCAData);
    cv::normalize(PCAData,PCAData,0,1,cv::NORM_MINMAX);

    //check folder exist or not
    QDir outInfo("out/bee_info");
    if(!outInfo.exists())
    {
        outInfo.cdUp();
        outInfo.mkdir("bee_info");
        outInfo.cd("bee_info");
    }
    QFile beePCAFile(outInfo.absolutePath()+"/"+"individual_PCA_behavior.csv");
    beePCAFile.open(QIODevice::ReadWrite);

    QTextStream outPCA(&beePCAFile);
    for(int i = 0; i < individualInfoRatio.size(); i++)
    {
        outPCA << individualInfoID[i] << ",";
        for(int j = 0; j < dims; j++)
        {
            outPCA << PCAData.at<double>(i,j) << ",";
        }
        outPCA << "\n";
    }
    beePCAFile.close();

    QFile beeInfoFile(outInfo.absolutePath()+"/"+"individual_info_behavior.csv");
    beeInfoFile.open(QIODevice::ReadWrite);

    QTextStream out(&beeInfoFile);

    for(int i = 0; i < individualInfoRatio.size(); i++)
    {
        out << individualInfoID[i] << ",";
        for(int j = 0; j < PATTERN_TYPES_BEHAVIOR; j++)
        {
            out << individualInfoRatio[i][j] << ",";
        }
        out << "\n";
    }
    beeInfoFile.close();

    //    QVector<beeDailyInfo> beeInfo;
    //    this->getDailyInfo(beeInfo);

    //    QFile motionPatternCountFile(outInfo.absolutePath()+"/"+"motion_pattern_count_behavior.csv");
    //    motionPatternCountFile.open(QIODevice::ReadWrite);
    //    QTextStream motionOut(&motionPatternCountFile);
    //    for(int i = 0; i < beeInfo.size();i++)
    //    {

    //        motionOut << beeInfo[i].date << "\n";
    //        for(int j = 0; j < beeInfo[i].IDList.size();j++)
    //        {
    //            motionOut << beeInfo[i].IDList[j] << ",";
    //            for(int k = 0 ; k < beeInfo[i].motionPatternCount[j].size(); k++)
    //            {
    //                motionOut << beeInfo[i].motionPatternCount[j][k];
    //                if(k != beeInfo[i].motionPatternCount[j].size()-1)
    //                    motionOut << ",";
    //            }
    //            motionOut << "\n";
    //        }
    //    }
    //    motionPatternCountFile.close();

    //get trajectory distance

    //    qDebug() << outInfo.absolutePath()+"/trajectory_distance.csv";
    QFile file;
    file.setFileName(outInfo.absolutePath()+"/trajectory_info.csv");
    file.open(QIODevice::WriteOnly);
    QTextStream outTra(&file);

    outTra << "ID,StartTime,Trajectory_distance,Trajectory_velocity,Detected_time,"
           << "Static_count,Loitering_count,Moving_forward_count,Moving_CW_count,Moving_CCW_count,Waggle_count,Count_sum,"
           << "Static_ratio,Loitering_ratio,Moving_ratio,Group\n";

    for(int i = 0; i < this->data.size(); i++)
    {
        if(this->data[i].getTrajectoryMovingVelocity() < 250 &&this->data[i].position.size() > 12)
        {
            outTra << this->data[i].ID << ",";
            outTra << this->data[i].startTime.toString("yyyy-MM-dd-hh-mm-ss") << ",";
            outTra << this->data[i].getTrajectoryMovingDistance() << ",";
            outTra << this->data[i].getTrajectoryMovingVelocity() << ",";
            outTra << this->data[i].getDetectedTime() << ",";

            QVector<double> patternCount = this->data[i].getPatternCount_behavior();
            outTra << patternCount[0] << ",";
            outTra << patternCount[1] << ",";
            outTra << patternCount[2] << ",";
            outTra << 0 << ",";
            outTra << 0 << ",";
            outTra << 0 << ",";

            double patternCountSum = patternCount[0]+patternCount[1]+patternCount[2];
            outTra << patternCountSum << ",";
            outTra << patternCount[0]/patternCountSum << ",";
            outTra << patternCount[1]/patternCountSum << ",";
            outTra << patternCount[2]/patternCountSum << ",";

            if(this->controlWhiteList.indexOf(QString(this->data[i].ID[0])) > -1)
                outTra << 0;
            else if(this->experimentWhiteList.indexOf(QString(this->data[i].ID[0])) > -1)
                outTra << 1;
            else
                outTra << -1;

            outTra << "\n";
        }
    }
    file.close();

    this->outBeeBehaviorInfo(this->data);
}





