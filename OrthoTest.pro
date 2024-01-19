QT       += core gui concurrent



greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

include(../build.pri)

include(../../dental-scan-3rd-libs/PCL_1.13.0/PCL_1.13.0_3rdParty.pri)
include(../../dental-scan-3rd-libs/onnxruntime/onnxruntime.pri)
include(../ToothModelLoader/ToothModelLoaderLib/ModelLoader.pri)
include(../Ai3DSegment/AI3DSegmentLib/AI3DSegmentLib.pri)

LIBS += -lToothModelLoader -lAi3DSegment -L$$DESTDIR

QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO += $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO -ZI -MD


SOURCES += \
    BrushCallBack.cpp \
    MouseInteractorStyle.cpp \
    ToolMenuWidget.cpp \
    VTKOpenGLWidget.cpp \
    list.cpp \
    littlebutton.cpp \
    main.cpp \
    mainwindow.cpp \
    patientcase.cpp \
    regist.cpp \
    signin.cpp

HEADERS += \
    BrushCallBack.h \
    MouseInteractorStyle.h \
    ToolMenuWidget.h \
    VTKOpenGLWidget.h \
    list.h \
    littlebutton.h \
    mainwindow.h \
    patientcase.h \
    regist.h \
    signin.h

FORMS += \
    UI/MainWindow.ui \
    UI/ToolMenuWidget.ui \
    list.ui \
    littlebutton.ui \
    patientcase.ui \
    regist.ui \
    signin.ui

RESOURCES += \
    resources/res.qrc

DISTFILES += \
    1.blz-logo.png


