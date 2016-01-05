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
#        += SAVE_TAG_IMAGE \
#        += DEBUG_OBJECT_TRACKING \
        += NO_OCL \
#        += SHOW_PATTERN_NAME

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
    qcustomplot/qcustomplot.cpp

HEADERS  += mainwindow.h \
    trajectory_tracking.h \
    tag_recognition.h \
    qsmartgraphicsview/qsmartgraphicsview.h \
    qsmartgraphicsview/qsmartlabel.h \
    object_tracking.h \
    math_function.h \
    dataprocesswindow.h \
    objecttrackingform.h \
    qcustomplot/qcustomplot.h

FORMS    += mainwindow.ui \
    dataprocesswindow.ui \
    objecttrackingform.ui

msvc {
  QMAKE_CXXFLAGS += -openmp -arch:AVX -D "_CRT_SECURE_NO_WARNINGS"
  QMAKE_CXXFLAGS_RELEASE *= -O2
}

INCLUDEPATH += C:\\opencv300_vc2013\\include \
               C:\\opencv300_vc2013\\include\\opencv \
               C:\\opencv300_vc2013\\include\\opencv2 \


LIBS +=  C:\\opencv300_vc2013\\x64\\lib\\opencv_world300.lib \
         C:\\opencv300_vc2013\\x64\\lib\\opencv_ts300.lib \
         C:\\opencv300_vc2013\\x64\\lib\\opencv_world300d.lib \
         C:\\opencv300_vc2013\\x64\\lib\\opencv_ts300d.lib

RESOURCES += \
    icon.qrc
