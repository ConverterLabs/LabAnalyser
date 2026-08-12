# CTest parity is deliberately explicit: every target below corresponds to one
# entry in tests/run-tests-msys2.ps1.  The qmake projects remain authoritative
# for behavioral contracts while CMake is introduced in parallel.

find_package(Qt6 REQUIRED COMPONENTS Test)

function(labanalyser_setup_test target)
    target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}")
    target_link_libraries(${target} PRIVATE Qt6::Test)
    add_test(NAME ${target} COMMAND ${target} -txt)
    set_tests_properties(${target} PROPERTIES
        ENVIRONMENT
            "LABANALYSER_TEST_REPOSITORY_ROOT=${PROJECT_SOURCE_DIR};LABANALYSER_TEST_PLUGIN_ROOT=${CMAKE_BINARY_DIR}/test-plugins")
endfunction()

function(labanalyser_setup_gui_test target)
    labanalyser_setup_test(${target})
    set_tests_properties(${target} PROPERTIES
        ENVIRONMENT
            "QT_QPA_PLATFORM=offscreen;LABANALYSER_TEST_REPOSITORY_ROOT=${PROJECT_SOURCE_DIR};LABANALYSER_TEST_PLUGIN_ROOT=${CMAKE_BINARY_DIR}/test-plugins")
endfunction()

function(labanalyser_add_full_graph_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE LabAnalyserCore)
    target_include_directories(${target} PRIVATE "${CMAKE_BINARY_DIR}/LabAnalyserCore_autogen/include")
    labanalyser_setup_gui_test(${target})
endfunction()

# Unit target.
add_executable(PlotMeasurementsTests
    ${PROJECT_SOURCE_DIR}/tests/PlotMeasurementsTests.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/Plots/PlotMeasurements.cpp)
target_compile_definitions(PlotMeasurementsTests PRIVATE LABANALYSER_USE_FFTW)
target_link_libraries(PlotMeasurementsTests PRIVATE Qt6::Core Qt6::Test PkgConfig::FFTW3)
labanalyser_setup_test(PlotMeasurementsTests)

# Component target with the same narrow production subset as its qmake file.
add_executable(DataManagementCharacterizationTests
    ${PROJECT_SOURCE_DIR}/tests/component/datamanagement/DataManagementCharacterizationTests.cpp
    ${PROJECT_SOURCE_DIR}/tests/component/datamanagement/mainwindow.h
    ${PROJECT_SOURCE_DIR}/tests/component/datamanagement/DropWidgets/DropWidgets.h
    ${PROJECT_SOURCE_DIR}/tests/component/datamanagement/DropWidgets/Plots/PlotWidget.h
    ${PROJECT_SOURCE_DIR}/DataManagement/ContainerStore.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DeviceRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/WidgetBindingRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/MessageDispatchPolicy.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataManagementClass.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataManagementSetClass.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataMessengerClass.cpp
    ${PROJECT_SOURCE_DIR}/plugins/InterfaceDataType.cpp)
target_include_directories(DataManagementCharacterizationTests BEFORE PRIVATE
    ${PROJECT_SOURCE_DIR}/tests/component/datamanagement)
target_link_libraries(DataManagementCharacterizationTests PRIVATE Qt6::Core Qt6::Widgets Qt6::Test Boost::boost)
labanalyser_setup_test(DataManagementCharacterizationTests)

# HDF5 export boundary stays slim; it must not pull GUI/plot sources.
add_executable(Hdf5ExportContractTests
    ${PROJECT_SOURCE_DIR}/tests/contract/hdf5/Hdf5ExportContractTests.cpp
    ${PROJECT_SOURCE_DIR}/Export/export2highfive.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/ContainerStore.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DeviceRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/WidgetBindingRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataManagementClass.cpp
    ${PROJECT_SOURCE_DIR}/plugins/InterfaceDataType.cpp)
target_link_libraries(Hdf5ExportContractTests PRIVATE Qt6::Widgets Qt6::Test PkgConfig::HDF5 Boost::boost)
labanalyser_setup_test(Hdf5ExportContractTests)

# Focused RemoteControl target retains the established DataManagement header
# seams; no full MainWindow or plot graph is linked.
add_executable(RemoteControlContractTests
    ${PROJECT_SOURCE_DIR}/tests/contract/remotecontrol/RemoteControlContractTests.cpp
    ${PROJECT_SOURCE_DIR}/tests/component/datamanagement/mainwindow.h
    ${PROJECT_SOURCE_DIR}/tests/component/datamanagement/DropWidgets/DropWidgets.h
    ${PROJECT_SOURCE_DIR}/tests/component/datamanagement/DropWidgets/Plots/PlotWidget.h
    ${PROJECT_SOURCE_DIR}/RemoteControl/RemoteControlConnectionState.cpp
    ${PROJECT_SOURCE_DIR}/RemoteControl/RemoteControlFrameSplitter.cpp
    ${PROJECT_SOURCE_DIR}/RemoteControl/RemoteControlProtocol.cpp
    ${PROJECT_SOURCE_DIR}/RemoteControl/RemoteControlServer.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/ContainerStore.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DeviceRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/WidgetBindingRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/MessageDispatchPolicy.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataManagementClass.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataManagementSetClass.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataMessengerClass.cpp
    ${PROJECT_SOURCE_DIR}/plugins/InterfaceDataType.cpp)
target_include_directories(RemoteControlContractTests BEFORE PRIVATE
    ${PROJECT_SOURCE_DIR}/tests/component/datamanagement)
target_link_libraries(RemoteControlContractTests PRIVATE Qt6::Core Qt6::Widgets Qt6::Network Qt6::Test Boost::boost)
labanalyser_setup_test(RemoteControlContractTests)

# The adapter suite preserves its two existing link seams and has no real
# MainWindow/qcustomplot graph.
add_executable(DropWidgetAdapterTests
    ${PROJECT_SOURCE_DIR}/tests/contract/dropwidgets/DropWidgetAdapterTests.cpp
    ${PROJECT_SOURCE_DIR}/tests/contract/dropwidgets/TestMainWindowSeam.cpp
    ${PROJECT_SOURCE_DIR}/tests/contract/dropwidgets/PlotWidgetLinkSeam.cpp
    ${PROJECT_SOURCE_DIR}/plugins/InterfaceDataType.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/ContainerStore.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DeviceRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/WidgetBindingRegistry.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/MessageDispatchPolicy.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataManagementClass.cpp
    ${PROJECT_SOURCE_DIR}/DataManagement/DataMessengerClass.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/CreateID.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/DropWidgetsUiLoader.cpp
    ${PROJECT_SOURCE_DIR}/TreeWidgetCustomDrop.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QBLed.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QCheckBox.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QComboBox.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QDoubleSpinBox.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QLCDNumber.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QLabel.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QLed.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QLineEdit.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QListView.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QProgressBar.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QPushButton.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QSlider.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QSpinBox.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QTableWidgeD.cpp
    ${PROJECT_SOURCE_DIR}/DropWidgets/QTSLed.cpp
    ${PROJECT_SOURCE_DIR}/CustomWidgets/QBLedIndicator.cpp
    ${PROJECT_SOURCE_DIR}/CustomWidgets/QLedIndicator.cpp
    ${PROJECT_SOURCE_DIR}/CustomWidgets/QTSLedIndicator.cpp
    ${PROJECT_SOURCE_DIR}/About.ui)
target_link_libraries(DropWidgetAdapterTests PRIVATE
    Qt6::Core Qt6::Widgets Qt6::UiTools Qt6::Network Qt6::PrintSupport Qt6::Test Boost::boost)
labanalyser_setup_gui_test(DropWidgetAdapterTests)

# Real application graph contracts.
labanalyser_add_full_graph_test(MatExportContractTests ${PROJECT_SOURCE_DIR}/tests/contract/mat/MatExportContractTests.cpp)
labanalyser_add_full_graph_test(ParameterContractTests ${PROJECT_SOURCE_DIR}/tests/contract/parameters/ParameterContractTests.cpp)
labanalyser_add_full_graph_test(XmlExperimentContractTests ${PROJECT_SOURCE_DIR}/tests/contract/xml/XmlExperimentContractTests.cpp)
labanalyser_add_full_graph_test(ProjectIoFacadeContractTests ${PROJECT_SOURCE_DIR}/tests/contract/projectio/ProjectIoFacadeContractTests.cpp)
labanalyser_add_full_graph_test(MainWindowIntegrationTests ${PROJECT_SOURCE_DIR}/tests/integration/mainwindow/MainWindowIntegrationTests.cpp)

# PlotWidget needs its existing test-only compile definition on the production
# sources, so it receives a separate private core variant rather than changing
# the application target's visibility.
add_library(LabAnalyserPlotTestCore STATIC ${LABANALYSER_CORE_SOURCES})
target_include_directories(LabAnalyserPlotTestCore PUBLIC "${PROJECT_SOURCE_DIR}" "${HIGHFIVE_INCLUDE_DIR}")
target_compile_definitions(LabAnalyserPlotTestCore PRIVATE
    LABANALYSER_PLOTWIDGET_TEST_SEAMS VERSION_MAJOR=1 VERSION_MINOR=1 VERSION_BUILD=1
    LABANALYSER_USE_FFTW GIT_VERSION=\"${LABANALYSER_GIT_VERSION}\")
target_link_libraries(LabAnalyserPlotTestCore PUBLIC LabAnalyserQCustomPlot
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Xml Qt6::UiTools Qt6::PrintSupport Qt6::OpenGL Qt6::OpenGLWidgets
    PkgConfig::MATIO PkgConfig::HDF5 PkgConfig::FFTW3 Boost::boost)
add_executable(PlotWidgetContractTests ${PROJECT_SOURCE_DIR}/tests/contract/plotwidget/PlotWidgetContractTests.cpp)
target_link_libraries(PlotWidgetContractTests PRIVATE LabAnalyserPlotTestCore)
target_include_directories(PlotWidgetContractTests PRIVATE "${CMAKE_BINARY_DIR}/LabAnalyserPlotTestCore_autogen/include")
labanalyser_setup_gui_test(PlotWidgetContractTests)
