CONFIG -= qt

TEMPLATE = lib
DEFINES += TOOTHMODELLOADER_LIBRARY

CONFIG += c++17

include(../build.pri)
include(../../dental-scan-3rd-libs/PCL_1.13.0/PCL_1.13.0_3rdParty.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ToothModelLoader.cpp

HEADERS += \
    ToothModelLoader_global.h \
    ToothModelLoader.h

