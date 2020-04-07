#-------------------------------------------------
#
# Project created by QtCreator 2019-10-21T16:01:56
#
#-------------------------------------------------

QT += core gui widgets network websockets

TARGET = SimpleRemoteDesktop
TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += src

SOURCES += \
    src/serverhttp.cpp \
    src/graberclass.cpp \
    src/inputsimulator.cpp \
    src/websockettransfer.cpp \
    src/remotedesktopuniting.cpp \
    src/websockethandler.cpp

HEADERS += \
    src/serverhttp.h \
    src/graberclass.h \
    src/inputsimulator.h \
    src/websockettransfer.h \
    src/remotedesktopuniting.h \
    src/websockethandler.h

RESOURCES += \
    src/res.qrc

linux-g++: \
    LIBS += -lX11 -lXtst
