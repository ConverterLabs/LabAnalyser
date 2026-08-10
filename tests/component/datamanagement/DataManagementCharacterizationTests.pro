QT += core testlib widgets

CONFIG += testcase console c++11
TEMPLATE = app
TARGET = DataManagementCharacterizationTests

INCLUDEPATH += . ../../..

SOURCES += \
    DataManagementCharacterizationTests.cpp \
    ../../../DataManagement/ContainerStore.cpp \
    ../../../DataManagement/DataRegistry.cpp \
    ../../../DataManagement/WidgetBindingRegistry.cpp \
    ../../../DataManagement/MessageDispatchPolicy.cpp \
    ../../../DataManagement/DataManagementClass.cpp \
    ../../../DataManagement/DataManagementSetClass.cpp \
    ../../../DataManagement/DataMessengerClass.cpp \
    ../../../plugins/InterfaceDataType.cpp

HEADERS += \
    mainwindow.h \
    DropWidgets/DropWidgets.h \
    DropWidgets/Plots/PlotWidget.h \
    ../../../DataManagement/DataManagementClass.h \
    ../../../DataManagement/ContainerStore.h \
    ../../../DataManagement/DataRegistry.h \
    ../../../DataManagement/WidgetBindingRegistry.h \
    ../../../DataManagement/MessageDispatchPolicy.h \
    ../../../DataManagement/DataManagementSetClass.h \
    ../../../DataManagement/DataMessengerClass.h \
    ../../../DataManagement/mapper.h \
    ../../../plugins/InterfaceDataType.h \
    ../../../plugins/platforminterface.h
