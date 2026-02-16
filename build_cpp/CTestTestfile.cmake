# CMake generated Testfile for 
# Source directory: /home/fra/Dev/plasmoid-tao/tao-widget
# Build directory: /home/fra/Dev/plasmoid-tao/build_cpp
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(appstreamtest "/usr/bin/cmake" "-DAPPSTREAMCLI=/usr/bin/appstreamcli" "-DINSTALL_FILES=/home/fra/Dev/plasmoid-tao/build_cpp/install_manifest.txt" "-P" "/usr/share/ECM/kde-modules/appstreamtest.cmake")
set_tests_properties(appstreamtest PROPERTIES  _BACKTRACE_TRIPLES "/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;173;add_test;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;191;appstreamtest;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;0;;/home/fra/Dev/plasmoid-tao/tao-widget/CMakeLists.txt;14;include;/home/fra/Dev/plasmoid-tao/tao-widget/CMakeLists.txt;0;")
