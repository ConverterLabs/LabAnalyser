QT += core widgets
TEMPLATE = lib
CONFIG += plugin c++11
TARGET = MemberOwnedInterfacePlugin
REPOSITORY_ROOT = $$clean_path($$PWD/../../../../..)
INCLUDEPATH += $$REPOSITORY_ROOT $$REPOSITORY_ROOT/src $$REPOSITORY_ROOT/src/DropWidgets
SOURCES += MemberOwnedInterfacePlugin.cpp
HEADERS += $$REPOSITORY_ROOT/src/plugins/platforminterface.h $$REPOSITORY_ROOT/src/plugins/InterfaceDataType.h
