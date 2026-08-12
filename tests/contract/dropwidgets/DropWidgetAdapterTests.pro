QT += core widgets uitools network printsupport testlib
TEMPLATE = app
CONFIG += testcase console c++11
TARGET = DropWidgetAdapterTests
REPOSITORY_ROOT = $$clean_path($$PWD/../../..)
INCLUDEPATH += $$REPOSITORY_ROOT $$REPOSITORY_ROOT/src $$REPOSITORY_ROOT/src/DropWidgets $$REPOSITORY_ROOT/src/app
FORMS += $$REPOSITORY_ROOT/src/app/About.ui
SOURCES += DropWidgetAdapterTests.cpp TestMainWindowSeam.cpp PlotWidgetLinkSeam.cpp \
    $$REPOSITORY_ROOT/src/plugins/InterfaceDataType.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/ContainerStore.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DataRegistry.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DeviceRegistry.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/WidgetBindingRegistry.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/MessageDispatchPolicy.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DataManagementClass.cpp \
    $$REPOSITORY_ROOT/src/DataManagement/DataMessengerClass.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/CreateID.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetBinding.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetConnectionMenu.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetTableCells.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetsUiLoader.cpp $$REPOSITORY_ROOT/src/DropWidgets/TreeWidgetCustomDrop.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/QBLed.cpp $$REPOSITORY_ROOT/src/DropWidgets/QCheckBox.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/QComboBox.cpp $$REPOSITORY_ROOT/src/DropWidgets/QDoubleSpinBox.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/QLCDNumber.cpp $$REPOSITORY_ROOT/src/DropWidgets/QLabel.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/QLed.cpp $$REPOSITORY_ROOT/src/DropWidgets/QLineEdit.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/QListView.cpp $$REPOSITORY_ROOT/src/DropWidgets/QProgressBar.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/QPushButton.cpp $$REPOSITORY_ROOT/src/DropWidgets/QSlider.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/QSpinBox.cpp $$REPOSITORY_ROOT/src/DropWidgets/QTableWidgeD.cpp \
    $$REPOSITORY_ROOT/src/DropWidgets/QTSLed.cpp \
    $$REPOSITORY_ROOT/src/CustomWidgets/QBLedIndicator.cpp $$REPOSITORY_ROOT/src/CustomWidgets/QLedIndicator.cpp \
    $$REPOSITORY_ROOT/src/CustomWidgets/QTSLedIndicator.cpp
HEADERS += $$REPOSITORY_ROOT/src/app/mainwindow.h \
    $$REPOSITORY_ROOT/src/DataManagement/UIDataManagementSetClass.h \
    $$REPOSITORY_ROOT/src/DataManagement/DataManagementSetClass.h \
    $$REPOSITORY_ROOT/src/DataManagement/DataManagementClass.h \
    $$REPOSITORY_ROOT/src/DataManagement/ContainerStore.h \
    $$REPOSITORY_ROOT/src/DataManagement/DataRegistry.h \
    $$REPOSITORY_ROOT/src/DataManagement/DeviceRegistry.h \
    $$REPOSITORY_ROOT/src/DataManagement/WidgetBindingRegistry.h \
    $$REPOSITORY_ROOT/src/DataManagement/MessageDispatchPolicy.h \
    $$REPOSITORY_ROOT/src/DataManagement/DataMessengerClass.h \
    $$REPOSITORY_ROOT/src/plugins/InterfaceDataType.h \
    $$REPOSITORY_ROOT/src/DropWidgets/QBLed.h $$REPOSITORY_ROOT/src/DropWidgets/QCheckBox.h \
    $$REPOSITORY_ROOT/src/DropWidgets/QComboBox.h $$REPOSITORY_ROOT/src/DropWidgets/QDoubleSpinBox.h \
    $$REPOSITORY_ROOT/src/DropWidgets/QLCDNumber.h $$REPOSITORY_ROOT/src/DropWidgets/QLabel.h \
    $$REPOSITORY_ROOT/src/DropWidgets/QLed.h $$REPOSITORY_ROOT/src/DropWidgets/QLineEdit.h \
    $$REPOSITORY_ROOT/src/DropWidgets/QListView.h $$REPOSITORY_ROOT/src/DropWidgets/QProgressBar.h \
    $$REPOSITORY_ROOT/src/DropWidgets/QPushButton.h $$REPOSITORY_ROOT/src/DropWidgets/QSlider.h \
    $$REPOSITORY_ROOT/src/DropWidgets/QSpinBox.h $$REPOSITORY_ROOT/src/DropWidgets/QTableWidgeD.h \
    $$REPOSITORY_ROOT/src/DropWidgets/QTSLed.h \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetBinding.h \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetConnectionMenu.h \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetDataAccess.h \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetDropBinding.h \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetDragSource.h \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetIndicatorBinding.h \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetTreePath.h \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetTableCells.h \
    $$REPOSITORY_ROOT/src/DropWidgets/DropWidgetsUiLoader.h $$REPOSITORY_ROOT/src/DropWidgets/TreeWidgetCustomDrop.h \
    $$REPOSITORY_ROOT/src/CustomWidgets/QBLedIndicator.h $$REPOSITORY_ROOT/src/CustomWidgets/QLedIndicator.h \
    $$REPOSITORY_ROOT/src/CustomWidgets/QTSLedIndicator.h
