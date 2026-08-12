QT += core testlib widgets

CONFIG += testcase console c++11
TEMPLATE = app
TARGET = DataManagementCharacterizationTests

INCLUDEPATH += . ../../.. ../../../src

SOURCES += \
    DataManagementCharacterizationTests.cpp \
    ../../../src/DataManagement/ContainerStore.cpp \
    ../../../src/DataManagement/DataRegistry.cpp \
    ../../../src/DataManagement/DeviceRegistry.cpp \
    ../../../src/DataManagement/WidgetBindingRegistry.cpp \
    ../../../src/DataManagement/MessageDispatchPolicy.cpp \
    ../../../src/DataManagement/DataManagementClass.cpp \
    ../../../src/DataManagement/DataManagementSetClass.cpp \
    ../../../src/DataManagement/DataMessengerClass.cpp \
    ../../../src/plugins/InterfaceDataType.cpp

HEADERS += \
    mainwindow.h \
    DropWidgets/DropWidgets.h \
    DropWidgets/Plots/PlotWidget.h \
    ../../../src/DataManagement/DataManagementClass.h \
    ../../../src/DataManagement/ContainerStore.h \
    ../../../src/DataManagement/DataRegistry.h \
    ../../../src/DataManagement/DeviceRegistry.h \
    ../../../src/DataManagement/WidgetBindingRegistry.h \
    ../../../src/DataManagement/MessageDispatchPolicy.h \
    ../../../src/DataManagement/DataManagementSetClass.h \
    ../../../src/DataManagement/DataMessengerClass.h \
    ../../../src/DataManagement/mapper.h \
    ../../../src/plugins/InterfaceDataType.h \
    ../../../src/plugins/platforminterface.h
