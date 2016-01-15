#-------------------------------------------------
#
# Project created by QtCreator 2015-03-08T13:19:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FlyDetector
TEMPLATE = app

CONFIG += c++11

QMAKE_CXXFLAGS+= -fopenmp
QMAKE_LFLAGS +=  -fopenmp

SOURCES += main.cpp\
        mainwindow.cpp \
    flycounter.cpp \
    samplecam.cpp \
    webcamera.cpp \
    reflexcam.cpp \
    flycam.cpp \
    dbscan/hpdbscan.cpp \
    dbscan/points.cpp \
    dbscan/rules.cpp \
    dbscan/space.cpp \
    shaker.cpp

HEADERS  += mainwindow.h \
    flycounter.h \
    samplecam.h \
    webcamera.h \
    reflexcam.h \
    flycam.h \
    dbscan/constants.h \
    dbscan/hpdbscan.h \
    dbscan/points.h \
    dbscan/rules.h \
    dbscan/space.h \
    dbscan/util.h \
    shaker.h

FORMS    += mainwindow.ui

LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_video
LIBS += -lgphoto2
LIBS += -lusb-1.0

