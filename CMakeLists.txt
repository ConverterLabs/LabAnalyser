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
    src/DropWidgets/Plots/qcustomplot.cpp
    src/DropWidgets/Plots/qcustomplot.h)
target_include_directories(LabAnalyserQCustomPlot PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(LabAnalyserQCustomPlot PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)
# qcustomplot is vendored implementation code, retained as an internal target
# rather than being treated as a project refactoring boundary.

set(LABANALYSER_CORE_SOURCES
    src/DropWidgets/CreateID.cpp
    src/DropWidgets/DropWidgetsUiLoader.cpp
    src/DropWidgets/DropWidgetBinding.cpp
    src/DropWidgets/DropWidgetConnectionMenu.cpp
    src/DropWidgets/DropWidgetConnectionMenu.h
    src/DropWidgets/DropWidgetDataAccess.h
    src/DropWidgets/DropWidgetDropBinding.h
    src/DropWidgets/DropWidgetDragSource.h
    src/DropWidgets/DropWidgetIndicatorBinding.h
    src/DropWidgets/DropWidgetTreePath.h
    src/DropWidgets/DropWidgetTableCells.cpp
    src/DropWidgets/DropWidgetTableCells.h
    src/DropWidgets/DropWidgetUpdate.h
    src/DropWidgets/DropWidgetBinding.h
    src/DropWidgets/Plots/FFTPlotWidget.cpp
    src/DropWidgets/Plots/PlotMeasurements.cpp
    src/DropWidgets/Plots/PlotWidget.cpp
    src/DropWidgets/QBLed.cpp
    src/DropWidgets/QCheckBox.cpp
    src/DropWidgets/QComboBox.cpp
    src/DropWidgets/QDoubleSpinBox.cpp
    src/DropWidgets/QLCDNumber.cpp
    src/DropWidgets/QLabel.cpp
    src/DropWidgets/QLed.cpp
    src/DropWidgets/QLineEdit.cpp
    src/DropWidgets/QListView.cpp
    src/DropWidgets/QProgressBar.cpp
    src/DropWidgets/QPushButton.cpp
    src/DropWidgets/QSlider.cpp
    src/DropWidgets/QSpinBox.cpp
    src/DropWidgets/QTSLed.cpp
    src/DropWidgets/QTableWidgeD.cpp
    src/Export/Export2Mat.cpp
    src/Export/LabDataArchive.cpp
    src/Export/export2highfive.cpp
    src/Export/exportinputs2xml.cpp
    src/Import/parameterloader.cpp
    src/Import/MatDataImport.cpp
    src/LoadSave/loadplugin.cpp
    src/LoadSave/PluginLeasePool.cpp
    src/LoadSave/XmlFigureDimensions.cpp
    src/LoadSave/xmlexperimentreader.cpp
    src/LoadSave/xmlexperimentwriter.cpp
    src/DataManagement/ContainerStore.cpp
    src/DataManagement/DataRegistry.cpp
    src/DataManagement/DeviceRegistry.cpp
    src/DataManagement/WidgetBindingRegistry.cpp
    src/DataManagement/MessageDispatchPolicy.cpp
    src/DataManagement/ProjectIoCoordinator.cpp
    src/DataManagement/UIDataManagementSetClass.cpp
    src/DataManagement/DataManagementClass.cpp
    src/DataManagement/DataManagementSetClass.cpp
    src/DataManagement/DataMessengerClass.cpp
    src/RemoteControl/RemoteControlConnectionState.cpp
    src/RemoteControl/RemoteControlFrameSplitter.cpp
    src/RemoteControl/RemoteControlProtocol.cpp
    src/RemoteControl/RemoteControlServer.cpp
    src/UIFunctions/MainWindowOutputLog.cpp
    src/UIFunctions/MainWindowTreePath.cpp
    src/UIFunctions/MainWindowTreeViewState.cpp
    src/UIFunctions/MainWindowSubplotDialog.cpp
    src/UIFunctions/MainWindowFormLoader.cpp
    src/UIFunctions/MainWindowTreeModel.cpp
    src/UIFunctions/MainWindowExplorerValues.cpp
    src/UIFunctions/MainWindowFigureFactory.cpp
    src/UIFunctions/MainWindowDockPresentation.cpp
    src/UIFunctions/MainWindowTrayController.cpp
    src/UIFunctions/MainWindowProjectCleanup.cpp
    src/UIFunctions/MainWindowContextMenus.cpp
    src/UIFunctions/MainWindowProjectActions.cpp
    src/UIFunctions/SubPlotMainWindow.cpp
    src/DropWidgets/TreeWidgetCustomDrop.cpp
    src/CustomWidgets/QBLedIndicator.cpp
    src/CustomWidgets/QLedIndicator.cpp
    src/CustomWidgets/QTSLedIndicator.cpp
    src/plugins/InterfaceDataType.cpp
    src/app/mainwindow.cpp
    src/app/mainwindow.ui
    src/app/About.ui
    resources/resources.qrc)

# Subdirectories reuse this exact graph for the one established PlotWidget
# test-only compile seam, so keep paths anchored at the repository root.
list(TRANSFORM LABANALYSER_CORE_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
# This private helper consumes the ui header generated from the root form; it
# does not own a colocated .ui input of its own.
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/UIFunctions/MainWindowFormLoader.cpp"
    PROPERTIES SKIP_AUTOUIC ON)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/UIFunctions/MainWindowDockPresentation.cpp"
    PROPERTIES SKIP_AUTOUIC ON)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/DataManagement/UIDataManagementSetClass.cpp"
    PROPERTIES SKIP_AUTOUIC ON)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/LoadSave/xmlexperimentreader.cpp"
    PROPERTIES SKIP_AUTOUIC ON)

add_library(LabAnalyserCore STATIC ${LABANALYSER_CORE_SOURCES})
set_property(TARGET LabAnalyserCore PROPERTY AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/app")
target_include_directories(LabAnalyserCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/src/app" "${CMAKE_CURRENT_SOURCE_DIR}/src/DropWidgets" "${HIGHFIVE_INCLUDE_DIR}")
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

add_executable(LabAnalyser src/app/main.cpp resources/LabAnlyser.rc)
target_link_libraries(LabAnalyser PRIVATE LabAnalyserCore)
# main.cpp includes mainwindow.h, which in turn includes the UIC-generated
# About header produced for the core target.
target_include_directories(LabAnalyser PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/LabAnalyserCore_autogen/include")

include(CTest)
if(WIN32 AND LABANALYSER_BUILD_MATLAB_CONNECTOR)
    add_subdirectory(src/MatlabRemoteConnector)
endif()
if(BUILD_TESTING AND LABANALYSER_BUILD_TESTS)
    add_subdirectory(tests/cmake)
endif()
