QT += core websockets
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/proxyunitingclass.cpp \
    src/serverhttp.cpp \
    src/serverweb.cpp

HEADERS += \
    src/proxyunitingclass.h \
    src/serverhttp.h \
    src/serverweb.h

RESOURCES += \
    src/res.qrc
