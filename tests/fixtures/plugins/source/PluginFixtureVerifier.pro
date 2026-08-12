QT += core
TEMPLATE = app
CONFIG += console c++11
TARGET = PluginFixtureVerifier
REPOSITORY_ROOT = $$clean_path($$PWD/../../../..)
INCLUDEPATH += $$REPOSITORY_ROOT $$REPOSITORY_ROOT/src $$REPOSITORY_ROOT/src/DropWidgets
SOURCES += PluginFixtureVerifier.cpp
HEADERS += $$REPOSITORY_ROOT/src/plugins/platforminterface.h $$REPOSITORY_ROOT/src/plugins/InterfaceDataType.h
