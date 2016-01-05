#include "trajectory_tracking.h"

trajectory_tracking::trajectory_tracking(QObject *parent) : QThread(parent)
{
    frame.resize(3);
    TR = new tag_recognition;


}

trajectory_tracking::~trajectory_tracking()
{

}

void trajectory_tracking::setImageShiftOriginPoint(std::vector<cv::Point> originPoint)
{
    this->originPoint = originPoint;

}

cv::Mat trajectory_tracking::imageShift(std::vector<cv::Mat> stitchFrame, std::vector<cv::Point> originPoint)
{
    cv::Mat cat(cv::Size(imgSizeX*3,imgSizeY),CV_8UC3,cv::Scalar(0));
    for (int i=0;i<3;i++)
    {
        if(this->originPoint[i].x>=0&&this->originPoint[i].y>=0)
        {
            stitchFrame[i](cv::Rect(0,0,fmin(imgSizeX,imgSizeX*3-this->originPoint[i].x),imgSizeY-this->originPoint[i].y)).copyTo(cat(cv::Rect(this->originPoint[i].x,this->originPoint[i].y,fmin(imgSizeX,imgSizeX*3-this->originPoint[i].x),imgSizeY-this->originPoint[i].y)));
        }

    }
    return cat;
}

cv::Mat trajectory_tracking::imageShiftLoaded(std::vector<cv::Mat> stitchFrame)
{
    //    qDebug() << this->originPoint[0].x << this->originPoint[0].y << this->originPoint[1].x << this->originPoint[1].y << this->originPoint[2].x << this->originPoint[2].y;
    cv::Mat cat(cv::Size(imgSizeX*3,imgSizeY),CV_8UC1,cv::Scalar(0));

    if(stitchFrame[0].type() == CV_8UC3)
    {
        cat.convertTo(cat,CV_8UC3);
    }


    for (int i=0;i<3;i++)
    {
        if(this->originPoint[i].x>=0&&this->originPoint[i].y>=0)
        {
            stitchFrame[i](cv::Rect(0,0,fmin(imgSizeX,imgSizeX*3-this->originPoint[i].x),imgSizeY-this->originPoint[i].y)).copyTo(cat(cv::Rect(this->originPoint[i].x,this->originPoint[i].y,fmin(imgSizeX,imgSizeX*3-this->originPoint[i].x),imgSizeY-this->originPoint[i].y)));
        }

    }
    return cat;
}
#ifndef NO_OCL
void trajectory_tracking::initOCL()
{
    cv::ocl::setUseOpenCL(true);

    if(cv::ocl::useOpenCL())
    {

        cv::ocl::Context context;
        context.create(cv::ocl::Device::TYPE_GPU);
        qDebug() << context.ndevices();
        cv::ocl::Device device = context.device(0);
        emit sendSystemLog("OpenCL is enabled. Version:"+QString::fromStdString(device.OpenCL_C_Version()));
        emit sendSystemLog(QString::fromStdString(device.vendorName())+" "+QString::fromStdString(device.name()));
        //        qDebug() << device.vendorName();

    }
}

#endif
void trajectory_tracking::setVideoName(std::vector<std::string> videoName)
{
    this->videoName = videoName;

}


void trajectory_tracking::setHoughCircleParameters(const int &dp,const int &minDist,const int &para_1,const int &para_2,const int &minRadius,const int &maxRadius)
{
    emit sendSystemLog("Hough circles parametrs set");

    this->dp = dp;

    this->minDist = minDist;

    this->para_1 = para_1;

    this->para_2 = para_2;

    this->minRadius = minRadius;

    this->maxRadius = maxRadius;
}

void trajectory_tracking::setShowImage(const bool &status)
{
    showImage = status;
}

void trajectory_tracking::setSVMModelFileName(const std::string &fileName)
{
    this->SVMModelFileName = fileName;
}

void trajectory_tracking::setPCAModelFileName(const std::string &fileName)
{
    this->PCAModelFileName = fileName;
}

void trajectory_tracking::setManualStitchingFileName(const std::string &fileName)
{
    cv::FileStorage f(fileName,cv::FileStorage::READ);
    std::vector<cv::Point> p;
    if (f.isOpened())
    {
        f["point"] >> p;
        f.release();
    }
    this->setImageShiftOriginPoint(p);
    emit sendSystemLog("model/manual_stitching.xml loaded");
}

void trajectory_tracking::setTagBinaryThreshold(const double &value)
{
    TR->setTagBinaryThreshold(value);
}

void trajectory_tracking::setPCAandHOG(const bool &PCAS, const bool &HOGS)
{
    TR->setPCAandHOG(PCAS,HOGS);
}

void trajectory_tracking::stopStitch()
{
    this->stopped = true;
}

void trajectory_tracking::receiveSystemLog(const QString &log)
{
    emit sendSystemLog(log);
}

void trajectory_tracking::run()
{
    emit sendSystemLog("Processing start!\n"+QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")+"\n");

    emit sendProcessingProgress(0);


    //open video file
    std::vector<cv::VideoCapture> cap(3);
    std::vector<float> deviceFPS(3);
    std::vector<float> nowFrameCount(3);

    //for save system log
    std::stringstream msgSS;
    msgSS << "Processing files :\n";

    //open video file and get FPS
    for(int i = 0; i < 3; i++)
    {
        cap[i].open(this->videoName[i]);
        if(!cap[i].isOpened())
        {
            return;
        }
        deviceFPS[i] = (float)cap[i].get(CV_CAP_PROP_FRAME_COUNT);
        nowFrameCount[i] = 0;

        msgSS << videoName[i] << " FPS :" << deviceFPS[i]/VIDEOTIME << "\n";
    }

    emit sendSystemLog(QString::fromStdString(msgSS.str()));

    //file time
    QDateTime fileTime;
    QString fileTimeS = QString::fromStdString(this->videoName[0]).mid(QString::fromStdString(this->videoName[0]).lastIndexOf("_")-10,19);
    fileTimeS += "-000";
    fileTime = QDateTime::fromString(fileTimeS,"yyyy-MM-dd_hh-mm-ss-zzz");

    OT = new object_tracking(this,fileTime);
    connect(OT,SIGNAL(sendSystemLog(QString)),this,SLOT(receiveSystemLog(QString)));

    //calculate FPS of each video file
    float maxFPS = mf::findMax(deviceFPS[0],deviceFPS[1],deviceFPS[2]);
    for(int j = 0;j<deviceFPS.size();j++)
    {
        deviceFPS[j] = deviceFPS[j]/maxFPS;
    }


    //load SVM model
    if(!TR->loadSVMModel(SVMModelFileName))
    {
        return;
    }
    //load PCA model
    if(!TR->loadPCAModel(PCAModelFileName))
    {
        return;
    }

    //thread stop flag
    this->stopped = false;
    int frameCount = 0;

    //for saving video frame
    std::vector<cv::Mat> frame(3);
    std::vector<cv::Mat> frameGray(3);

    //main processing loop
    while(!this->stopped)
    {
        emit sendProcessingProgress(frameCount/maxFPS*100.0);


        //calculate processing fps
        QTime clock;
        clock.start();

        //capture frame and convert to gray
        for(int i = 0; i < 3; i++)
        {
            //for fit different FPS
            if(frameCount == 0)
            {
                cap[i] >> frame[i];
            }
            else
            {
                nowFrameCount[i] += deviceFPS[i];
                if(nowFrameCount[i] > 1)
                {
                    cap[i] >> frame[i];
                    nowFrameCount[i]--;
                }
            }

            //convert to gray
            if(frame[i].channels() ==3)
            {
                cv::cvtColor(frame[i],frameGray[i],cv::COLOR_BGR2GRAY);
            }
        }

        //update time
        fileTime = fileTime.addMSecs(VIDEOTIME/maxFPS*1000.0);

        //end of file?
        if(frame[0].empty()||frame[1].empty()||frame[2].empty())
        {
            this->stopped = true;
            break;
        }


        //stitching image
        cv::Mat pano;
        pano = this->imageShiftLoaded(frameGray);
        pano = this->imageCutBlack(pano);

        if(frameCount == 0)
            OT->setImageRange(cv::Size(pano.cols,pano.rows));

        //hough circle detection

        cv::Mat panoSmall(pano.cols/HOUGH_CIRCLE_RESIZE,pano.rows/HOUGH_CIRCLE_RESIZE,CV_8UC1);
        cv::resize(pano,panoSmall,cv::Size(pano.cols/HOUGH_CIRCLE_RESIZE,pano.rows/HOUGH_CIRCLE_RESIZE));

        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(panoSmall,circles,CV_HOUGH_GRADIENT,dp,minDist,para_1,para_2,minRadius,maxRadius);
        this->circleResize(circles);

#ifndef DEBUG_TAG_RECOGNITION
        std::vector<std::string> w1(circles.size());
        std::vector<std::string> w2(circles.size());
#endif

#ifndef DEBUG_TAG_RECOGNITION
//#pragma omp parallel for
#endif
        for (int j=0;j<circles.size();j++)
        {
            cv::Mat tagImg;
            cv::getRectSubPix(pano,cv::Size(circles[j][2]*2+11,circles[j][2]*2+11),cv::Point(circles[j][0], circles[j][1]),tagImg);
            cv::Mat word1,word2;
            TR->tagImgProc(tagImg,word1,word2);

#ifndef DEBUG_TAG_RECOGNITION
            w1[j].push_back(TR->wordRecognition(word1));
            w2[j].push_back(TR->wordRecognition(word2));
#endif

#ifdef DEBUG_TAG_RECOGNITION
            cv::normalize(word1,word1,0,255,cv::NORM_MINMAX);
            cv::normalize(word2,word2,0,255,cv::NORM_MINMAX);
            cv::imshow("w1",word1);
            cv::imshow("w2",word2);
            cv::imwrite("tag/w1.bmp",word1);
            cv::imwrite("tag/w2.bmp",word2);
            cv::waitKey(2000);
#endif

#ifdef SAVE_TAG_IMAGE
            cv::imwrite("SVM/"+w1[j]+"/"+std::to_string(frameCount)+"_"+std::to_string(i)+"1.jpg",word1);
            cv::imwrite("SVM/"+w2[j]+"/"+std::to_string(frameCount)+"_"+std::to_string(i)+"2.jpg",word2);
            qDebug() << QString::fromStdString(w1[j]) << QString::fromStdString(w2[j]);
#endif
        }

        //Bee tracking classify
        //QTime fileTime
        //std::vector<cv::Vec3f> circles;
        //std::vector<std::string> w1;
        //std::vector<std::string> w2;
#ifndef DEBUG_TAG_RECOGNITION
        OT->compute(fileTime,circles,w1,w2);
        OT->savePath();
#endif

        if (showImage)
        {
            //draw cicle
            cv::Mat panoDrawCircle;
            cv::cvtColor(pano,panoDrawCircle,cv::COLOR_GRAY2BGR);

            for(int i = 0; i < circles.size(); i++ )
            {
                cv::Point center(circles[i][0], circles[i][1]);
                int radius = circles[i][2];
                cv::Scalar color = cv::Scalar(255,255,255);
                cv::circle( panoDrawCircle, center, radius, color, 1, 8, 0 );
#ifndef DEBUG_TAG_RECOGNITION
                cv::putText(panoDrawCircle,w1[i],cv::Point(circles[i][0]-15, circles[i][1]+40),cv::FONT_HERSHEY_DUPLEX,1,color);
                cv::putText(panoDrawCircle,w2[i],cv::Point(circles[i][0]+5, circles[i][1]+40),cv::FONT_HERSHEY_DUPLEX,1,color);
#endif
            }
            //            std::vector<std::vector<cv::Point> > path;
            //            OT->lastPath(path);
            //            this->drawPath(panoDrawCircle,path);
#ifndef DEBUG_TAG_RECOGNITION
            OT->drawPath(panoDrawCircle);
#endif
            //resize and show image
            cv::resize(panoDrawCircle,panoDrawCircle,cv::Size(panoDrawCircle.cols/2,panoDrawCircle.rows/2));
            emit sendImage(panoDrawCircle);

        }
        //for calculate processing FPS
        frameCount++;
        emit sendFPS(1000.0/clock.elapsed());
    }
#ifndef DEBUG_TAG_RECOGNITION
    OT->saveAllPath();
#endif
    //close video file and emit finish signal
    for(int i = 0; i < 3; i++)
    {
        cap[i].release();
    }

    delete OT;

    emit sendSystemLog("Processing finish!\n"+QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")+"\n");
    emit finish();
}

cv::Mat trajectory_tracking::imageCutBlack(cv::Mat src)
{
    cv::Mat dst(src, cv::Rect(0, 0, originPoint[2].x+imgSizeX, imgSizeY));
    return dst;
}

void trajectory_tracking::circleResize(std::vector<cv::Vec3f> &circles)
{
    for(int i = 0; i < circles.size(); i++)
    {
        circles[i][0] = circles[i][0]*HOUGH_CIRCLE_RESIZE;
        circles[i][1] = circles[i][1]*HOUGH_CIRCLE_RESIZE;
        circles[i][2] = circles[i][2]*HOUGH_CIRCLE_RESIZE;
    }
}



