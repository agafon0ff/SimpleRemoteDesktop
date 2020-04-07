QT += core websockets
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

INCLUDEPATH += src

SOURCES += \
    src/proxyunitingclass.cpp \
    src/serverhttp.cpp \
    src/websockethandler.cpp \
    src/websockettransfer.cpp

HEADERS += \
    src/proxyunitingclass.h \
    src/serverhttp.h \
    src/websockethandler.h \
    src/websockettransfer.h

RESOURCES += \
    src/res.qrc
