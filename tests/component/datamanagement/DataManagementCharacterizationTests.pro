QT += core testlib widgets

CONFIG += testcase console c++11
TEMPLATE = app
TARGET = DataManagementCharacterizationTests

INCLUDEPATH += . ../../..

SOURCES += \
    DataManagementCharacterizationTests.cpp \
    ../../../DataManagement/DataRegistry.cpp \
    ../../../DataManagement/DataManagementClass.cpp \
    ../../../DataManagement/DataManagementSetClass.cpp \
    ../../../DataManagement/DataMessengerClass.cpp \
    ../../../plugins/InterfaceDataType.cpp

HEADERS += \
    mainwindow.h \
    DropWidgets/DropWidgets.h \
    DropWidgets/Plots/PlotWidget.h \
    ../../../DataManagement/DataManagementClass.h \
    ../../../DataManagement/DataRegistry.h \
    ../../../DataManagement/DataManagementSetClass.h \
    ../../../DataManagement/DataMessengerClass.h \
    ../../../DataManagement/mapper.h \
    ../../../plugins/InterfaceDataType.h \
    ../../../plugins/platforminterface.h
