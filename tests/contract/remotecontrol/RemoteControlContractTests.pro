QT += core widgets network testlib
TEMPLATE = app
TARGET = RemoteControlContractTests
CONFIG += testcase console c++11

REPOSITORY_ROOT = $$clean_path($$PWD/../../..)
INCLUDEPATH += $$REPOSITORY_ROOT/tests/component/datamanagement $$REPOSITORY_ROOT $$REPOSITORY_ROOT/src $$REPOSITORY_ROOT/src/DropWidgets

SOURCES += \
    RemoteControlContractTests.cpp \
    $$REPOSITORY_ROOT/src/RemoteControl/RemoteControlConnectionState.cpp \
    $$REPOSITORY_ROOT/src/RemoteControl/RemoteControlFrameSplitter.cpp \
    $$REPOSITORY_ROOT/src/RemoteControl/RemoteControlProtocol.cpp \
    $$REPOSITORY_ROOT/src/RemoteControl/RemoteControlServer.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/ContainerStore.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DataRegistry.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DeviceRegistry.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/WidgetBindingRegistry.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/MessageDispatchPolicy.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DataManagementClass.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DataManagementSetClass.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DataMessengerClass.cpp \
    $$REPOSITORY_ROOT/src/plugins/InterfaceDataType.cpp

HEADERS += \
    $$REPOSITORY_ROOT/src/RemoteControl/RemoteControlConnectionState.h \
    $$REPOSITORY_ROOT/src/RemoteControl/RemoteControlFrameSplitter.h \
    $$REPOSITORY_ROOT/src/RemoteControl/RemoteControlProtocol.h \
    $$REPOSITORY_ROOT/src/RemoteControl/RemoteControlServer.h \
    $$REPOSITORY_ROOT/tests/component/datamanagement/mainwindow.h \
    $$REPOSITORY_ROOT/tests/component/datamanagement/DropWidgets/DropWidgets.h \
    $$REPOSITORY_ROOT/tests/component/datamanagement/DropWidgets/Plots/PlotWidget.h \
    $$REPOSITORY_ROOT/src/DataManagement/ContainerStore.h \
    $$REPOSITORY_ROOT/src/DataManagement/DataRegistry.h \
    $$REPOSITORY_ROOT/src/DataManagement/DeviceRegistry.h \
    $$REPOSITORY_ROOT/src/DataManagement/WidgetBindingRegistry.h \
    $$REPOSITORY_ROOT/src/DataManagement/MessageDispatchPolicy.h \
    $$REPOSITORY_ROOT/src/DataManagement/DataManagementClass.h \
    $$REPOSITORY_ROOT/src/DataManagement/DataManagementSetClass.h \
    $$REPOSITORY_ROOT/src/DataManagement/DataMessengerClass.h \
    $$REPOSITORY_ROOT/src/DataManagement/mapper.h \
    $$REPOSITORY_ROOT/src/plugins/InterfaceDataType.h
