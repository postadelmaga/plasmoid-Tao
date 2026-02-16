# Tech Stack - Mystical Tao

## Core Technologies
- **C++ (Standard 17):** Used for the high-performance particle system plugin (`taoplugin`).
- **QML (Qt 6.0+):** Used for the widget UI, animations, and top-level logic.
- **JavaScript:** Used for complex logic within QML files.

## Frameworks and Libraries
- **KDE Frameworks 6 (KF6):** Specifically KF6Config and KF6CoreAddons for integration with the Plasma desktop.
- **Plasma 6:** Provides the widget API (`PlasmaQuick`) and desktop shell integration.
- **Qt 6 (Core, Gui, Quick, Concurrent):** The foundational UI and application framework.

## Build and Deployment
- **CMake:** Used to build the C++ shared library.
- **Extra CMake Modules (ECM):** Required for KDE/Plasma-specific CMake macros and settings.
- **Custom build.sh:** A script that automates building the C++ plugin and packaging it into the `.plasmoid` file structure.
