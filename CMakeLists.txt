cmake_minimum_required(VERSION 3.21)

project(LabAnalyser VERSION 1.1.1 LANGUAGES CXX)

# The CMake build is intentionally additive.  LabAnalyser.pro and all qmake
# test projects remain supported compatibility build inputs.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

option(LABANALYSER_BUILD_TESTS "Build LabAnalyser CTest targets" ON)
option(LABANALYSER_BUILD_MATLAB_CONNECTOR "Build the Windows MATLAB TCP connector" ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Xml UiTools PrintSupport OpenGL OpenGLWidgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MATIO REQUIRED IMPORTED_TARGET matio)
pkg_check_modules(FFTW3 REQUIRED IMPORTED_TARGET fftw3)
pkg_check_modules(HDF5 REQUIRED IMPORTED_TARGET hdf5)
find_package(Boost REQUIRED)
# MSYS2 packages HighFive as headers beside HDF5 rather than as a CMake
# package.  Use the HDF5 pkg-config include prefix, not a machine path.
find_path(HIGHFIVE_INCLUDE_DIR highfive/H5File.hpp HINTS ${HDF5_INCLUDE_DIRS} REQUIRED)

execute_process(
    COMMAND git describe --always --tags
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE LABANALYSER_GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
if(NOT LABANALYSER_GIT_VERSION)
    set(LABANALYSER_GIT_VERSION "unknown")
endif()

add_library(LabAnalyserQCustomPlot STATIC
    DropWidgets/Plots/qcustomplot.cpp
    DropWidgets/Plots/qcustomplot.h)
target_include_directories(LabAnalyserQCustomPlot PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(LabAnalyserQCustomPlot PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)
# qcustomplot is vendored implementation code, retained as an internal target
# rather than being treated as a project refactoring boundary.

set(LABANALYSER_CORE_SOURCES
    DropWidgets/CreateID.cpp
    DropWidgets/DropWidgetsUiLoader.cpp
    DropWidgets/DropWidgetBinding.cpp
    DropWidgets/DropWidgetConnectionMenu.cpp
    DropWidgets/DropWidgetConnectionMenu.h
    DropWidgets/DropWidgetDataAccess.h
    DropWidgets/DropWidgetDropBinding.h
    DropWidgets/DropWidgetDragSource.h
    DropWidgets/DropWidgetIndicatorBinding.h
    DropWidgets/DropWidgetTreePath.h
    DropWidgets/DropWidgetTableCells.cpp
    DropWidgets/DropWidgetTableCells.h
    DropWidgets/DropWidgetUpdate.h
    DropWidgets/DropWidgetBinding.h
    DropWidgets/Plots/FFTPlotWidget.cpp
    DropWidgets/Plots/PlotMeasurements.cpp
    DropWidgets/Plots/PlotWidget.cpp
    DropWidgets/QBLed.cpp
    DropWidgets/QCheckBox.cpp
    DropWidgets/QComboBox.cpp
    DropWidgets/QDoubleSpinBox.cpp
    DropWidgets/QLCDNumber.cpp
    DropWidgets/QLabel.cpp
    DropWidgets/QLed.cpp
    DropWidgets/QLineEdit.cpp
    DropWidgets/QListView.cpp
    DropWidgets/QProgressBar.cpp
    DropWidgets/QPushButton.cpp
    DropWidgets/QSlider.cpp
    DropWidgets/QSpinBox.cpp
    DropWidgets/QTSLed.cpp
    DropWidgets/QTableWidgeD.cpp
    Export/Export2Mat.cpp
    Export/export2highfive.cpp
    Export/exportinputs2xml.cpp
    Import/parameterloader.cpp
    LoadSave/loadplugin.cpp
    LoadSave/PluginLeasePool.cpp
    LoadSave/XmlFigureDimensions.cpp
    LoadSave/xmlexperimentreader.cpp
    LoadSave/xmlexperimentwriter.cpp
    DataManagement/ContainerStore.cpp
    DataManagement/DataRegistry.cpp
    DataManagement/DeviceRegistry.cpp
    DataManagement/WidgetBindingRegistry.cpp
    DataManagement/MessageDispatchPolicy.cpp
    DataManagement/ProjectIoCoordinator.cpp
    DataManagement/UIDataManagementSetClass.cpp
    DataManagement/DataManagementClass.cpp
    DataManagement/DataManagementSetClass.cpp
    DataManagement/DataMessengerClass.cpp
    RemoteControl/RemoteControlConnectionState.cpp
    RemoteControl/RemoteControlFrameSplitter.cpp
    RemoteControl/RemoteControlProtocol.cpp
    RemoteControl/RemoteControlServer.cpp
    UIFunctions/MainWindowOutputLog.cpp
    UIFunctions/MainWindowTreePath.cpp
    UIFunctions/MainWindowTreeViewState.cpp
    UIFunctions/MainWindowSubplotDialog.cpp
    UIFunctions/MainWindowFormLoader.cpp
    UIFunctions/SubPlotMainWindow.cpp
    TreeWidgetCustomDrop.cpp
    CustomWidgets/QBLedIndicator.cpp
    CustomWidgets/QLedIndicator.cpp
    CustomWidgets/QTSLedIndicator.cpp
    plugins/InterfaceDataType.cpp
    mainwindow.cpp
    mainwindow.ui
    About.ui
    resources.qrc)

# Subdirectories reuse this exact graph for the one established PlotWidget
# test-only compile seam, so keep paths anchored at the repository root.
list(TRANSFORM LABANALYSER_CORE_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
# This private helper consumes the ui header generated from the root form; it
# does not own a colocated .ui input of its own.
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/UIFunctions/MainWindowFormLoader.cpp"
    PROPERTIES SKIP_AUTOUIC ON)

add_library(LabAnalyserCore STATIC ${LABANALYSER_CORE_SOURCES})
target_include_directories(LabAnalyserCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${HIGHFIVE_INCLUDE_DIR}")
target_compile_definitions(LabAnalyserCore PUBLIC
    VERSION_MAJOR=1 VERSION_MINOR=1 VERSION_BUILD=1
    LABANALYSER_USE_FFTW
    GIT_VERSION=\"${LABANALYSER_GIT_VERSION}\")
target_compile_options(LabAnalyserCore PRIVATE -Werror=return-type)
target_link_libraries(LabAnalyserCore
    PUBLIC
        LabAnalyserQCustomPlot
        Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Xml Qt6::UiTools
        Qt6::PrintSupport Qt6::OpenGL Qt6::OpenGLWidgets
        PkgConfig::MATIO PkgConfig::HDF5 PkgConfig::FFTW3 Boost::boost)

add_executable(LabAnalyser main.cpp LabAnlyser.rc)
target_link_libraries(LabAnalyser PRIVATE LabAnalyserCore)
# main.cpp includes mainwindow.h, which in turn includes the UIC-generated
# About header produced for the core target.
target_include_directories(LabAnalyser PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/LabAnalyserCore_autogen/include")

include(CTest)
if(WIN32 AND LABANALYSER_BUILD_MATLAB_CONNECTOR)
    add_subdirectory(MatlabRemoteConnector)
endif()
if(BUILD_TESTING AND LABANALYSER_BUILD_TESTS)
    add_subdirectory(tests/cmake)
endif()
