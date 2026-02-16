cmake_minimum_required(VERSION 3.16)
project(tao-widget)

set(QT_MIN_VERSION "6.0.0")
set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Quick Gui Core)
find_package(KF6Config ${KF_MIN_VERSION} REQUIRED)
find_package(KF6CoreAddons ${KF_MIN_VERSION} REQUIRED)
find_package(Plasma ${KF_MIN_VERSION} REQUIRED)
find_package(PlasmaQuick ${KF_MIN_VERSION} REQUIRED)

# Component Plugin
add_library(taoplugin SHARED
    src/TaoPlugin.cpp
    src/TaoQuickItem.cpp
)

target_link_libraries(taoplugin
    Qt6::Quick
    Qt6::Gui
    Qt6::Core
    KF6::ConfigCore
    Plasma::Plasma
    Plasma::PlasmaQuick
)

# Install location for QML plugin
install(TARGETS taoplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/tao)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/tao)

# Install plasmoid structure
plasma_install_package(contents org.kde.tao.widget)
