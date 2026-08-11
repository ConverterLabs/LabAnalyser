QT += core widgets
TEMPLATE = lib
CONFIG += plugin c++11
TARGET = HeapOwnedInterfacePlugin
REPOSITORY_ROOT = $$clean_path($$PWD/../../../../..)
INCLUDEPATH += $$REPOSITORY_ROOT
SOURCES += HeapOwnedInterfacePlugin.cpp
HEADERS += $$REPOSITORY_ROOT/plugins/platforminterface.h $$REPOSITORY_ROOT/plugins/InterfaceDataType.h
