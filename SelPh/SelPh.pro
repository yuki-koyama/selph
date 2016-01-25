#-------------------------------------------------
#
# Project created by QtCreator 2013-11-27T19:44:06
#
#-------------------------------------------------

QT += core gui opengl concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SelPh
TEMPLATE = app

CONFIG += \
    c++11

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    core.cpp \
    core_io.cpp \
    previewwidget.cpp \
    goodnessfunction.cpp \
    goodnessfunction_best-worst.cpp \
    studydata.cpp \
    metriclearning.cpp \
    imagewidget.cpp \
    ../common/visualizationwidget.cpp \
    ../common/image.cpp \
    ../common/imagemodifier.cpp \
    ../common/utility.cpp \
    ../common/colorutility.cpp \
    ../common/drawutility.cpp \
    ../common/histogram.cpp \
    ../common/mds.cpp

HEADERS += \
    mainwindow.h \
    core.h \
    previewwidget.h \
    goodnessfunction.h \
    studydata.h \
    metriclearning.h \
    imagewidget.h \
    ../common/visualizationwidget.h \
    ../common/image.h \
    ../common/imagemodifier.h \
    ../common/utility.h \
    ../common/eigenutility.h \
    ../common/colorutility.h \
    ../common/drawutility.h \
    ../common/histogram.h \
    ../common/mds.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += \
    ../include \
    ../common

LIBS += \
    -framework GLUT \
    $$PWD/../lib/libnlopt.a

QMAKE_CXXFLAGS = \
    -Wno-deprecated-register \
    -Wno-inconsistent-missing-override

DATA.files = \
    ../resources/data

DATA.path = \
    Contents/Resources

SHADER.files = \
    ../resources/shaders

SHADER.path = \
    Contents/Resources

QMAKE_BUNDLE_DATA += \
    DATA \
    SHADER
