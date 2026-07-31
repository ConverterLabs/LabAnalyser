QT += core testlib

CONFIG += testcase console c++11
TEMPLATE = app
TARGET = PlotMeasurementsTests

DEFINES += LABANALYSER_USE_FFTW
LIBS += -lfftw3

INCLUDEPATH += ..

SOURCES += \
    PlotMeasurementsTests.cpp \
    ../DropWidgets/Plots/PlotMeasurements.cpp

HEADERS += \
    ../DropWidgets/Plots/PlotMeasurements.h
