#-------------------------------------------------
#
# Project created by QtCreator 2019-10-21T16:01:56
#
#-------------------------------------------------

QT += core gui widgets network websockets
QT += winextras

TARGET = SimpleRemoteDesktop
TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/serverhttp.cpp \
    src/serverweb.cpp \
    src/unitingclass.cpp \
    src/graberclass.cpp \
    src/dataparser.cpp \
    src/inputsimulator.cpp

HEADERS += \
    src/serverhttp.h \
    src/serverweb.h \
    src/unitingclass.h \
    src/graberclass.h \
    src/dataparser.h \
    src/inputsimulator.h

RESOURCES += \
    src/res.qrc

linux-g++: LIBS += -lX11 -lXtst
