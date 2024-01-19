#CONFIG -= qt

TEMPLATE = lib
DEFINES += AI3DSEGMENTLIB_LIBRARY

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

include(../../dental-scan-3rd-libs/onnxruntime/onnxruntime.pri)
include(../../dental-scan-3rd-libs/vcglib/vcg.pri)
include(../../dental-scan-3rd-libs/PCL_1.13.0/PCL_1.13.0_3rdParty.pri)
include(../../dental-scan-3rd-libs/cgco/cgco.pri)

include(../build.pri)

QMAKE_CXXFLAGS += /openmp
QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO += $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO -ZI -MD

win32:CONFIG(release, debug|release):{
    QMAKE_CFLAGS_RELEASE    += /arch:AVX2
    QMAKE_CXXFLAGS_RELEASE  += /arch:AVX2
}
else:win32:CONFIG(debug, debug|release):{
    QMAKE_CFLAGS_DEBUG      += /arch:AVX2
    QMAKE_CXXFLAGS_DEBUG    += /arch:AVX2
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Ai3DSegment.cpp \
    ModelProcess.cpp \
    VcgModel.cpp

HEADERS += \
    Ai3DSegment_global.h \
    Ai3DSegment.h \
    TeethColor.h \
    ModelProcess.h \
    nei_id.h \
    VcgModel.h




