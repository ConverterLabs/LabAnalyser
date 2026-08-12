QT += testlib xml

# Real application graph without main.cpp: this keeps the UI caller test on
# the same libmatio and QObject ownership path as production.
include(../../../LabAnalyser.pro)
REPOSITORY_ROOT = $$clean_path($$PWD/../../..)
INHERITED_SOURCES = $$SOURCES
INHERITED_HEADERS = $$HEADERS
INHERITED_FORMS = $$FORMS
INHERITED_RESOURCES = $$RESOURCES
SOURCES =
HEADERS =
FORMS =
RESOURCES =
for(file, INHERITED_SOURCES): SOURCES += $$absolute_path($$file, $$REPOSITORY_ROOT)
for(file, INHERITED_HEADERS): HEADERS += $$absolute_path($$file, $$REPOSITORY_ROOT)
for(file, INHERITED_FORMS): FORMS += $$absolute_path($$file, $$REPOSITORY_ROOT)
for(file, INHERITED_RESOURCES): RESOURCES += $$absolute_path($$file, $$REPOSITORY_ROOT)
RC_FILE = $$absolute_path($$RC_FILE, $$REPOSITORY_ROOT)
TEMPLATE = app
TARGET = MatExportContractTests
CONFIG += testcase console
SOURCES -= $$REPOSITORY_ROOT/src/app/main.cpp
SOURCES += MatExportContractTests.cpp
INCLUDEPATH += $$REPOSITORY_ROOT $$REPOSITORY_ROOT/src $$REPOSITORY_ROOT/src/DropWidgets
