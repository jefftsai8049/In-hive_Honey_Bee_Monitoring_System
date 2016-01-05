#include "object_tracking.h"



int track::size()
{
    return time.size();
}

cv::Point track::lastPosition()
{
    if(time.size() > 0)
        return position[time.size()-1];
    else
        return  cv::Point(-1,-1);
}

cv::Point track::futurePosition()
{
    if(time.size() > 1)
        return 2*position[time.size()-1]-position[time.size()-2];
    else
        return cv::Point(-1,-1);
}

double track::howClose(const cv::Vec3f &circle)
{
    cv::Point circleCenter(circle[0],circle[1]);
    cv::Point lastPoint = lastPosition();
    cv::Point futurePoint = futurePosition();

    if(lastPoint != cv::Point(-1,-1))
    {
        cv::Point distance = circleCenter-lastPoint;
        cv::Point predictDistance = circleCenter-futurePoint;
        return fmin(pow(distance.x,2)+pow(distance.y,2),pow(predictDistance.x,2)+pow(predictDistance.y,2));

    }
    else
    {
        return NULL;
    }
}



object_tracking::object_tracking(QObject *parent, const QDateTime fileTime)
{
    this->startTime = fileTime;


    if(!outDataDir.exists())
    {
        outDataDir.cdUp();
        outDataDir.mkdir("processing_data");
        outDataDir.cd("processing_data");
    }
}

object_tracking::~object_tracking()
{

}

void object_tracking::compute(const QDateTime &time, const std::vector<cv::Vec3f> &circles, const std::vector<std::string> &w1, const std::vector<std::string> &w2)
{

    this->nowTime = time;

    //threshold of distance
    int minDistanceThreshold = 2000;
    //    int minTimeGap = 5;

    //for saving path and circle size
    int circleSize = circles.size();
    int pathSize = this->path.size();
    std::vector<int> circleMarked(circleSize);

    //-1-for not belong any path
    //0+-for belong which path
    for(int i = 0; i < circleSize; i++)
    {
        circleMarked[i] = -1;
    }

    //check path and circle size
    for(int j = 0; j < pathSize; j++)
    {
        //calculate each circle to each path distance
        std::vector<double> distanceSqrt(circleSize);
        for(int k = 0; k < circleSize; k++)
        {
            distanceSqrt[k] = this->path[j].howClose(circles[k]);
        }
        //find the min distance of circle to lastapth
        double minDiatanceSqrt;
        int index;
        mf::vectorFindMin(minDiatanceSqrt,index,distanceSqrt);

        int timeGap = this->path[j].time[this->path[j].time.size()-1].secsTo(time);


#ifdef DEBUG_OBJECT_TRACKING
        qDebug() <<"path:" << j << "circle:" << index << "min distance " << minDiatanceSqrt;
#endif
        //if distance and time is close enough
        if(minDiatanceSqrt < minDistanceThreshold)
        {
            if(timeGap < FORGET_TRACKING_TIME)
            {
                this->path[j].time.push_back(time);
                this->path[j].w1.push_back(w1[index].c_str()[0]);
                this->path[j].w2.push_back(w2[index].c_str()[0]);
                this->path[j].position.push_back(cv::Point(circles[index][0],circles[index][1]));
                circleMarked[index] = j;
            }
        }

    }

    int count = 0;
    for(int k = 0; k < circleSize; k++)
    {

        if(circleMarked[k] == -1)
        {
            //qDebug() << "new path" << count+pathSize;
            track emptyPath;
            emptyPath.time.push_back(time);
            emptyPath.w1.push_back(w1[k].c_str()[0]);
            emptyPath.w2.push_back(w2[k].c_str()[0]);
            emptyPath.position.push_back(cv::Point(circles[k][0],circles[k][1]));

            circleMarked[k] = count+pathSize;
            count++;

            this->path.push_back(emptyPath);
        }
    }

    //    for(int i = 0; i < this->path.size(); i++)
    //    {
    //        qDebug() << "pathSize" << this->path.size() << "position" << this->path[i].position[0].x << this->path[i].position[0].y;
    //    }

}

void object_tracking::setImageRange(const cv::Size &imageRange)
{
    this->range = imageRange;
}

void object_tracking::lastPath(std::vector<std::vector<cv::Point> > &path)
{
    path.clear();
    for(int i = 0;i < this->path.size();i++)
    {
        if((this->path[i].time[this->path[i].time.size()-1].secsTo(nowTime)) < 5)
        {

            if(this->path[i].size() < REMAIN_SIZE)
            {
                path.push_back(this->path[i].position);

            }
            else
            {
                std::vector<cv::Point>::const_iterator first = this->path[i].position.end()-REMAIN_SIZE;
                std::vector<cv::Point>::const_iterator last = this->path[i].position.end();
                std::vector<cv::Point> temp(first,last);
                path.push_back(temp);

            }
        }
    }

}

void object_tracking::drawPath(cv::Mat& src)
{

    std::vector<std::vector<cv::Point> > path;
    this->lastPath(path);
    //qDebug() << "i" << path.size();

    for(int i = 0; i < path.size();i++)
    {
        //qDebug() <<"i" << i << "j" << path[i].size();
        if(path[i].size() < 1)
        {
            //qDebug() << "size not enough";
            break;
        }
        for(int j = 0 ;j < path[i].size()-1;j++)
        {

            //qDebug() << path[i][j].x <<path[i][j].y << path[i][j+1].x <<path[i][j+1].y;
            if(path[i][j] != cv::Point(-1,-1) || path[i][j+1] != cv::Point(-1,-1))
                cv::line(src,path[i][j],path[i][j+1],cv::Scalar(255,255,255),4);
        }
    }
}

void object_tracking::savePath()
{    
    //QFileInfo fileInfo("out/processing_data/"+startTime.toString("yyyy-MM-dd_hh-mm-ss-zzz")+".csv");

    QFile file(outDataDir.path()+"/"+this->startTime.toString("yyyy-MM-dd_hh-mm-ss-zzz")+".csv");

    if(!file.exists())
    {
        file.open(QIODevice::WriteOnly);
        emit sendSystemLog(file.fileName()+" establish!");
    }
    else
    {
        file.open(QIODevice::Append);
    }
    std::stringstream outFile;
    for(int i = 0; i < this->path.size(); i++)
    {
        //qDebug() << this->path[i].time[this->path[i].time.size()-1].secsTo(this->nowTime);
        if(this->path[i].time[this->path[i].time.size()-1].secsTo(this->nowTime) > FORGET_TRACKING_TIME)
        {
            if(this->path[i].time.size() > SHORTEST_SAMPLE_SIZE)
            {
                for(int j = 0; j < this->path[i].time.size(); j++)
                {

                    outFile << this->path[i].w1[j] << "," << this->path[i].w2[j] << ","
                            << this->path[i].time[j].toString("yyyy-MM-dd_hh-mm-ss-zzz").toStdString() << ","
                            << this->path[i].position[j].x << ","
                            << this->path[i].position[j].y << ",";
                }
                outFile << "\n";
            }
            this->path.erase(this->path.begin()+i);
            i--;
        }
    }
    file.write(outFile.str().c_str());
    file.close();
}

void object_tracking::saveAllPath()
{
    QFile file(outDataDir.path()+"/"+this->startTime.toString("yyyy-MM-dd_hh-mm-ss-zzz")+".csv");

    if(!file.exists())
    {
        file.open(QIODevice::WriteOnly);
        emit sendSystemLog(file.fileName()+" establish!");
    }
    else
    {
        file.open(QIODevice::Append);
    }
    std::stringstream outFile;
    for(int i = 0; i < this->path.size(); i++)
    {
        //qDebug() << this->path[i].time[this->path[i].time.size()-1].secsTo(this->nowTime);
        if(this->path[i].time.size() > SHORTEST_SAMPLE_SIZE)
        {
            for(int j = 0; j < this->path[i].time.size(); j++)
            {

                outFile << this->path[i].w1[j] << "," << this->path[i].w2[j] << ","
                        << this->path[i].time[j].toString("yyyy-MM-dd_hh-mm-ss-zzz").toStdString() << ","
                        << this->path[i].position[j].x << ","
                        << this->path[i].position[j].y << ",";
            }
            outFile << "\n";

        }
    }
    this->path.clear();
    file.write(outFile.str().c_str());
    file.close();
}

QString object_tracking::voting(track path)
{
    std::vector<char> w1_word;
    std::vector<int> w1_count;
    std::vector<char> w2_word;
    std::vector<int> w2_count;

    for(int i = 0; i < path.w1.size(); i++)
    {
        bool fined = false;
        for(int j = 0; j < w1_word.size(); j++)
        {
            if(w1_word[j] == path.w1[i])
            {
                fined = true;
                w1_count[j]++;
                break;
            }
        }
        if(!fined)
        {
            w1_word.push_back(path.w1[i]);
            w1_count.push_back(1);
        }

        fined = false;
        for(int j = 0; j < w2_word.size(); j++)
        {
            if(w2_word[j] == path.w2[i])
            {
                fined = true;
                w2_count[j]++;
                break;
            }
        }
        if(!fined)
        {
            w2_word.push_back(path.w2[i]);
            w2_count.push_back(1);
        }
    }

    for(int i = 0; i < w1_word.size()-1; i++)
    {
        for(int j = i+1; j < w1_word.size(); j++)
        {
            if(w1_count[i]<w1_count[j])
            {
                std::swap(w1_count[i],w1_count[j]);
                std::swap(w1_word[i],w1_word[j]);
            }
        }
    }

    for(int i = 0; i < w2_word.size()-1; i++)
    {
        for(int j = i+1; j < w2_word.size(); j++)
        {
            if(w2_count[i]<w2_count[j])
            {
                std::swap(w2_count[i],w2_count[j]);
                std::swap(w2_word[i],w2_word[j]);
            }
        }
    }

    char word1_final,word2_final;;

    for(int m = 0; m < w1_word.size(); m++)
    {
        if(w1_word[m] != '!')
        {
            word1_final = w1_word[m];
            break;
        }

    }

    for(int m = 0; m < w2_word.size(); m++)
    {
        if(w2_word[m] != '!')
        {
            word2_final = w2_word[m];
            break;
        }

    }
    QString result;
    result.push_back(word1_final);
    result.push_back(word2_final);
    return result;
}

QVector<cv::Point> object_tracking::interpolation(const std::vector<cv::Point> &position, const std::vector<QDateTime> &time)
{
    int minTimeStep;
    minTimeStep = this->minTimeStep(time);
    //qDebug() << minTimeStep;

    QVector<cv::Point> positionNew;

    for(int i = 0; i < time.size()-1; i++)
    {
        positionNew.append(position[i]);
        int timeGap = time[i].msecsTo(time[i+1]);
        if(timeGap > minTimeStep)
        {
            int cSize = timeGap/minTimeStep-1;
            for(int j = 0; j < cSize; j++)
            {
                cv::Point cPoint(position[i]+(j+1/cSize+1)*(position[i+1]-position[i]));
                positionNew.append(cPoint);
            }
        }
    }
    positionNew.append(position[position.size()-1]);
    return positionNew;
}

void object_tracking::saveTrackPro(const QVector<trackPro> &path, const QString &fileName)
{
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadWrite);

    QTextStream out(&file);
    out << "ID,StartTime,EndTime,Size,Position\n";
    for(int i = 0; i < path.size(); i++)
    {
        out << path.at(i).ID << ","
            << path.at(i).startTime.toString("yyyy-MM-dd_hh-mm-ss-zzz") << ","
            << path.at(i).endTime.toString("yyyy-MM-dd_hh-mm-ss-zzz") << ","
            << path.at(i).size << ",";
        for(int j = 0; j < path.at(i).size; j++)
        {
            out << path.at(i).position.at(j).x << "-" << path.at(i).position.at(j).y;
            if(j != path.at(i).size-1)
                out << ",";
        }
        out << "\n";
    }
    file.close();
}

void object_tracking::loadDataTrack(const QStringList &fileNames, std::vector<track> *path)
{
    path->clear();
    for(int i = 0; i < fileNames.size(); i++)
    {
        QFile f;
        f.setFileName(fileNames[i]);
        if(f.exists())
        {
            f.open(QIODevice::ReadOnly);
            while(!f.atEnd())
            {
                track t;
                QString msg = f.readLine();
                msg = msg.trimmed();
                while(msg.at(msg.size()-1) == ",")
                    msg = msg.left(msg.size()-1);

                QStringList data = msg.split(",");
                for(int m = 0; m < data.size(); m=m+5)
                {
                    t.w1.push_back(data[m].toStdString()[0]);
                    t.w2.push_back(data[m+1].toStdString()[0]);
                    t.time.push_back(QDateTime::fromString(data[m+2],"yyyy-MM-dd_hh-mm-ss-zzz"));
                    t.position.push_back(cv::Point(data[m+3].toInt(),data[m+4].toInt()));
                }
                path->push_back(t);

            }
            emit sendSystemLog(fileNames[i]);
            emit sendProgress((i+1)*100/fileNames.size());
        }
        else
        {
            emit sendSystemLog("File not exist!");
        }
    }
    emit sendLoadRawDataFinish();
}

int object_tracking::minTimeStep(const std::vector<QDateTime> &time)
{
    int min = 1000;
    for(int i = 0; i < time.size()-1; i++)
    {
        int timeGap = time[i].msecsTo(time[i+1]);
        if(timeGap < min)
            min = timeGap;

        if(min < 1000.0/MIN_FPS)
            break;
    }
    return min;
}

cv::Point object_tracking::mean(const QVector<cv::Point> &motion)
{
    float x,y;
    for(int i = 0; i < motion.size(); i++)
    {
        x+=(float)motion.at(i).x/(float)motion.size();
        y+=(float)motion.at(i).y/(float)motion.size();
    }
    return cv::Point(x,y);
}

QVector<float> object_tracking::variance(const QVector<cv::Point> &motion)
{
    float xVar = 0,yVar = 0,xyCoVar = 0;

    cv::Point meanPoint = this->mean(motion);

    for(int i = 0; i < motion.size(); i++)
    {
        xVar += pow(motion.at(i).x-meanPoint.x,2);
        yVar += pow(motion.at(i).y-meanPoint.y,2);
        xyCoVar += (motion.at(i).x-meanPoint.x)*(motion.at(i).y-meanPoint.y);
        //qDebug() << xVar << yVar << xyCoVar;
    }
    //qDebug() << xVar << yVar << xyCoVar;
    QVector<float> var(3);
    var[0] = (float)xVar/(float)(motion.size()-1);
    var[1] = (float)yVar/(float)(motion.size()-1);
    var[2] = (float)xyCoVar/(float)(motion.size()-1);
    //qDebug() << var[0] << var[1] << var[2];
    return var;
}

int object_tracking::direction(const QVector<cv::Point> &motion,const objectTrackingParameters params)
{
    QVector<cv::Point> motionVector;
    double angleSum = 0;
    for(int i = 0 ; i < motion.size()-1; i++)
    {
        motionVector.append(motion.at(i+1)-motion.at(i));
    }
    float *angleEach;
    angleEach = new float[motionVector.size()-1];
    for(int i = 0 ; i < motionVector.size()-1; i++)
    {
        float angle[2];
        float r[2],x[2],y[2];
        for(int k = 0; k < 2; k++)
        {
            r[k] = sqrt(pow(motionVector.at(i+k).x,2)+pow(motionVector.at(i+k).y,2));
            x[k] = motionVector.at(i+k).x;
            y[k] = motionVector.at(i+k).y;

            if(r[k] == 0)
            {
                angle[k] = 0;
            }
            else
            {
                angle[k] = asin(y[k]/r[k])/2.0/3.1415926*360.0;
                if(x[k] >= 0)
                {
                    angle[k] = angle[k];
                }
                else
                {
                    angle[k] = 180.0-(angle[k]);
                }
            }

        }
        angleEach[i] = angle[0]-angle[1];
        angleSum += angleEach[i];
    }



    if(angleSum > params.thresholdDirection)
        return RIGHT_MOVE;
    else if(angleSum < -params.thresholdDirection)
        return LEFT_MOVE;
    else
    {
        bool isWaggle = true;
        for(int m = 0;m < motionVector.size()-1;m++)
        {
            if(abs(angleEach[m]) == 0)
            {
                isWaggle = false;
                return FORWARD_MOVE;
            }
        }
        return WAGGLE;

    }
}

void object_tracking::rawDataPreprocessing(const std::vector<track> *path, QVector<trackPro> *TPVector)
{
    TPVector->clear();
    for(int i = 0; i < path->size(); i++)
    {
        trackPro TP;
        TP.ID = this->voting(path->at(i));
        TP.startTime = path->at(i).time[0];
        TP.endTime = path->at(i).time[path->at(i).time.size()-1];
        TP.position = this->interpolation(path->at(i).position,path->at(i).time);
        TP.size = TP.position.size();
        TPVector->append(TP);
        emit sendProgress((i+1)*100/path->size());
    }

    //check dir is exist or not
    QDir dir("processed_data");
    if(!dir.exists())
    {
        dir.cdUp();
        if(dir.mkdir("processed_data"))
        {
            dir.cd("processed_data");
        }
        else
        {
            emit sendSystemLog("Create dir failed!");
            return;
        }
    }

    //open file for save sensor data
    QString fileName = dir.absolutePath()+"/"+TPVector->at(0).startTime.toString("yyyy-MM-dd_hh-mm-ss-zzz")+"_processed"+".csv";
    this->saveTrackPro(*TPVector,fileName);
}

void object_tracking::loadDataTrackPro(const QString &fileName, QVector<trackPro> *path)
{
    path->clear();
    QFile file(fileName);
    if(file.exists())
    {
        file.open(QIODevice::ReadOnly);
        file.readLine();
        while(!file.atEnd())
        {
            QString msg = file.readLine();
            msg = msg.trimmed();
            QStringList data = msg.split(",");
            trackPro t;
            t.ID = data.at(0);
            t.startTime = QDateTime::fromString(data.at(1),"yyyy-MM-dd_hh-mm-ss-zzz");
            t.endTime = QDateTime::fromString(data.at(2),"yyyy-MM-dd_hh-mm-ss-zzz");
            t.size = data.at(3).toInt();
            for(int i = 4; i < data.size(); i++)
            {
                QStringList position = data.at(i).split("-");
                cv::Point p = cv::Point(position[0].toInt(),position[1].toInt());
                t.position.append(p);
            }
            path->append(t);
        }

        sendSystemLog(fileName);

    }
}

void object_tracking::tracjectoryClassify(QVector<trackPro> &path, const objectTrackingParameters params)
{
    //check all path
    for(int i = 0; i < path.size(); i++)
    {
        QVector<char> patternSequence;
        //check path position size is bigger than  SHORTEST_SAMPLE_SIZE or not
        if(path.at(i).size > this->segmentSize-1)
        {
            for(int j = 0; j < path.at(i).size-this->segmentSize+1; j++)
            {
                //grab path segment
                QVector<cv::Point> motion(this->segmentSize);
                for(int m = 0; m < this->segmentSize; m++)
                {
                    motion[m] = path.at(i).position.at(j+m);
                }
                QVector<float> var = this->variance(motion);
                //qDebug() << var[0] << var[1] << var[2];

                //NO_MOVE
                if(var[0] < params.thresholdNoMove && var[1] < params.thresholdNoMove && abs(var[2]) < params.thresholdNoMove)
                {
                    patternSequence.append(NO_MOVE);
                    //drawPathPattern(motion);
                }
                //LOITERING
                else if(var[0] < params.thresholdLoitering && var[1] < params.thresholdLoitering && abs(var[2]) < params.thresholdLoitering)
                {
                    patternSequence.append(LOITERING);
                    //drawPathPattern(motion);
                }
                else
                {
                    //FORWARD_MOVE
                    //RIGHT_MOVE
                    //LEFT_MOVE
                    //WAGGLE
                    patternSequence.append(this->direction(motion,params));

                    //INTERACTION
                    //FORAGING
                    //OTHER
                }
#ifdef SHOW_PATTERN_NAME
                qDebug() << tracjectoryName(patternSequence.at(patternSequence.size()-1));
#endif
            }
        }
        path[i].pattern = patternSequence;
        emit sendProgress((i+1)*100.0/path.size());
    }
}

QString object_tracking::tracjectoryName(const char &pattern)
{
    if(pattern == NO_MOVE)
        return "NO_MOVE";
    else if(pattern == LOITERING)
        return "LOITERING";
    else if(pattern == FORWARD_MOVE)
        return "FORWARD_MOVE";
    else if(pattern == RIGHT_MOVE)
        return "RIGHT_MOVE";
    else if(pattern == LEFT_MOVE)
        return "LEFT_MOVE";
    else if(pattern == WAGGLE)
        return "WAGGLE";
    else if(pattern == INTERACTION)
        return "INTERACTION";
    else if(pattern == FORAGING)
        return "FORAGING";
    else
        return "OTHER";
}

void object_tracking::setPathSegmentSize(const int &size)
{
    this->segmentSize = size;
}

void object_tracking::drawPathPattern(const QVector<cv::Point> &path)
{
    int edgeWidth = 20;

    int maxX,maxY,minX,minY;
    if(path.size() < 1)
        return;

    maxX = path[0].x;
    minX = path[0].x;
    maxY = path[0].y;
    minY = path[0].y;

    for(int i = 1; i < path.size(); i++)
    {
        if(path[i].x > maxX)
            maxX = path[i].x;
        if(path[i].x < minX)
            minX = path[i].x;
        if(path[i].y > maxY)
            maxY = path[i].y;
        if(path[i].y < minY)
            minY = path[i].y;
    }

    if(maxY-minY == 0 || maxX-minX == 0)
        return;
    cv::Mat src;
    src = cv::Mat::zeros(maxY-minY+edgeWidth,maxX-minX+edgeWidth,CV_8UC3);
    for(int i = 0; i < path.size()-1; i++)
    {
        cv::line(src,path[i]-cv::Point(minX,minY)+cv::Point(edgeWidth/2,edgeWidth/2),path[i+1]-cv::Point(minX,minY)+cv::Point(edgeWidth/2,edgeWidth/2),cv::Scalar(rand()%255,rand()%255,rand()%255),1);
    }

    cv::resize(src,src,cv::Size(src.rows*1,src.cols*1));
    cv::imshow("Path",src);
    cv::waitKey(1000);
}

