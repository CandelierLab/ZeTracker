#-------------------------------------------------
#
# Project created by QtCreator 2019-03-29T14:02:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ZeTracker
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    Motion.cpp

HEADERS += \
        mainwindow.h \
    Motion.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# === Platform-specific libraries ==========================================

unix: LIBS += -L$$PWD/'../ftd2xx_linux/build/' -lftd2xx
unix: INCLUDEPATH += $$PWD/'../ftd2xx_linux'
unix: DEPENDPATH += $$PWD/'../ftd2xx_linux'

unix:!macx: LIBS += -L/usr/lib -lSpinnaker
unix:!macx: INCLUDEPATH += /usr/include/spinnaker

unix: INCLUDEPATH += /usr/local/include/opencv4/
unix: LIBS += -L/usr/local/lib64/ -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio -lopencv_video -lopencv_photo
