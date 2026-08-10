# Test-only qmake wrapper: it preserves the production project exactly while
# disabling its return-type warning promotion for the non-blocking analysis
# baseline. It lives beside LabAnalyser.pro so included relative source paths
# retain their production meaning.
include($$PWD/LabAnalyser.pro)
QMAKE_CXXFLAGS -= -Werror=return-type
QMAKE_CXXFLAGS_RELEASE -= -Werror=return-type
