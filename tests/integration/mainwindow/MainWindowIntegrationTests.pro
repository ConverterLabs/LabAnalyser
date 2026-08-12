QT += testlib xml

# Build the real application graph except production main.cpp. This target has
# no MainWindow seam: the tests exercise mainwindow.cpp, mainwindow.ui,
# resources and its immediate GUI dependencies exactly as the application does.
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
TARGET = MainWindowIntegrationTests
CONFIG += testcase console
SOURCES -= $$REPOSITORY_ROOT/src/app/main.cpp
SOURCES += MainWindowIntegrationTests.cpp
INCLUDEPATH += $$REPOSITORY_ROOT $$REPOSITORY_ROOT/src $$REPOSITORY_ROOT/src/DropWidgets
