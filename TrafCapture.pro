#-------------------------------------------------
#
# Project created by QtCreator 2016-01-20T10:35:45
#
#-------------------------------------------------

QT       += core gui
linux: QMAKE_CXXFLAGS += -std=gnu++0x

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = TrafCapture
TEMPLATE = app

win32: INCLUDEPATH += WinPCap/Include

SOURCES += main.cpp\
        MainWindow.cpp \
    Capture.cpp \
    TLV.cpp

HEADERS  += MainWindow.h \
    Capture.h \
    CaptureSettings.h \
    SvDecoded.h \
    TLV.h

FORMS    += MainWindow.ui


win32: LIBS += -L../TrafCapture/WinPCap/Lib -lwpcap
linux: LIBS += -lpcap
