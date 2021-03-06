//
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//               佛祖保佑         永無bug
//
//
//********************************************************************
//本程式為R03 蔡靜偉 的碩士研究 應用蜜蜂箱內影像影像系統於蜜蜂行為分析
//
//主要功能為將影片檔轉換成為蜜蜂軌跡與ID，並且分析其特徵等
//
//********************************************************************

#include "mainwindow.h"
#include "ui_mainwindow.h"

std::vector<cv::Mat> stitchFrame;

std::vector<cv::Point> originPoint;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //load icon image
    this->setWindowIcon(QIcon("icon/honeybee.jpg"));

    //open file for saving system log
    QDir outDir;
    outDir.setPath("out");
    if(!outDir.exists())
    {
        outDir.cdUp();
        outDir.mkdir("out");
        outDir.cd("out");
    }

    systemLog.setFileName(outDir.path()+"/"+"system_log.txt");
    systemLog.open(QIODevice::ReadWrite);

    //for tracking honeybee position
    TT = new trajectory_tracking;

    //for tag recognition
    TR = new tag_recognition;

    qRegisterMetaType<cv::Mat>("cv::Mat");

    //send execute speed back to mainwindow
    connect(TT,SIGNAL(sendFPS(double)),this,SLOT(receiveFPS(double)));

    //send image back to mainwindow
    connect(TT,SIGNAL(sendImage(cv::Mat)),this,SLOT(receiveShowImage(cv::Mat)));

    //send system log back to mainwindow
    connect(TT,SIGNAL(sendSystemLog(QString)),this,SLOT(receiveSystemLog(QString)));
    connect(TR,SIGNAL(sendSystemLog(QString)),this,SLOT(receiveSystemLog(QString)));

    //send processing progress to mainwindows to show on progess bar
    connect(TT,SIGNAL(sendProcessingProgress(int)),this,SLOT(receiveProcessingProgress(int)));

    //send system log to mainwindow
    connect(this,SIGNAL(sendSystemLog(QString)),this,SLOT(receiveSystemLog(QString)));

    //load model path
    QFile modelPathFile(MODEL_PATH_NAME);
    if(modelPathFile.exists())
    {
        modelPathFile.open(QIODevice::ReadOnly);
        while(!modelPathFile.atEnd())
        {
            QString msg = modelPathFile.readLine().trimmed();

            if(msg[0] == '@')
            {
                QStringList list = msg.split(':');
                if(list[0] == "@PCA model")
                {
                    //load PCA model for tag image reduce dimensions
                    QFile PCAModel(list[1]);
                    if(PCAModel.exists())
                    {
                        TT->setPCAModelFileName(PCAModel.fileName().toStdString());
                        QString msg;
                        QTextStream TS(&msg);
                        TS << "PCA Model File Name : \n" << PCAModel.fileName();
                        ui->PCA_model_name_label->setText(msg);
                    }
                    else
                        emit sendSystemLog(PCAModel.fileName()+" no found!");

                }
                else if(list[0] == "@stitching model")
                {
                    //load stitching model file
                    QFile manulStitchModel(list[1]);
                    if(manulStitchModel.exists())
                    {
                        TT->setManualStitchingFileName(manulStitchModel.fileName().toStdString());
                        QString msg;
                        QTextStream TS(&msg);
                        TS << "Stitching Model File Name : \n" << manulStitchModel.fileName();
                        ui->Stitching_model_name_label->setText(msg);
                    }
                    else
                        emit sendSystemLog(manulStitchModel.fileName()+" no found!");
                }
                else if(list[0] == "@SVM_SUTM model")
                {
                    SVMModel_SUTM.setFileName(list[1]);
                    //load SVM model
                    if(SVMModel_SUTM.exists())
                    {
                        TT->setSVMModelFileName(SVMModel_SUTM.fileName().toStdString());
                        QString msg;
                        QTextStream TS(&msg);
                        TS << "SVM Model File Name : \n" << SVMModel_SUTM.fileName();
                        ui->SVM_model_name_label->setText(msg);
                    }
                    else
                        emit sendSystemLog(SVMModel_SUTM.fileName()+" no found!");
                }
                else if(list[0] == "@SVM_MUTM model")
                {
                    SVMModel_MUTM.setFileName(list[1]);
                }
            }
        }
    }

    //set initial hough circle parameters
    TT->setHoughCircleParameters(ui->dp_hough_circle_spinBox->value(),ui->minDist_hough_circle_spinBox->value(),ui->para_1_hough_circle_spinBox->value(),ui->para_2_hough_circle_spinBox->value(),ui->minRadius_hough_circle_spinBox->value(),ui->maxRadius_hough_circle_spinBox->value());

    //set inital PCA and HOG parameters
    TT->setPCAandHOG(ui->actionWith_PCA->isChecked(),ui->actionWith_HOG->isChecked());

    //start up OpenCL for GPU speed up
#ifndef NO_OCL
    TT->initOCL();
#endif

#ifdef DEBUG_TAG_RECOGNITION
    emit sendSystemLog("Tag Recognition Debuging");
#endif

    //detect how many COM port avaliable
    this->portList = QSerialPortInfo::availablePorts();
    for(int i = 0; i < this->portList.size(); i++)
    {
        ui->port_name_comboBox->insertItem(i,this->portList.at(i).portName()+" "+this->portList.at(i).description());
    }

    //timer for receive COM port data
    serialClock = new QTimer;
    connect(serialClock,SIGNAL(timeout()),this,SLOT(receiveSerialData()));

    //timer for record sensor data
    recordClock = new QTimer;
    connect(recordClock,SIGNAL(timeout()),this,SLOT(recordSensorData()));

    //set text system
    TT->setTextSystem(ui->text_system_comboBox->currentText());

}

MainWindow::~MainWindow()
{
    systemLog.close();
    delete ui;
}


void MainWindow::on_actionLoad_Raw_Video_File_triggered()
{
    //load raw video file to process

    //check dir is exist or not
    QString dirName = QFileDialog::getExistingDirectory();

    if(dirName.isEmpty())
    {
        return;
    }

    //set dir path
    dir.setPath(dirName);
    QStringList dirList = dir.entryList(QDir::Dirs,QDir::Name);

    //set file type
    QStringList nameFilter;
    nameFilter.append("*.avi");

    //get file names in dir
    for (int j = 0;j<dirList.size()-2;j++)
    {
        dir.cd(dirList[j+2]);
        videoList.push_back(dir.entryList(nameFilter,QDir::Files,QDir::Name));
        dir.cdUp();
    }

    //show on ui
    for (int k = 0;k<videoList[0].size();k++)
    {
        QString fileName = videoList[0][k]+"\n";
        //remove first two chracters from the video file name
        fileName = fileName.mid(2,fileName.length()-2);
        ui->videoName_textBrowser->insertPlainText(fileName);
    }


}

void MainWindow::receivePano(cv::Mat pano)
{
    //receive manul stitch result and show
    cv::Mat show;
    cv::resize(pano,show,cv::Size(pano.cols/2,pano.rows/2));
    cv::imshow("Panorama",show);
}

void MainWindow::receiveFPS(const double &fpsRun)
{
    //receive processing FPS from TT and show
    ui->processing_lcdNumber->display(fpsRun);
}

void MainWindow::receiveShowImage(const cv::Mat &src)
{
    //receive processing image form Trajectory Tracjing class and show

    static bool status = 0;
    if(status == 0)
    {
        //first time use the imageShow_widget need to initialize and set the image size
        ui->imageShow_widget->initialize(1,src.cols,src.rows);
        status = 1;
    }
    ui->imageShow_widget->setImage(src);
}

void MainWindow::receiveSystemLog(const QString &msg)
{
    //receive system log from everywhere and show

    ui->system_log_textBrowser->append(msg);
    systemLog.write((msg+"\n").toStdString().c_str());
}

void MainWindow::receiveProcessingProgress(const int &percentage)
{
    //receive video processing precentage and show
    ui->processing_progressBar->setValue(percentage);
}

void MainWindow::stitchImage()
{
    //processing video

    std::vector<std::string> fileNames;
    std::string path = dir.absolutePath().toStdString();
    fileNames = getVideoName(videoList,path);
    for(int i = 0;i<videoList.size();i++)
    {
        videoList[i].erase(videoList[i].begin());
    }


    TT->setVideoName(fileNames);
    TT->start();

    ui->statusBar->showMessage(QString::fromStdString(fileNames[1])+" is processing...");

}

void MainWindow::receiveSerialData()
{
    //receive sensor data from COM port

    QByteArray msg;

    //check available data size
    if(port->bytesAvailable() > 12)
    {
        //read data from COM port
        msg = port->readAll();
        QString data;

        //cut usful substring
        for(int i = 0; i < msg.size(); i++)
        {
            //check first and last word
            if(msg.at(i) == 35 && msg.at(i+12) == 65)
            {
                for(int j = 0;j < 13; j++)
                {
                    data.append(msg.at(j+i));
                }

                //convert to temperture, RH and pressure
                std::vector<float> T(2);
                std::vector<float> RH(2);
                float P;

                T[0] = data.at(1).toLatin1()+data.at(2).toLatin1()/100.0;
                T[1] = data.at(5).toLatin1()+data.at(5).toLatin1()/100.0;
                RH[0] = data.at(3).toLatin1()+data.at(4).toLatin1()/100.0;
                RH[1] = data.at(7).toLatin1()+data.at(8).toLatin1()/100.0;
                P = data.at(9).toLatin1() * 128*128+data.at(10).toLatin1()*128+data.at(11).toLatin1();
                P = P / 100.0;

                //show on UI
                ui->inhive_t_lcdNumber->display(T[0]);
                ui->inhive_rh_lcdNumber->display(RH[0]);
                ui->outhive_t_lcdNumber->display(T[1]);
                ui->outhive_rh_lcdNumber->display(RH[1]);
                ui->pressure_lcdNumber->display(P);
                break;
            }
        }
    }
}

void MainWindow::recordSensorData()
{
    //check dir is exist or not
    QDir dir("sensor_data");
    if(!dir.exists())
    {
        dir.cdUp();
        if(dir.mkdir("sensor_data"))
        {
            dir.cd("sensor_data");
        }
        else
        {
            emit sendSystemLog("Create dir failed!");
            return;
        }
    }

    //open file for save sensor data
    QString fileName = QDateTime::currentDateTime().toString("yyyy-MM-dd")+".csv";
    QFile file(dir.absolutePath()+"/"+fileName);
    QTextStream out(&file);
    if(!file.exists())
    {
        file.open(QIODevice::ReadWrite);
        out << ""
            << "," << "In-hive Temperture" << "," << "In-hive RH"
            << "," << "Out-hive Temperture" << "," << "Out-hive RH"
            << "," << "Air Pressure" << "\n";
    }
    else
    {
        file.open(QIODevice::Append);
    }

    //get sensor information from UI and save
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss-zzz")
        << "," << ui->inhive_t_lcdNumber->value() << "," << ui->inhive_rh_lcdNumber->value()
        << "," << ui->outhive_t_lcdNumber->value() << "," << ui->outhive_rh_lcdNumber->value()
        << "," << ui->pressure_lcdNumber->value() << "\n";

    //close file
    file.close();

}


std::vector<std::string> MainWindow::getVideoName(QVector<QStringList> list,std::string path)
{
    //get video file name from fir
    std::vector<std::string> fileNames;
    for(int i = 0;i<videoList.size();i++)
    {
        if (list[i].size()<1)
        {
            return fileNames;
        }
        std::string folder;
        if (i == 0)
            folder = "/Camera_L/";
        else if(i == 1)
            folder = "/Camera_M/";
        else if(i == 2)
            folder = "/Camera_R/";
        fileNames.push_back(path+folder+list[i][ 0].toStdString());
    }
    return fileNames;
}

void mouseCallBack(int event, int x, int y, int flag,void* userdata)
{
    //manul stitch mouse callback function

    static cv::Point lastPoint;
    cv::Point shiftDelta = cv::Point(0,0);
    int imgIndex;

    if (event == CV_EVENT_LBUTTONDOWN)
    {
        lastPoint = cv::Point(x,y);
        return;
    }
    else if (event == CV_EVENT_MBUTTONDOWN)
    {
        qDebug() << "fileSaved";
        cv::FileStorage f("model/manual_stitching.xml",cv::FileStorage::WRITE);
        f << "point" << originPoint;
        f.release();

    }
    if (flag == 1)
    {
        qDebug() << event << x << y << flag;
        shiftDelta = cv::Point(x,y)-lastPoint;
        if (x > originPoint[0].x && x <= originPoint[0].x+IMAGE_SIZE_X)
        {
            imgIndex = 0;
        }
        if (x > originPoint[1].x && x <= originPoint[1].x+IMAGE_SIZE_X)
        {
            imgIndex = 1;
        }
        if (x > originPoint[2].x && x <= originPoint[2].x+IMAGE_SIZE_X)
        {
            imgIndex = 2;
        }

        originPoint[imgIndex].x = originPoint[imgIndex].x+shiftDelta.x;
        originPoint[imgIndex].y = originPoint[imgIndex].y+shiftDelta.y;

        lastPoint = cv::Point(x,y);

        qDebug() << originPoint[0].x << originPoint[0].y << originPoint[1].x << originPoint[1].y << originPoint[2].x << originPoint[2].y;
        cv::Mat cat(cv::Size(IMAGE_SIZE_X*3,IMAGE_SIZE_Y),CV_8UC3,cv::Scalar(0));
        for (int i=0;i<3;i++)
        {
            if(originPoint[i].x>=0&&originPoint[i].y>=0)
            {
                stitchFrame[i](cv::Rect(0,0,fmin(IMAGE_SIZE_X,IMAGE_SIZE_X*3-originPoint[i].x),IMAGE_SIZE_Y-originPoint[i].y)).copyTo(cat(cv::Rect(originPoint[i].x,originPoint[i].y,fmin(IMAGE_SIZE_X,IMAGE_SIZE_X*3-originPoint[i].x),IMAGE_SIZE_Y-originPoint[i].y)));
            }

        }

        cv::imshow("Stitch",cat);
    }

}

void MainWindow::on_stitchingStart_pushButton_clicked()
{
    if(isStopProcessing != 1)
    {
        static int pressCount = 0;
        //start processing video
        ui->videoName_textBrowser->clear();
        for (int k = 0;k<videoList[0].size();k++)
        {
            QString fileName = videoList[0][k]+"\n";
            fileName = fileName.mid(2,fileName.length()-2);
            ui->videoName_textBrowser->insertPlainText(fileName);
        }

        stitchImage();

        //send finish signal to mainwindow
        if(pressCount == 0)
        {
            connect(TT,SIGNAL(finish()),this,SLOT(on_stitchingStart_pushButton_clicked()));

        }
        isStopProcessing = 0;
        pressCount++;
        ui->stitchingStart_pushButton->setEnabled(false);
        ui->stitchingStop_pushButton->setEnabled(true);
    }
    isStopProcessing = 0;

}

void MainWindow::on_stitchingStop_pushButton_clicked()
{
    //stop processing video
    isStopProcessing = 1;
    TT->stopStitch();

    ui->stitchingStart_pushButton->setEnabled(true);
    ui->stitchingStop_pushButton->setEnabled(false);

    ui->videoName_textBrowser->clear();
    for (int k = 0;k<videoList[0].size();k++)
    {
        QString fileName = videoList[0][k]+"\n";
        fileName = fileName.mid(2,fileName.length()-2);
        ui->videoName_textBrowser->insertPlainText(fileName);
    }
}

void MainWindow::on_stitching_pushButton_clicked()
{
    //manul stitch image
    if (stitchMode == 0)
    {
        std::vector<std::string> fileNames;
        std::string path = dir.absolutePath().toStdString();
        fileNames = getVideoName(videoList,path);
        for(int i = 0;i<videoList.size();i++)
        {
            cv::VideoCapture cap(fileNames[i]);
            cv::Mat temp;
            cap.read(temp);
            stitchFrame.push_back(temp);
            cap.release();
        }

        originPoint.resize(3);
        originPoint[0] = cv::Point(0,0);
        originPoint[1] = cv::Point(IMAGE_SIZE_X,0);
        originPoint[2] = cv::Point(IMAGE_SIZE_X*2,0);

        cv::Mat cat;
        cv::hconcat(stitchFrame,cat);
        cv::namedWindow("Stitch");
        cv::setMouseCallback("Stitch",mouseCallBack,0);
        cv::imshow("Stitch",cat);
    }
    else
    {
        std::vector<std::string> fileNames;
        std::string path = dir.absolutePath().toStdString();
        fileNames = getVideoName(videoList,path);
        for(int i = 0;i<videoList.size();i++)
        {
            cv::VideoCapture cap(fileNames[i]);
            cv::Mat temp;
            cap.read(temp);
            stitchFrame.push_back(temp);
            cap.release();
        }

    }
}

void MainWindow::on_dp_hough_circle_spinBox_valueChanged(int arg1)
{
    TT->setHoughCircleParameters(ui->dp_hough_circle_spinBox->value(),ui->minDist_hough_circle_spinBox->value(),ui->para_1_hough_circle_spinBox->value(),ui->para_2_hough_circle_spinBox->value(),ui->minRadius_hough_circle_spinBox->value(),ui->maxRadius_hough_circle_spinBox->value());
}

void MainWindow::on_minDist_hough_circle_spinBox_valueChanged(int arg1)
{
    TT->setHoughCircleParameters(ui->dp_hough_circle_spinBox->value(),ui->minDist_hough_circle_spinBox->value(),ui->para_1_hough_circle_spinBox->value(),ui->para_2_hough_circle_spinBox->value(),ui->minRadius_hough_circle_spinBox->value(),ui->maxRadius_hough_circle_spinBox->value());
}

void MainWindow::on_para_1_hough_circle_spinBox_valueChanged(int arg1)
{
    TT->setHoughCircleParameters(ui->dp_hough_circle_spinBox->value(),ui->minDist_hough_circle_spinBox->value(),ui->para_1_hough_circle_spinBox->value(),ui->para_2_hough_circle_spinBox->value(),ui->minRadius_hough_circle_spinBox->value(),ui->maxRadius_hough_circle_spinBox->value());
}

void MainWindow::on_para_2_hough_circle_spinBox_valueChanged(int arg1)
{
    TT->setHoughCircleParameters(ui->dp_hough_circle_spinBox->value(),ui->minDist_hough_circle_spinBox->value(),ui->para_1_hough_circle_spinBox->value(),ui->para_2_hough_circle_spinBox->value(),ui->minRadius_hough_circle_spinBox->value(),ui->maxRadius_hough_circle_spinBox->value());
}

void MainWindow::on_minRadius_hough_circle_spinBox_valueChanged(int arg1)
{
    TT->setHoughCircleParameters(ui->dp_hough_circle_spinBox->value(),ui->minDist_hough_circle_spinBox->value(),ui->para_1_hough_circle_spinBox->value(),ui->para_2_hough_circle_spinBox->value(),ui->minRadius_hough_circle_spinBox->value(),ui->maxRadius_hough_circle_spinBox->value());
}

void MainWindow::on_maxRadius_hough_circle_spinBox_valueChanged(int arg1)
{
    TT->setHoughCircleParameters(ui->dp_hough_circle_spinBox->value(),ui->minDist_hough_circle_spinBox->value(),ui->para_1_hough_circle_spinBox->value(),ui->para_2_hough_circle_spinBox->value(),ui->minRadius_hough_circle_spinBox->value(),ui->maxRadius_hough_circle_spinBox->value());
}

void MainWindow::on_show_image_checkBox_clicked()
{
    TT->setShowImage(ui->show_image_checkBox->isChecked());
}

void MainWindow::on_actionChange_SVM_Model_triggered()
{
    //for change the SVM classifer model
    QString fileNameQ = QFileDialog::getOpenFileName(this,"SVM Model (.yaml)","","Model Files (*.yaml)");
    TT->setSVMModelFileName(fileNameQ.toStdString());
    ui->statusBar->showMessage("Change SVM model "+fileNameQ);
}

void MainWindow::on_actionChange_PCA_Model_triggered()
{
    //for change the PCA model
    QString fileNameQ = QFileDialog::getOpenFileName(this,"PCA Model (.txt)","","Model Files (*.txt)");
    TT->setPCAModelFileName(fileNameQ.toStdString());
    ui->statusBar->showMessage("Change PCA model "+fileNameQ);
}

void MainWindow::on_actionChange_Stitching_Model_triggered()
{
    //for change manul stitching model
    QString fileNameQ = QFileDialog::getOpenFileName(this,"Stitching Model (.xml)","","Model Files (*.xml)");
    TT->setManualStitchingFileName(fileNameQ.toStdString());
    ui->statusBar->showMessage("Change Stitching model "+fileNameQ);
}

void MainWindow::on_actionTrain_New_Tag_Model_triggered()
{
    //choose train data folder
    QString fileName = QFileDialog::getExistingDirectory();

    //choose test data folder
    QString fileName2 = QFileDialog::getExistingDirectory();

    //presetting parameters
    tagRecognitionParameters params;
    params.withHOG = ui->actionWith_HOG->isChecked();
    params.withPCA = ui->actionWith_PCA->isChecked();
    params.CValPowLower = C_LOWER;
    params.CValPowUpper = C_UPPER;
    params.PCARemains = ui->PCARemains_spinBox->value();

    QFuture<void> trainAllStep = QtConcurrent::run(TR,&tag_recognition::trainAllStep,fileName,fileName2,params);
}

void MainWindow::on_erase_pushButton_clicked()
{
    //erase file from wating processing list
    for(int i = 0;i<videoList.size();i++)
    {
        videoList[i].erase(videoList[i].begin());
    }
    ui->videoName_textBrowser->clear();
    for (int k = 0;k<videoList[0].size();k++)
    {
        QString fileName = videoList[0][k]+"\n";
        fileName = fileName.mid(2,fileName.length()-2);
        ui->videoName_textBrowser->insertPlainText(fileName);
    }

}

void MainWindow::on_port_name_comboBox_activated(int index)
{
    //connect to COM port and
    if(serialClock->isActive())
    {
        serialClock->stop();
        serialClock->setInterval(SERIAL_TIME);
        serialClock->start();

        recordClock->stop();
        recordClock->setInterval(RECORD_TIME);
        recordClock->start();

        port->close();
        port->setBaudRate(QSerialPort::Baud9600);
        port->setPort(this->portList[index]);
        port->open(QIODevice::ReadOnly);

    }
    else
    {
        serialClock->setInterval(SERIAL_TIME);
        serialClock->start();

        recordClock->setInterval(RECORD_TIME);
        recordClock->start();

        port = new QSerialPort;
        port->setBaudRate(QSerialPort::Baud9600);
        port->setPort(this->portList[index]);
        port->open(QIODevice::ReadOnly);
    }


    emit sendSystemLog("Sensor Connect Successful.");
}

void MainWindow::on_actionStart_Analysis_Data_triggered()
{
    DPW = new DataProcessWindow;
    DPW->show();
}

void MainWindow::on_actionWith_PCA_triggered()
{
    //set inital PCA and HOG parameters
    TT->setPCAandHOG(ui->actionWith_PCA->isChecked(),ui->actionWith_HOG->isChecked());
}

void MainWindow::on_actionWith_HOG_triggered()
{
    //set inital PCA and HOG parameters
    TT->setPCAandHOG(ui->actionWith_PCA->isChecked(),ui->actionWith_HOG->isChecked());
}

void MainWindow::on_erase__ten_pushButton_clicked()
{
    //erase file from wating processing list
    for(int i = 0;i<videoList.size();i++)
    {
        if(videoList[i].size() > 10)
        {
            for(int j = 0; j < 10;j++)
            {
                videoList[i].erase(videoList[i].begin());
            }
        }
    }
    ui->videoName_textBrowser->clear();
    for (int k = 0;k<videoList[0].size();k++)
    {
        QString fileName = videoList[0][k]+"\n";
        fileName = fileName.mid(2,fileName.length()-2);
        ui->videoName_textBrowser->insertPlainText(fileName);
    }
}

void MainWindow::on_show_text_checkBox_clicked()
{
    //show text ID in ui image window?
    TT->setShowText(ui->show_text_checkBox->isChecked());
}

void MainWindow::on_text_system_comboBox_currentIndexChanged(const QString &arg1)
{
    //change text system
    TT->setTextSystem(arg1);
    emit sendSystemLog("Change text system into "+arg1);

    //check mode file exist and load
    if(arg1 == "SUTM")
    {
        if(SVMModel_SUTM.exists())
            TT->setSVMModelFileName(SVMModel_SUTM.fileName().toStdString());
        else
            emit sendSystemLog(SVMModel_SUTM.fileName()+" no found!");
    }
    else if(arg1 == "MUTM")
    {
        if(SVMModel_MUTM.exists())
            TT->setSVMModelFileName(SVMModel_MUTM.fileName().toStdString());
        else
            emit sendSystemLog(SVMModel_MUTM.fileName()+" no found!");
    }
    else if(arg1 == "Test")
    {
        if(SVMModel_Test.exists())
            TT->setSVMModelFileName(SVMModel_Test.fileName().toStdString());
        else
            emit sendSystemLog(SVMModel_Test.fileName()+" no found!");
    }
}

void MainWindow::on_actionRecord_Process_Procedure_triggered()
{
    //record video from ui image window
    //some crash?
    if(ui->actionRecord_Process_Procedure->isChecked())
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Video File"),"record.avi",tr("Video (*.avi)"));
        TT->initVideoRecord(fileName);
    }
    else
    {
        TT->finishVideoRecord();
    }

}

void MainWindow::on_show_trajectory_checkBox_clicked()
{
    //show trajectory in ui image window?
    TT->setShowTrajectory(ui->show_trajectory_checkBox->isChecked());
}
