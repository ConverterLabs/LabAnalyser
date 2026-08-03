QT += testlib xml

# Parameter import/export is also exposed through UIDataManagementSetClass.
# Use the unchanged application graph (without main.cpp), rather than seams,
# so the direct classes and their real UI-level callers share one contract.
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
TARGET = ParameterContractTests
CONFIG += testcase console
SOURCES -= $$REPOSITORY_ROOT/main.cpp
SOURCES += ParameterContractTests.cpp
INCLUDEPATH += $$REPOSITORY_ROOT
