QT += core widgets network testlib

TEMPLATE = app
TARGET = RemoteControlContractTests
CONFIG += testcase console c++11

# Focused protocol target: the production server and its data value type only.
# No production target includes this project or its test helpers.
REPOSITORY_ROOT = $$clean_path($$PWD/../../..)
INCLUDEPATH += $$REPOSITORY_ROOT

SOURCES += \
    RemoteControlContractTests.cpp \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlConnectionState.cpp \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlFrameSplitter.cpp \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlProtocol.cpp \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlServer.cpp \
    $$REPOSITORY_ROOT/plugins/InterfaceDataType.cpp

HEADERS += \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlConnectionState.h \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlFrameSplitter.h \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlProtocol.h \
    $$REPOSITORY_ROOT/RemoteControl/RemoteControlServer.h \
    $$REPOSITORY_ROOT/DataManagement/mapper.h \
    $$REPOSITORY_ROOT/plugins/InterfaceDataType.h
