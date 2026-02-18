#!/bin/bash

# Stop on any error
set -e

# ── Colors ────────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; NC='\033[0m'

ok()    { echo -e "${GREEN}✓${NC} $*"; }
info()  { echo -e "${CYAN}[${1}/${TOTAL_STEPS}]${NC} ${2}"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
die()   { echo -e "\n${RED}✗ ERROR:${NC} $*\n" >&2; show_help; exit 1; }

TOTAL_STEPS=5

# ── Parse arguments ───────────────────────────────────────────────────────────
SKIP_NATIVE=false
for arg in "$@"; do
    case "$arg" in
        --skip-native) SKIP_NATIVE=true ;;
        *) die "Unknown argument: ${arg}\nUsage: $0 [--skip-native]" ;;
    esac
done

# ── Dependency hints ──────────────────────────────────────────────────────────
show_help() {
    echo -e "${BOLD}Missing dependencies? Install them with:${NC}"
    echo
    echo -e "  ${CYAN}Arch Linux / Manjaro:${NC}"
    echo "    sudo pacman -S cmake ninja extra-cmake-modules qt6-base qt6-declarative"
    echo "    sudo pacman -S kf6-config kf6-coreaddons plasma-framework"
    echo "    sudo pacman -S qt6-shader-baker   # provides qsb"
    echo
    echo -e "  ${CYAN}Ubuntu 24.04 / KDE Neon:${NC}"
    echo "    sudo apt install cmake ninja-build extra-cmake-modules"
    echo "    sudo apt install qt6-base-dev qt6-declarative-dev qt6-tools-dev"
    echo "    sudo apt install libkf6config-dev libkf6coreaddons-dev"
    echo "    sudo apt install libplasma-dev"
    echo "    sudo apt install qt6-shader-baker  # provides qsb6"
    echo
    echo -e "  ${CYAN}Fedora:${NC}"
    echo "    sudo dnf install cmake ninja-build extra-cmake-modules"
    echo "    sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qttools-devel"
    echo "    sudo dnf install kf6-kconfig-devel kf6-kcoreaddons-devel plasma-devel"
    echo
}

echo -e "${BOLD}"
echo "========================================"
echo "    Tao Widget — Build & Packaging      "
echo "========================================"
echo -e "${NC}"

if [ "${SKIP_NATIVE}" = true ]; then
    warn "Skipping shader and native C++ plugin compilation (--skip-native)"
fi

# ── Paths ─────────────────────────────────────────────────────────────────────
PROJECT_DIR=$(pwd)
BUILD_DIR="${PROJECT_DIR}/build_cpp"
NATIVE_DIR="${PROJECT_DIR}/tao-widget/contents/ui/native"
SHADER_SRC_DIR="${PROJECT_DIR}/tao-widget/src/shaders"
SHADER_OUT_DIR="${NATIVE_DIR}/shaders"

# ── Step 1: Cleanup ───────────────────────────────────────────────────────────
info 1 "Cleaning old artifacts..."
rm -f tao-widget.plasmoid
if [ "${SKIP_NATIVE}" = false ]; then
    rm -f "${NATIVE_DIR}/libtaoplugin.so"
    rm -f "${SHADER_OUT_DIR}/"*.qsb
fi

# ── Step 2: Compile shaders ───────────────────────────────────────────────────
if [ "${SKIP_NATIVE}" = true ]; then
    info 2 "Skipping shader compilation..."
    for shader_type in vert frag; do
        qsb="${SHADER_OUT_DIR}/particle.${shader_type}.qsb"
        [ -f "${qsb}" ] \
            || die "Compiled shader not found: ${qsb}\n\
       With --skip-native the .qsb files must already be present.\n\
       Compile them locally first with: ./build.sh"
    done
    ok "Using existing .qsb shaders."
else
    # Locate qsb — name varies by distro
    if   command -v qsb   &>/dev/null; then QSB=$(command -v qsb)
    elif command -v qsb6  &>/dev/null; then QSB=$(command -v qsb6)
    elif [ -x "/usr/lib/qt6/bin/qsb" ]; then QSB="/usr/lib/qt6/bin/qsb"
    else
        die "qsb (Qt Shader Baker) not found.\n\
       Make sure qt6-shader-baker (Arch) or qt6-tools-dev (Ubuntu) is installed."
    fi

    info 2 "Compiling GLSL shaders → QSB..."
    mkdir -p "${SHADER_OUT_DIR}"

    for shader_type in vert frag; do
        src="${SHADER_SRC_DIR}/particle.${shader_type}"
        out="${SHADER_OUT_DIR}/particle.${shader_type}.qsb"

        [ -f "${src}" ] || die "Shader source not found: ${src}"

        "${QSB}" --glsl "100 es,120,150" --hlsl 50 --msl 12 \
            -o "${out}" "${src}" \
            || die "Shader compilation failed for: ${src}\n\
       Check that '${QSB}' supports the requested targets."
    done
    ok "Shaders compiled."
fi

# ── Step 3: Compile native C++ plugin ─────────────────────────────────────────
if [ "${SKIP_NATIVE}" = true ]; then
    info 3 "Skipping native C++ plugin compilation..."
    [ -f "${NATIVE_DIR}/libtaoplugin.so" ] \
        || die "libtaoplugin.so not found in ${NATIVE_DIR}/\n\
       With --skip-native the .so must already be present in the package.\n\
       Compile it locally first with: ./build.sh"
    ok "Using existing libtaoplugin.so."
else
    info 3 "Compiling native C++ plugin..."

    command -v cmake &>/dev/null \
        || die "cmake not found. See package hints above."

    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"

    cmake "${PROJECT_DIR}/tao-widget" \
        -DCMAKE_BUILD_TYPE=Release \
        || die "CMake configuration failed.\n\
       Check that all KDE/Qt6 development packages are installed.\n\
       Run this script again after installing missing dependencies."

    make clean
    make -j"$(nproc)" \
        || die "Compilation failed.\n\
       Check the error output above for missing headers or libraries."

    echo
    ok "Native plugin compiled successfully."
    echo

    # ── Step 4: Package plugin ────────────────────────────────────────────────
    info 4 "Integrating plugin into package..."
    mkdir -p "${NATIVE_DIR}"

    [ -f "${BUILD_DIR}/bin/libtaoplugin.so" ] \
        || die "libtaoplugin.so not found in ${BUILD_DIR}/bin/\n\
       The build may have succeeded but placed the .so in an unexpected location.\n\
       Check CMakeLists.txt LIBRARY_OUTPUT_DIRECTORY setting."

    cp "${BUILD_DIR}/bin/libtaoplugin.so" "${NATIVE_DIR}/"
    ok "Plugin copied to ${NATIVE_DIR}/"
fi

# ── Step 5: Create .plasmoid package ─────────────────────────────────────────
cd "${PROJECT_DIR}"
info 5 "Generating tao-widget.plasmoid..."

command -v zip &>/dev/null \
    || die "'zip' not found. Install it with: sudo pacman -S zip  /  sudo apt install zip"

zip -r tao-widget.plasmoid tao-widget/ \
    -x "tao-widget/.git/*"        \
    -x "tao-widget/src/*"         \
    -x "tao-widget/CMakeLists.txt"\
    -x "tao-widget/reference/*"   \
    -x "tao-widget/screenshots/*" \
    || die "Failed to create .plasmoid archive."

# ── Final report ──────────────────────────────────────────────────────────────
if [ -f "tao-widget.plasmoid" ]; then
    SIZE=$(du -sh tao-widget.plasmoid | cut -f1)
    echo
    echo -e "${GREEN}${BOLD}========================================"
    echo "✓ BUILD COMPLETE"
    echo -e "========================================${NC}"
    echo
    echo -e "  Package:       ${BOLD}tao-widget.plasmoid${NC} (${SIZE})"
    echo -e "  Native plugin: ${BOLD}${NATIVE_DIR}/libtaoplugin.so${NC}"
    echo -e "  Shaders:       ${BOLD}${SHADER_OUT_DIR}/${NC}"
    echo
    echo -e "${BOLD}Install / update:${NC}"
    echo "  kpackagetool6 -t Plasma/Applet --install tao-widget.plasmoid"
    echo
    echo -e "${BOLD}Force reinstall (if already installed):${NC}"
    echo "  kpackagetool6 -t Plasma/Applet --remove  org.kde.plasma.taowidget"
    echo "  kpackagetool6 -t Plasma/Applet --install tao-widget.plasmoid"
    echo
else
    die "Package file not created despite no errors — check disk space and permissions."
fi