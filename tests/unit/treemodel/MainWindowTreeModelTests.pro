QT += core testlib widgets

CONFIG += testcase console c++17
TEMPLATE = app
TARGET = MainWindowTreeModelTests

INCLUDEPATH += . ../../.. ../../../src

SOURCES += \
    MainWindowTreeModelTests.cpp \
    ../../../src/UIFunctions/MainWindowTreeModel.cpp \
    ../../../src/plugins/InterfaceDataType.cpp

HEADERS += \
    ../../../src/UIFunctions/MainWindowTreeModel.h \
    ../../../src/plugins/InterfaceDataType.h
