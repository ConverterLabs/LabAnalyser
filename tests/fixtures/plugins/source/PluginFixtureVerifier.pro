QT += core
TEMPLATE = app
CONFIG += console c++11
TARGET = PluginFixtureVerifier
REPOSITORY_ROOT = $$clean_path($$PWD/../../../..)
INCLUDEPATH += $$REPOSITORY_ROOT
SOURCES += PluginFixtureVerifier.cpp
HEADERS += $$REPOSITORY_ROOT/plugins/platforminterface.h $$REPOSITORY_ROOT/plugins/InterfaceDataType.h
