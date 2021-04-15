#***************************************************************************
#*                                                                        **
#*  LabAnlyser, a plugin based data modification and visualization tool   **
#*  Copyright (C) 2015-2021 Andreas Hoffmann                              **
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
CONFIG += c++17
#CONFIG += force_debug_info

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 2

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"
#Target version
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = LabAnalyser
TEMPLATE = app

win32: INCLUDEPATH += C:/libraries/boost_1_59_0
unix: INCLUDEPATH += /usr/include
win32: INCLUDEPATH += C:/libraries/matOut-master
unix: INCLUDEPATH += ./build/libs/matOut
win32: INCLUDEPATH += C:/libraries/HighFive/include
unix: INCLUDEPATH += /usr/include/highfive/

INCLUDEPATH += C:/libraries/CMake-hdf5-1.10.5/hdf5-1.10.5/include/



SOURCES += main.cpp\
        DropWidgets/CreateID.cpp \
        DropWidgets/DropWidgetsUiLoader.cpp \
        DropWidgets/Plots/PlotWidget.cpp \
        DropWidgets/Plots/qcustomplot.cpp \
        DropWidgets/QBLed.cpp \
        DropWidgets/QCheckBox.cpp \
        DropWidgets/QComboBox.cpp \
        DropWidgets/QDoubleSpinBox.cpp \
        DropWidgets/QLCDNumber.cpp \
        DropWidgets/QLabel.cpp \
        DropWidgets/QLed.cpp \
        DropWidgets/QLineEdit.cpp \
        DropWidgets/QListView.cpp \
        DropWidgets/QProgressBar.cpp \
        DropWidgets/QPushButton.cpp \
        DropWidgets/QSlider.cpp \
        DropWidgets/QSpinBox.cpp \
        DropWidgets/QTSLed.cpp \
        Export/Export2Mat.cpp \
        DataManagement/UIDataManagementSetClass.cpp \
        Export/export2highfive.cpp \
        Export/exportinputs2xml.cpp \
        Import/parameterloader.cpp \
        LoadSave/loadplugin.cpp \
        LoadSave/xmlexperimentreader.cpp \
        LoadSave/xmlexperimentwriter.cpp \
        mainwindow.cpp\
        plugins/InterfaceDataType.cpp \
        DataManagement/DataManagementClass.cpp \
        DataManagement/DataManagementSetClass.cpp \
        DataManagement/DataMessengerClass.cpp \
        RemoteControl/RemoteControlServer.cpp \
        UIFunctions/SubPlotMainWindow.cpp \
        TreeWidgetCustomDrop.cpp\
        CustomWidgets/QBLedIndicator.cpp \
        CustomWidgets/QLedIndicator.cpp \
        CustomWidgets/QTSLedIndicator.cpp

HEADERS  += mainwindow.h\
            CustomWidgets/QBLedIndicator.h \
            CustomWidgets/QLedIndicator.h \
            CustomWidgets/QTSLedIndicator.h \
            DropWidgets/CreateID.h \
            DropWidgets/DropWidget.h \
            DropWidgets/DropWidgets.h \
            DropWidgets/DropWidget.h \
            DropWidgets/DropWidgetsUiLoader.h \
            DropWidgets/Plots/PlotWidget.h \
            DropWidgets/Plots/qcustomplot.h \
            DropWidgets/QBLed.h \
            DropWidgets/QCheckBox.h \
            DropWidgets/QComboBox.h \
            DropWidgets/QDoubleSpinBox.h \
            DropWidgets/QLCDNumber.h \
            DropWidgets/QLabel.h \
            DropWidgets/QLed.h \
            DropWidgets/QLineEdit.h \
            DropWidgets/QListView.h \
            DropWidgets/QProgressBar.h \
            DropWidgets/QPushButton.h \
            DropWidgets/QSlider.h \
            DropWidgets/QSpinBox.h \
            DropWidgets/QTSLed.h \
            Export/Export2Mat.h \
            DataManagement/UIDataManagementSetClass.h \
            Export/export2highfive.h \
            Export/exportinputs2xml.h \
            Import/parameterloader.h \
            LoadSave/loadplugin.h \
            LoadSave/xmlexperimentreader.h \
            LoadSave/xmlexperimentwriter.h \
            DataManagement/mapper.h \
            plugins/platforminterface.h\
            plugins/InterfaceDataType.h\
            DataManagement/DataManagementClass.h \
            DataManagement/DataManagementSetClass.h \
            DataManagement/DataMessengerClass.h \
            RemoteControl/RemoteControlServer.h \
            UIFunctions/SubPlotMainWindow.h \
            TreeWidgetCustomDrop.h

FORMS    += mainwindow.ui \
            About.ui
RC_FILE = LabAnlyser.rc

RESOURCES += \
    resources.qrc

#QMAKE_CXXFLAGS_RELEASE += -O1
QMAKE_CXXFLAGS -= -O2
QMAKE_CXXFLAGS -= -O1
QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS -= -O0

#QMAKE_CXXFLAGS += -O0
#QMAKE_CXXFLAGS -= -O1
#QMAKE_CXXFLAGS -= -O2
#QMAKE_CXXFLAGS -= -O3


LIBS += -LC:/libraries/CMake-hdf5-1.10.5/build/bin -lhdf5
Release:DESTDIR         = C:/LabAnalyser

#QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO
#QMAKE_CFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
#QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
