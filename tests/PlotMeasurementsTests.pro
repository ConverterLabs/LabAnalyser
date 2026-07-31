QT += core testlib

CONFIG += testcase console c++11
TEMPLATE = app
TARGET = PlotMeasurementsTests

INCLUDEPATH += ..

SOURCES += \
    PlotMeasurementsTests.cpp \
    ../DropWidgets/Plots/PlotMeasurements.cpp

HEADERS += \
    ../DropWidgets/Plots/PlotMeasurements.h
