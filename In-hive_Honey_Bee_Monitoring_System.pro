#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T12:33:31
#
#-------------------------------------------------

QT       += core gui opengl serialport concurrent printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = In-hive_Honey_Bee_Monitoring_System
TEMPLATE = app

DEFINES += HAVE_OPENCV \
#        += DEBUG_TAG_RECOGNITION \
#        DEBUG_VOTING \
#        += SAVE_TAG_IMAGE \
#        += DEBUG_OBJECT_TRACKING \
        NO_OCL \
#        += SHOW_PATTERN_NAME
#        DEBUG_BEHAVIOR_CLASSIFIER \
#        RECORD_BEHAVIOR_CLASSIFIER \


SOURCES += main.cpp\
        mainwindow.cpp \
    trajectory_tracking.cpp \
    tag_recognition.cpp \
    qsmartgraphicsview/qsmartgraphicsview.cpp \
    qsmartgraphicsview/qsmartlabel.cpp \
    object_tracking.cpp \
    math_function.cpp \
    dataprocesswindow.cpp \
    objecttrackingform.cpp \
    qcustomplot/qcustomplot.cpp \
    whitelist.cpp \
    mdl.cpp

HEADERS  += mainwindow.h \
    trajectory_tracking.h \
    tag_recognition.h \
    qsmartgraphicsview/qsmartgraphicsview.h \
    qsmartgraphicsview/qsmartlabel.h \
    object_tracking.h \
    math_function.h \
    dataprocesswindow.h \
    objecttrackingform.h \
    qcustomplot/qcustomplot.h \
    whitelist.h \
    mdl.h \
    parameters.h

FORMS    += mainwindow.ui \
    dataprocesswindow.ui \
    objecttrackingform.ui \
    whitelist.ui
win32 {
    msvc {
        QMAKE_CXXFLAGS += -openmp -arch:AVX -D "_CRT_SECURE_NO_WARNINGS"
        QMAKE_CXXFLAGS_RELEASE *= -O2
    }

    INCLUDEPATH += C:\\opencv300_vc2013\\include \
                   C:\\opencv300_vc2013\\include\\opencv \
                   C:\\opencv300_vc2013\\include\\opencv2 \
#                   "C:\\Program Files\\MATLAB\\MATLAB Production Server\\R2015a\\extern\\include"  \


    LIBS += -LC:\\opencv300_vc2013\\x64\\lib \
            -lopencv_world300 \
            -lopencv_ts300 \
            -lopencv_world300d \
            -lopencv_ts300d  \
#            "C:\\Program Files\\MATLAB\\MATLAB Production Server\\R2015a\\extern\\lib\\win64\\microsoft\\libeng.lib"  \
#            "C:\\Program Files\\MATLAB\\MATLAB Production Server\\R2015a\\extern\\lib\\win64\\microsoft\\libmx.lib"

}

unix {
    DEFINES += IN_UNIX

    INCLUDEPATH += /usr/local/include \
                    /usr/local/include/opencv \
                    /usr/local/include/opencv2

    LIBS += -L/usr/local/lib \
            -lopencv_calib3d \
            -lopencv_flann \
            -lopencv_imgproc \
            -lopencv_photo \
            -lopencv_superres \
            -lopencv_videostab \
            -lopencv_core \
            -lopencv_highgui \
            -lopencv_ml \
            -lopencv_shape \
            -lopencv_videoio \
            -lopencv_features2d \
            -lopencv_imgcodecs \
            -lopencv_objdetect \
            -lopencv_stitching \
            -lopencv_video

}
RESOURCES += \
    icon.qrc
