QT += core testlib

CONFIG += testcase console c++11
TEMPLATE = app
TARGET = PlotMeasurementsTests

DEFINES += LABANALYSER_USE_FFTW
LIBS += -lfftw3

INCLUDEPATH += .. ../src

SOURCES += \
    PlotMeasurementsTests.cpp \
    ../src/DropWidgets/Plots/PlotMeasurements.cpp

HEADERS += \
    ../src/DropWidgets/Plots/PlotMeasurements.h
