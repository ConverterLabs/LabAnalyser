QT += testlib xml

# The XML reader and writer explicitly traverse their QObject parent chain and
# qobject_cast it to MainWindow.  Build the unchanged application graph (minus
# its production main.cpp) so these contracts are exercised without test-only
# replacement headers or seams.
include(../../../LabAnalyser.pro)

# LabAnalyser.pro lists paths relative to its own directory. qmake keeps the
# including test project's directory as PWD, so normalize every inherited path
# back to the repository root after inclusion.
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
TARGET = XmlExperimentContractTests
CONFIG += testcase console
SOURCES -= $$REPOSITORY_ROOT/main.cpp
SOURCES += XmlExperimentContractTests.cpp
INCLUDEPATH += $$REPOSITORY_ROOT
