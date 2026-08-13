#***************************************************************************
#*                                                                        **
#*  LabAnlyser, a plugin based data modification and visualization tool   **
#*  Copyright (C) 2015-2023 Andreas Hoffmann                              **
#*                                                                        **
#*  LabAnlyser is free software: you can redistribute it and/or modify ´  **
#*  it under the terms of the GNU General Public License as published by  **
#*  the Free Software Foundation, either version 3 of the License, or     **
#*  (at your option) any later version.                                   **
#*                                                                        **
#*  This program is distributed in the hope that it will be useful,       **
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
#*  GNU General Public License for more details.                          **
#*                                                                        **
#*  You should have received a copy of the GNU General Public License     **
#*  along with this program.  If not, see http://www.gnu.org/licenses/.   **
#*                                                                        **
#**************************************************************************
#***************************************************************************/

QT       += core gui
QT       += uitools
QT       += network
#CONFIG += c++17
#CONFIG += force_debug_info

VERSION_MAJOR = 1
VERSION_MINOR = 1
VERSION_BUILD = 1

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"
DEFINES += LABANALYSER_USE_FFTW
#Target version
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = LabAnalyser
TEMPLATE = app


unix: INCLUDEPATH += /usr/include
INCLUDEPATH += $$PWD/src $$PWD/src/app $$PWD/src/DropWidgets
LIBS += -lmatio


SOURCES += src/app/main.cpp\
        src/DropWidgets/CreateID.cpp \
        src/DropWidgets/DropWidgetsUiLoader.cpp \
        src/DropWidgets/DropWidgetBinding.cpp \
        src/DropWidgets/DropWidgetConnectionMenu.cpp \
        src/DropWidgets/DropWidgetTableCells.cpp \
        src/DropWidgets/Plots/FFTPlotWidget.cpp \
        src/DropWidgets/Plots/PlotMeasurements.cpp \
        src/DropWidgets/Plots/PlotWidget.cpp \
        src/DropWidgets/Plots/qcustomplot.cpp \
        src/DropWidgets/QBLed.cpp \
        src/DropWidgets/QCheckBox.cpp \
        src/DropWidgets/QComboBox.cpp \
        src/DropWidgets/QDoubleSpinBox.cpp \
        src/DropWidgets/QLCDNumber.cpp \
        src/DropWidgets/QLabel.cpp \
        src/DropWidgets/QLed.cpp \
        src/DropWidgets/QLineEdit.cpp \
        src/DropWidgets/QListView.cpp \
        src/DropWidgets/QProgressBar.cpp \
        src/DropWidgets/QPushButton.cpp \
        src/DropWidgets/QSlider.cpp \
        src/DropWidgets/QSpinBox.cpp \
        src/DropWidgets/QTSLed.cpp \
        src/DropWidgets/QTableWidgeD.cpp \
        src/Export/Export2Mat.cpp \
        src/Export/LabDataArchive.cpp \
        src/DataManagement/UIDataManagementSetClass.cpp \
        src/Export/export2highfive.cpp \
        src/Export/exportinputs2xml.cpp \
        src/Import/parameterloader.cpp \
        src/Import/MatDataImport.cpp \
        src/LoadSave/loadplugin.cpp \
        src/LoadSave/PluginLeasePool.cpp \
        src/LoadSave/XmlFigureDimensions.cpp \
        src/LoadSave/xmlexperimentreader.cpp \
        src/LoadSave/xmlexperimentwriter.cpp \
        src/app/mainwindow.cpp\
        src/plugins/InterfaceDataType.cpp \
        src/DataManagement/ContainerStore.cpp \
        src/DataManagement/DataRegistry.cpp \
        src/DataManagement/DeviceRegistry.cpp \
        src/DataManagement/WidgetBindingRegistry.cpp \
        src/DataManagement/MessageDispatchPolicy.cpp \
        src/DataManagement/ProjectIoCoordinator.cpp \
        src/DataManagement/DataManagementClass.cpp \
        src/DataManagement/DataManagementSetClass.cpp \
        src/DataManagement/DataMessengerClass.cpp \
        src/RemoteControl/RemoteControlConnectionState.cpp \
        src/RemoteControl/RemoteControlFrameSplitter.cpp \
        src/RemoteControl/RemoteControlProtocol.cpp \
        src/RemoteControl/RemoteControlServer.cpp \
        src/UIFunctions/MainWindowOutputLog.cpp \
        src/UIFunctions/MainWindowTreePath.cpp \
        src/UIFunctions/MainWindowTreeViewState.cpp \
        src/UIFunctions/MainWindowSubplotDialog.cpp \
        src/UIFunctions/MainWindowFormLoader.cpp \
        src/UIFunctions/MainWindowTreeModel.cpp \
        src/UIFunctions/MainWindowExplorerValues.cpp \
        src/UIFunctions/MainWindowFigureFactory.cpp \
        src/UIFunctions/MainWindowDockPresentation.cpp \
        src/UIFunctions/MainWindowTrayController.cpp \
        src/UIFunctions/MainWindowProjectCleanup.cpp \
        src/UIFunctions/MainWindowContextMenus.cpp \
        src/UIFunctions/MainWindowProjectActions.cpp \
        src/UIFunctions/SubPlotMainWindow.cpp \
        src/DropWidgets/TreeWidgetCustomDrop.cpp\
        src/CustomWidgets/QBLedIndicator.cpp \
        src/CustomWidgets/QLedIndicator.cpp \
        src/CustomWidgets/QTSLedIndicator.cpp

HEADERS  += src/app/mainwindow.h\
            src/UIFunctions/MainWindowOutputLog.h \
            src/Export/LabDataArchive.h \
            src/Import/MatDataImport.h \
            src/UIFunctions/MainWindowTreePath.h \
            src/UIFunctions/MainWindowTreeViewState.h \
            src/UIFunctions/MainWindowSubplotDialog.h \
            src/UIFunctions/MainWindowFormLoader.h \
            src/UIFunctions/MainWindowTreeModel.h \
            src/UIFunctions/MainWindowExplorerValues.h \
            src/UIFunctions/MainWindowFigureFactory.h \
            src/UIFunctions/MainWindowDockPresentation.h \
            src/UIFunctions/MainWindowTrayController.h \
            src/UIFunctions/MainWindowProjectCleanup.h \
            src/UIFunctions/MainWindowContextMenus.h \
            src/UIFunctions/MainWindowProjectActions.h \
            src/CustomWidgets/QBLedIndicator.h \
            src/CustomWidgets/QLedIndicator.h \
            src/CustomWidgets/QTSLedIndicator.h \
            src/DropWidgets/CreateID.h \
            src/DropWidgets/DropWidget.h \
            src/DropWidgets/DropWidgetBinding.h \
            src/DropWidgets/DropWidgetConnectionMenu.h \
            src/DropWidgets/DropWidgetDataAccess.h \
            src/DropWidgets/DropWidgetDropBinding.h \
            src/DropWidgets/DropWidgetDragSource.h \
            src/DropWidgets/DropWidgetIndicatorBinding.h \
            src/DropWidgets/DropWidgetTreePath.h \
            src/DropWidgets/DropWidgetTableCells.h \
            src/DropWidgets/DropWidgetUpdate.h \
            src/DropWidgets/DropWidgets.h \
            src/DropWidgets/DropWidget.h \
            src/DropWidgets/DropWidgetsUiLoader.h \
            src/DropWidgets/Plots/FFTPlotWidget.h \
            src/DropWidgets/Plots/PlotMeasurements.h \
            src/DropWidgets/Plots/PlotWidget.h \
            src/DropWidgets/Plots/qcustomplot.h \
            src/DropWidgets/QBLed.h \
            src/DropWidgets/QCheckBox.h \
            src/DropWidgets/QComboBox.h \
            src/DropWidgets/QDoubleSpinBox.h \
            src/DropWidgets/QLCDNumber.h \
            src/DropWidgets/QLabel.h \
            src/DropWidgets/QLed.h \
            src/DropWidgets/QLineEdit.h \
            src/DropWidgets/QListView.h \
            src/DropWidgets/QProgressBar.h \
            src/DropWidgets/QPushButton.h \
            src/DropWidgets/QSlider.h \
            src/DropWidgets/QSpinBox.h \
            src/DropWidgets/QTSLed.h \
            src/DropWidgets/QTableWidgeD.h \
            src/Export/Export2Mat.h \
            src/DataManagement/UIDataManagementSetClass.h \
            src/Export/export2highfive.h \
            src/Export/exportinputs2xml.h \
            src/Import/parameterloader.h \
            src/LoadSave/loadplugin.h \
            src/LoadSave/PluginLeasePool.h \
            src/LoadSave/XmlFigureDimensions.h \
            src/LoadSave/xmlexperimentreader.h \
            src/LoadSave/xmlexperimentwriter.h \
            src/DataManagement/mapper.h \
            src/plugins/platforminterface.h\
            src/plugins/InterfaceDataType.h\
            src/DataManagement/ContainerStore.h \
            src/DataManagement/DataRegistry.h \
            src/DataManagement/DeviceRegistry.h \
            src/DataManagement/WidgetBindingRegistry.h \
            src/DataManagement/MessageDispatchPolicy.h \
            src/DataManagement/ProjectIoCoordinator.h \
            src/DataManagement/DataManagementClass.h \
            src/DataManagement/DataManagementSetClass.h \
            src/DataManagement/DataMessengerClass.h \
            src/RemoteControl/RemoteControlConnectionState.h \
            src/RemoteControl/RemoteControlFrameSplitter.h \
            src/RemoteControl/RemoteControlProtocol.h \
            src/RemoteControl/RemoteControlServer.h \
            src/UIFunctions/SubPlotMainWindow.h \
            src/DropWidgets/TreeWidgetCustomDrop.h

FORMS    += src/app/mainwindow.ui \
            src/app/About.ui
RC_FILE = resources/LabAnlyser.rc

RESOURCES += \
    resources/resources.qrc

QMAKE_CXXFLAGS -= -O2
QMAKE_CXXFLAGS -= -O1
QMAKE_CXXFLAGS -= -O3
#QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS += -Werror=return-type
QMAKE_CXXFLAGS += -O0
QMAKE_CXXFLAGS_RELEASE += -O3

#QMAKE_CXXFLAGS += -no-opengl



#QMAKE_CXXFLAGS += -O0
#QMAKE_CXXFLAGS -= -O1
#QMAKE_CXXFLAGS -= -O2
#QMAKE_CXXFLAGS -= -O3


win32: LIBS += -LC:/libraries/CMake-hdf5-1.10.5/build/bin -lhdf5 -lfftw3
unix: LIBS += -lhdf5 -lfftw3

#QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO
#QMAKE_CFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
#QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
