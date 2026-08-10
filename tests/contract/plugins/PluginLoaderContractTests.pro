QT += core widgets uitools printsupport opengl openglwidgets network testlib
TEMPLATE = app
CONFIG += testcase console c++11
TARGET = PluginLoaderContractTests
REPOSITORY_ROOT = $$clean_path($$PWD/../../..)
INCLUDEPATH += $$REPOSITORY_ROOT
SOURCES += PluginLoaderContractTests.cpp DataManagementSetClassTestSeam.cpp $$REPOSITORY_ROOT/LoadSave/loadplugin.cpp $$REPOSITORY_ROOT/DataManagement/ContainerStore.cpp $$REPOSITORY_ROOT/DataManagement/DataRegistry.cpp $$REPOSITORY_ROOT/DataManagement/WidgetBindingRegistry.cpp $$REPOSITORY_ROOT/DataManagement/DataManagementClass.cpp $$REPOSITORY_ROOT/DataManagement/DataMessengerClass.cpp $$REPOSITORY_ROOT/plugins/InterfaceDataType.cpp
HEADERS += $$REPOSITORY_ROOT/LoadSave/loadplugin.h $$REPOSITORY_ROOT/DataManagement/ContainerStore.h $$REPOSITORY_ROOT/DataManagement/DataRegistry.h $$REPOSITORY_ROOT/DataManagement/WidgetBindingRegistry.h $$REPOSITORY_ROOT/DataManagement/DataManagementClass.h $$REPOSITORY_ROOT/DataManagement/DataManagementSetClass.h $$REPOSITORY_ROOT/DataManagement/DataMessengerClass.h $$REPOSITORY_ROOT/DataManagement/mapper.h $$REPOSITORY_ROOT/plugins/platforminterface.h $$REPOSITORY_ROOT/plugins/InterfaceDataType.h
