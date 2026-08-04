QT += core widgets uitools network printsupport testlib
TEMPLATE = app
CONFIG += testcase console c++11
TARGET = DropWidgetAdapterTests
REPOSITORY_ROOT = $$clean_path($$PWD/../../..)
INCLUDEPATH += $$REPOSITORY_ROOT
FORMS += $$REPOSITORY_ROOT/About.ui
SOURCES += DropWidgetAdapterTests.cpp TestMainWindowSeam.cpp PlotWidgetLinkSeam.cpp \
    $$REPOSITORY_ROOT/plugins/InterfaceDataType.cpp \
    $$REPOSITORY_ROOT/DataManagement/DataManagementClass.cpp \
    $$REPOSITORY_ROOT/DataManagement/DataMessengerClass.cpp \
    $$REPOSITORY_ROOT/DropWidgets/CreateID.cpp \
    $$REPOSITORY_ROOT/DropWidgets/DropWidgetsUiLoader.cpp $$REPOSITORY_ROOT/TreeWidgetCustomDrop.cpp \
    $$REPOSITORY_ROOT/DropWidgets/QBLed.cpp $$REPOSITORY_ROOT/DropWidgets/QCheckBox.cpp \
    $$REPOSITORY_ROOT/DropWidgets/QComboBox.cpp $$REPOSITORY_ROOT/DropWidgets/QDoubleSpinBox.cpp \
    $$REPOSITORY_ROOT/DropWidgets/QLCDNumber.cpp $$REPOSITORY_ROOT/DropWidgets/QLabel.cpp \
    $$REPOSITORY_ROOT/DropWidgets/QLed.cpp $$REPOSITORY_ROOT/DropWidgets/QLineEdit.cpp \
    $$REPOSITORY_ROOT/DropWidgets/QListView.cpp $$REPOSITORY_ROOT/DropWidgets/QProgressBar.cpp \
    $$REPOSITORY_ROOT/DropWidgets/QPushButton.cpp $$REPOSITORY_ROOT/DropWidgets/QSlider.cpp \
    $$REPOSITORY_ROOT/DropWidgets/QSpinBox.cpp $$REPOSITORY_ROOT/DropWidgets/QTableWidgeD.cpp \
    $$REPOSITORY_ROOT/DropWidgets/QTSLed.cpp \
    $$REPOSITORY_ROOT/CustomWidgets/QBLedIndicator.cpp $$REPOSITORY_ROOT/CustomWidgets/QLedIndicator.cpp \
    $$REPOSITORY_ROOT/CustomWidgets/QTSLedIndicator.cpp
HEADERS += $$REPOSITORY_ROOT/mainwindow.h \
    $$REPOSITORY_ROOT/DataManagement/UIDataManagementSetClass.h \
    $$REPOSITORY_ROOT/DataManagement/DataManagementSetClass.h \
    $$REPOSITORY_ROOT/DataManagement/DataManagementClass.h \
    $$REPOSITORY_ROOT/DataManagement/DataMessengerClass.h \
    $$REPOSITORY_ROOT/plugins/InterfaceDataType.h \
    $$REPOSITORY_ROOT/DropWidgets/QBLed.h $$REPOSITORY_ROOT/DropWidgets/QCheckBox.h \
    $$REPOSITORY_ROOT/DropWidgets/QComboBox.h $$REPOSITORY_ROOT/DropWidgets/QDoubleSpinBox.h \
    $$REPOSITORY_ROOT/DropWidgets/QLCDNumber.h $$REPOSITORY_ROOT/DropWidgets/QLabel.h \
    $$REPOSITORY_ROOT/DropWidgets/QLed.h $$REPOSITORY_ROOT/DropWidgets/QLineEdit.h \
    $$REPOSITORY_ROOT/DropWidgets/QListView.h $$REPOSITORY_ROOT/DropWidgets/QProgressBar.h \
    $$REPOSITORY_ROOT/DropWidgets/QPushButton.h $$REPOSITORY_ROOT/DropWidgets/QSlider.h \
    $$REPOSITORY_ROOT/DropWidgets/QSpinBox.h $$REPOSITORY_ROOT/DropWidgets/QTableWidgeD.h \
    $$REPOSITORY_ROOT/DropWidgets/QTSLed.h \
    $$REPOSITORY_ROOT/DropWidgets/DropWidgetsUiLoader.h $$REPOSITORY_ROOT/TreeWidgetCustomDrop.h \
    $$REPOSITORY_ROOT/CustomWidgets/QBLedIndicator.h $$REPOSITORY_ROOT/CustomWidgets/QLedIndicator.h \
    $$REPOSITORY_ROOT/CustomWidgets/QTSLedIndicator.h
