QT += testlib xml

# Exercise the real PlotWidget/DataManagement/Messenger graph. qcustomplot is
# inherited as a vendored runtime dependency; this target neither changes nor
# tests its implementation in isolation.
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
TARGET = PlotWidgetContractTests
CONFIG += testcase console
SOURCES -= $$REPOSITORY_ROOT/main.cpp
SOURCES += PlotWidgetContractTests.cpp
INCLUDEPATH += $$REPOSITORY_ROOT
