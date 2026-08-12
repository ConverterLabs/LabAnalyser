cmake_minimum_required(VERSION 3.21)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(MatlabRemoteConnector LANGUAGES CXX)
    include(CTest)
endif()

if(NOT WIN32)
    message(FATAL_ERROR "MatlabRemoteConnector currently targets the Windows MATLAB DLL ABI")
endif()

set(MATLAB_PACKAGE_FILES
    +LabAnalyser/Connect.m
    +LabAnalyser/Disconnect.m
    +LabAnalyser/ExReceive.m
    +LabAnalyser/ExSendDouble.m
    +LabAnalyser/ExSendString.m
    +LabAnalyser/Get.m
    +LabAnalyser/Set.m
    +LabAnalyser/TCPClient.h
    +LabAnalyser/TestRemote.m)

add_library(TCPClient SHARED
    TCPClient.cpp
    TCPClient.h
    TCPDummyClass.h)
target_compile_features(TCPClient PRIVATE cxx_std_11)
target_compile_definitions(TCPClient PRIVATE _WIN32_WINNT=0x0601)
target_link_libraries(TCPClient PRIVATE ws2_32)
if(MINGW)
    # Keep the MATLAB package self-contained. Windows system DLLs remain
    # dynamic; the MinGW runtime, including winpthread, is linked statically.
    target_link_options(TCPClient PRIVATE -static)
endif()
set_target_properties(TCPClient PROPERTIES
    PREFIX ""
    OUTPUT_NAME "TCPClient")

install(TARGETS TCPClient RUNTIME DESTINATION +LabAnalyser)
install(FILES ${MATLAB_PACKAGE_FILES} DESTINATION +LabAnalyser)

if(BUILD_TESTING)
    add_executable(MatlabRemoteConnectorContractTests
        tests/MatlabRemoteConnectorContractTests.cpp)
    target_compile_features(MatlabRemoteConnectorContractTests PRIVATE cxx_std_11)
    target_link_libraries(MatlabRemoteConnectorContractTests PRIVATE TCPClient ws2_32)
    add_test(NAME MatlabRemoteConnectorContractTests
        COMMAND MatlabRemoteConnectorContractTests)
    add_test(NAME MatlabPackageSourceContract
        COMMAND ${CMAKE_COMMAND}
            -DPACKAGE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/+LabAnalyser
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/MatlabPackageSourceContract.cmake)
endif()
