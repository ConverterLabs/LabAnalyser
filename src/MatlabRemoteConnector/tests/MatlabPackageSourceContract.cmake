set(expected_files
    Connect.m
    Disconnect.m
    ExReceive.m
    ExSendDouble.m
    ExSendString.m
    Get.m
    Set.m
    TCPClient.h
    TestRemote.m)

foreach(file IN LISTS expected_files)
    if(NOT EXISTS "${PACKAGE_DIR}/${file}")
        message(FATAL_ERROR "Required MATLAB package source is missing: ${file}")
    endif()
endforeach()

file(READ "${PACKAGE_DIR}/Connect.m" connect_source)
foreach(required_text
        "mfilename('fullpath')"
        "fullfile(packageDirectory, 'TCPClient.dll')"
        "fullfile(packageDirectory, 'TCPClient.h')"
        "'alias', 'TCPClient'")
    string(FIND "${connect_source}" "${required_text}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Connect.m does not contain required package-relative lookup: ${required_text}")
    endif()
endforeach()

file(READ "${PACKAGE_DIR}/TCPClient.h" public_header)
foreach(exported_function
        Connect
        Disconnect
        IsConnected
        ReadReceivedStringData
        ReceiveDoubleData
        ReadReceivedDoubleData
        SendDoubleData
        SendStringData)
    string(FIND "${public_header}" "${exported_function}(" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "MATLAB loadlibrary header is missing: ${exported_function}")
    endif()
endforeach()

file(READ "${PACKAGE_DIR}/Get.m" get_source)
foreach(required_text
        "results = struct('ID', {}, 'Time', {}, 'Data', {});"
        "strsplit(char(encodedIds), '|')"
        "results(end + 1) = struct('ID', currentId, 'Time', time, 'Data', value);"
        "'IsConnected'")
    string(FIND "${get_source}" "${required_text}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Get.m does not contain required wildcard behavior: ${required_text}")
    endif()
endforeach()
