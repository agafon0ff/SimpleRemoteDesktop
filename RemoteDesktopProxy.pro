QT += core websockets
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle
QMAKE_LFLAGS += -no-pie
QMAKE_CXXFLAGS_RELEASE += -O3

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

# === build parameters ===
win32: OS_SUFFIX = win32
linux-g++: OS_SUFFIX = linux

CONFIG(debug, debug|release) {
    BUILD_FLAG = debug
    LIB_SUFFIX = d
} else {
    BUILD_FLAG = release
}

RCC_DIR = $${PWD}/build/proxy/$${BUILD_FLAG}
UI_DIR = $${PWD}/build/proxy/$${BUILD_FLAG}
UI_HEADERS_DIR = $${PWD}/build/proxy/$${BUILD_FLAG}
UI_SOURCES_DIR = $${PWD}/build/proxy/$${BUILD_FLAG}
MOC_DIR = $${PWD}/build/proxy/$${BUILD_FLAG}
OBJECTS_DIR = $${PWD}/build/proxy/$${BUILD_FLAG}
DESTDIR = $${PWD}/bin/$${BUILD_FLAG}
