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
    add_compile_options(-O3 -march=native)
endif()

# Component Plugin
add_library(taoplugin SHARED
    src/TaoPlugin.cpp
    src/TaoQuickItem.cpp
    src/TaoQGraphGemini.cpp
    src/TaoQGraphClaude.cpp
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
)

# Output directory for the shared library
set_target_properties(taoplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install location for QML plugin
# Correct qmldir path is in contents/ui/native
set(INSTALL_PATH ${KDE_INSTALL_QMLDIR}/org/kde/private/tao)
install(TARGETS taoplugin DESTINATION ${INSTALL_PATH})
install(FILES contents/ui/native/qmldir DESTINATION ${INSTALL_PATH})

# Install plasmoid structure
plasma_install_package(contents org.kde.tao.widget)
