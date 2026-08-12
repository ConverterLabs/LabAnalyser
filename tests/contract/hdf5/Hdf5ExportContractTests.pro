QT += testlib widgets

# This project deliberately compiles only the HDF5 export boundary and its
# data model.  It never includes LabAnalyser.pro, so test-only seams cannot
# affect the production target.  Use qmake paths exclusively with '/'.
REPOSITORY_ROOT = $$clean_path($$PWD/../../..)
TEMPLATE = app
TARGET = Hdf5ExportContractTests
CONFIG += testcase console

SOURCES += \
    $$REPOSITORY_ROOT/src/Export/export2highfive.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/ContainerStore.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DataRegistry.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DeviceRegistry.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/WidgetBindingRegistry.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DataManagementClass.cpp \
    $$REPOSITORY_ROOT/src/plugins/InterfaceDataType.cpp \
    Hdf5ExportContractTests.cpp

HEADERS += \
    $$REPOSITORY_ROOT/src/Export/export2highfive.h \
    $$REPOSITORY_ROOT/src/DataManagement/DataManagementClass.h \
    $$REPOSITORY_ROOT/src/DataManagement/ContainerStore.h \
    $$REPOSITORY_ROOT/src/DataManagement/DataRegistry.h \
    $$REPOSITORY_ROOT/src/DataManagement/DeviceRegistry.h \
    $$REPOSITORY_ROOT/src/DataManagement/WidgetBindingRegistry.h \
    $$REPOSITORY_ROOT/src/DataManagement/mapper.h \
    $$REPOSITORY_ROOT/src/plugins/InterfaceDataType.h

INCLUDEPATH += $$REPOSITORY_ROOT $$REPOSITORY_ROOT/src $$REPOSITORY_ROOT/src/DropWidgets
LIBS += -lhdf5
