#-------------------------------------------------
#
# Project created by QtCreator 2017-02-18T14:53:28
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HandRecognition
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainview.cpp \
    handanalysis.cpp \
    settingdialog.cpp \
    trackingdialog.cpp

HEADERS  += mainview.h \
    qtcvimageconverter.h \
    handanalysis.h \
    settingdialog.h \
    trackingdialog.h

FORMS    += mainview.ui \
    settingdialog.ui \
    trackingdialog.ui

INCLUDEPATH += /usr/local/Cellar/opencv3/3.2.0/include
LIBS += -L/usr/local/Cellar/opencv3/3.2.0/lib -lopencv_imgproc -lopencv_core
