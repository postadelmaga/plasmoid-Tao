cmake_minimum_required(VERSION 3.16)
project(tao-widget)

set(QT_MIN_VERSION "6.0.0")
set(KF_MIN_VERSION "6.0.0")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Quick Gui Core Concurrent)
find_package(KF6Config ${KF_MIN_VERSION} REQUIRED)
find_package(KF6CoreAddons ${KF_MIN_VERSION} REQUIRED)
find_package(Plasma ${KF_MIN_VERSION} REQUIRED)
find_package(PlasmaQuick ${KF_MIN_VERSION} REQUIRED)

# Optimization and Performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -ffast-math)
endif()

# Component Plugin
add_library(taoplugin SHARED
    src/TaoPlugin.cpp
    src/TaoQGraphHybrid.cpp
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Enable LTO if supported
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT error)
    if(lto_supported)
        set_target_properties(taoplugin PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

target_link_libraries(taoplugin
    Qt6::Quick
    Qt6::Gui
    Qt6::Core
    Qt6::Concurrent
    KF6::ConfigCore
    Plasma::Plasma
    Plasma::PlasmaQuick
    ${CMAKE_DL_LIBS}
)

# Output directory for the shared library
set_target_properties(taoplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# NOTE: System installation rules (install, plasma_install_package) 
# have been removed because we are now using a "portable" approach 
# where the build.sh script packages the .so directly into the 
# .plasmoid file in contents/ui/native/.