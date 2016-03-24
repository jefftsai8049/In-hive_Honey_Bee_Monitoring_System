#include "dataprocesswindow.h"
#include "ui_dataprocesswindow.h"

QStringList SMUTM;

DataProcessWindow::DataProcessWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DataProcessWindow)
{
    ui->setupUi(this);
    OT = new object_tracking;
    OTS = new ObjectTrackingForm;
    WL = new WhiteList;

    connect(this,SIGNAL(sendSystemLog(QString)),this,SLOT(receiveSystemLog(QString)));

    connect(OT,SIGNAL(sendSystemLog(QString)),this,SLOT(receiveSystemLog(QString)));
    connect(OT,SIGNAL(sendLoadRawDataFinish()),this,SLOT(receiveLoadDataFinish()));
    connect(OT,SIGNAL(sendProgress(int)),this,SLOT(receiveProgress(int)));

    connect(OTS,SIGNAL(setObjectTrackingParameters(objectTrackingParameters)),this,SLOT(setObjectTrackingParameters(objectTrackingParameters)));

    connect(WL,SIGNAL(sendWhiteList(QStringList,QStringList)),this,SLOT(receiveWhiteList(QStringList,QStringList)));

    OTS->requestObjectTrackingParameters();
    OT->setPathSegmentSize(this->OTParams.segmentSize);

    SMUTM << "A" << "B" << "C" << "E" << "F" << "G" << "H" << "K" << "L" << "O" << "P" << "R"
          << "S" << "T" << "U" << "Y" << "Z";

}

DataProcessWindow::~DataProcessWindow()
{
    delete ui;
}


void DataProcessWindow::on_data_preprocessing_pushButton_clicked()
{
    //process raw data
    QFuture<void> rawDataProcessing = QtConcurrent::run(OT,&object_tracking::rawDataPreprocessing,&this->path,&this->data);
    rawDataProcessing.waitForFinished();
    this->path.clear();

    //set next button enable
    ui->trajectory_classify_pushButton->setEnabled(true);
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
    ui->white_list_smoothing_pushButton->setEnabled(true);


    //    //plot chart
    //    this->plotBeeInfo(this->data);
}

void DataProcessWindow::on_trajectory_classify_pushButton_clicked()
{
    //pattern classification
    OT->tracjectoryClassify(this->data,this->OTParams);

    //group pattern count and ratio
    QVector<QStringList> infoID;
    infoID << this->controlWhiteList << this->experimentWhiteList;
    QVector< QVector<double> > infoRatio;
    this->getGroupBeePatternRation(this->data,infoID,infoRatio);

    for(int i = 0; i < infoID.size(); i++)
    {
        qDebug() << infoID[i] << infoRatio[i];
    }

    //individual pattern ratio
    QStringList individualInfoID;
    QVector< QVector<double> > individualInfoRatio;
    this->getIndividualBeePatternRatio(this->data,individualInfoID,individualInfoRatio);

    //get transition matrix


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
    qDebug() << PCA_2D->eigenvalues.cols << PCA_2D->eigenvalues.rows;

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
        qDebug() << individualInfoID[i] << individualInfoRatio[i];
        out << individualInfoID[i] << ",";
        for(int j = 0; j < PATTERN_TYPES; j++)
        {
            out << individualInfoRatio[i][j] << ",";
        }
        out << "\n";
    }
    beeInfoFile.close();
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

void DataProcessWindow::getDailyBeeInfo(const QVector<trackPro> &data, QVector<QString> &x, QVector<double> &y)
{
    x.clear();
    y.clear();

    //int days = data.at(0).startTime.daysTo(data.at(data.size()-1).startTime)+1;
    //    x.resize(days);
    //    y.resize(days);

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
        if(!SMUTM.contains(QString(individualInfoID[i][1])))
        {
            individualInfoID.removeAt(i);
            individualInfoRatio.remove(i);
            i--;
        }
    }

}

void DataProcessWindow::getTransitionMatrix(QVector<trackPro> &data, QStringList &individualInfoID, QVector<cv::Mat> &transition)
{
    //    for(int i = 0; i < data.size(); i++)
    //    {
    //        if()

    //    }
    //transition.create(PATTERN_TYPES,PATTERN_TYPES,CV_8UC1);
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

    //white list filter
    emit sendSystemLog("before white list filter : "+QString::number(this->data.size()));
    QStringList whiteList = this->controlWhiteList+this->experimentWhiteList;
    OT->tracjectoryWhiteList(this->data,whiteList);
    emit sendSystemLog("after white list filter : "+QString::number(this->data.size()));

    //plot chart
    this->plotBeeInfo(this->data);

    //get daily info
    QVector<beeDailyInfo> beeInfo;
    for(int i = 0;i < this->data.size();i++)
    {
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
            beeInfo[day].trajectoryCount.append(1);
            IDNumber = beeInfo[day].IDList.size();
        }
        beeInfo[day].trajectoryCount[IDNumber]++;
    }

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
    qDebug() << outlierDate << outlierList;
    //remove some outlier
    for(int i = 0; i < this->data.size(); i++)
    {
        //toString("yyyy-MM-dd")
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
//            qDebug() << i << j;
//            qDebug() << outlierDate[whichDay];
//            qDebug() << this->data[i].ID << outlierList[whichDay][j];
            if(this->data[i].ID == outlierList[whichDay][j])
            {

                qDebug() << "kick";
                this->data.remove(i);
                i--;
                break;
            }
        }
    }

    emit sendSystemLog("after remove outlier : "+QString::number(this->data.size()));

    //get daily info
//    QVector<beeDailyInfo> beeInfo;
    beeInfo.clear();
    for(int i = 0;i < this->data.size();i++)
    {
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
            beeInfo[day].trajectoryCount.append(1);
            IDNumber = beeInfo[day].IDList.size();
        }
        beeInfo[day].trajectoryCount[IDNumber]++;
    }

    //show bee daily info
    for(int i = 0; i < beeInfo.size(); i++)
    {
        qDebug() << beeInfo[i].date;
        qDebug() << beeInfo[i].IDList;
        qDebug() << beeInfo[i].trajectoryCount;
    }

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



}
