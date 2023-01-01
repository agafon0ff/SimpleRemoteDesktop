#-------------------------------------------------
#
# Project created by QtCreator 2019-10-21T16:01:56
#
#-------------------------------------------------

QT += core gui widgets network websockets

TARGET = SimpleRemoteDesktop
TEMPLATE = app
QMAKE_LFLAGS += -no-pie
QMAKE_CXXFLAGS_RELEASE += -O3
RC_FILE = src/res/icon.rc

#QMAKE_LFLAGS_RELEASE += -static -static-libgcc

CONFIG += c++17

INCLUDEPATH += src

SOURCES += \
    src/infowidget.cpp \
    src/serverhttp.cpp \
    src/graberclass.cpp \
    src/inputsimulator.cpp \
    src/websockettransfer.cpp \
    src/remotedesktopuniting.cpp \
    src/websockethandler.cpp

HEADERS += \
    src/infowidget.h \
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

# === build parameters ===
win32: OS_SUFFIX = win32
linux-g++: OS_SUFFIX = linux

CONFIG(debug, debug|release) {
    BUILD_FLAG = debug
    LIB_SUFFIX = d
} else {
    BUILD_FLAG = release
}

RCC_DIR = $${PWD}/build/$${BUILD_FLAG}
UI_DIR = $${PWD}/build/$${BUILD_FLAG}
UI_HEADERS_DIR = $${PWD}/build/$${BUILD_FLAG}
UI_SOURCES_DIR = $${PWD}/build/$${BUILD_FLAG}
MOC_DIR = $${PWD}/build/$${BUILD_FLAG}
OBJECTS_DIR = $${PWD}/build/$${BUILD_FLAG}
DESTDIR = $${PWD}/bin/$${BUILD_FLAG}
