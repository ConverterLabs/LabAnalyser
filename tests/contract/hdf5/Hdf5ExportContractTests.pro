QT += testlib widgets

# This project deliberately compiles only the HDF5 export boundary and its
# data model.  It never includes LabAnalyser.pro, so test-only seams cannot
# affect the production target.  Use qmake paths exclusively with '/'.
REPOSITORY_ROOT = $$clean_path($$PWD/../../..)
TEMPLATE = app
TARGET = Hdf5ExportContractTests
CONFIG += testcase console

SOURCES += \
    $$REPOSITORY_ROOT/Export/export2highfive.cpp \
    $$REPOSITORY_ROOT/DataManagement/DataManagementClass.cpp \
    $$REPOSITORY_ROOT/plugins/InterfaceDataType.cpp \
    Hdf5ExportContractTests.cpp

HEADERS += \
    $$REPOSITORY_ROOT/Export/export2highfive.h \
    $$REPOSITORY_ROOT/DataManagement/DataManagementClass.h \
    $$REPOSITORY_ROOT/DataManagement/mapper.h \
    $$REPOSITORY_ROOT/plugins/InterfaceDataType.h

INCLUDEPATH += $$REPOSITORY_ROOT
LIBS += -lhdf5
