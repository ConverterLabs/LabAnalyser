QT += core widgets
TEMPLATE = lib
CONFIG += plugin c++11
TARGET = CompatiblePlugin
REPOSITORY_ROOT = $$clean_path($$PWD/../../../../..)
INCLUDEPATH += $$REPOSITORY_ROOT
SOURCES += CompatiblePlugin.cpp
HEADERS += $$REPOSITORY_ROOT/plugins/platforminterface.h $$REPOSITORY_ROOT/plugins/InterfaceDataType.h
