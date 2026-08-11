QT += core widgets network testlib

TEMPLATE = app
TARGET = RemoteControlContractTests
CONFIG += testcase console c++11

# Focused RemoteControl target. TCP_DM_001 also compiles the real
# DataManagement/Messenger path. The three existing DataManagement component
# test headers are test-only stubs for MainWindow and DropWidget symbols
# referenced by DataManagementSetClass.cpp; no stub is used for the Messenger,
# manager, mapper or container mutation under test.
# No production target includes this project or its test helpers.
REPOSITORY_ROOT = $$clean_path($$PWD/../../..)
INCLUDEPATH += \
    $$REPOSITORY_ROOT/tests/component/datamanagement \
    $$REPOSITORY_ROOT

SOURCES += \
    RemoteControlContractTests.cpp \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlConnectionState.cpp \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlFrameSplitter.cpp \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlProtocol.cpp \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlServer.cpp \
    $$REPOSITORY_ROOT/DataManagement/ContainerStore.cpp \
    $$REPOSITORY_ROOT/DataManagement/DataRegistry.cpp \
    $$REPOSITORY_ROOT/DataManagement/DeviceRegistry.cpp \
    $$REPOSITORY_ROOT/DataManagement/WidgetBindingRegistry.cpp \
    $$REPOSITORY_ROOT/DataManagement/MessageDispatchPolicy.cpp \
    $$REPOSITORY_ROOT/DataManagement/DataManagementClass.cpp \
    $$REPOSITORY_ROOT/DataManagement/DataManagementSetClass.cpp \
    $$REPOSITORY_ROOT/DataManagement/DataMessengerClass.cpp \
    $$REPOSITORY_ROOT/plugins/InterfaceDataType.cpp

HEADERS += \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlConnectionState.h \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlFrameSplitter.h \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlProtocol.h \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlServer.h \
    $$REPOSITORY_ROOT/tests/component/datamanagement/mainwindow.h \
    $$REPOSITORY_ROOT/tests/component/datamanagement/DropWidgets/DropWidgets.h \
    $$REPOSITORY_ROOT/tests/component/datamanagement/DropWidgets/Plots/PlotWidget.h \
    $$REPOSITORY_ROOT/DataManagement/ContainerStore.h \
    $$REPOSITORY_ROOT/DataManagement/DataRegistry.h \
    $$REPOSITORY_ROOT/DataManagement/DeviceRegistry.h \
    $$REPOSITORY_ROOT/DataManagement/WidgetBindingRegistry.h \
    $$REPOSITORY_ROOT/DataManagement/MessageDispatchPolicy.h \
    $$REPOSITORY_ROOT/DataManagement/DataManagementClass.h \
    $$REPOSITORY_ROOT/DataManagement/DataManagementSetClass.h \
    $$REPOSITORY_ROOT/DataManagement/DataMessengerClass.h \
    $$REPOSITORY_ROOT/DataManagement/mapper.h \
    $$REPOSITORY_ROOT/plugins/InterfaceDataType.h
