#!/bin/bash

# Fermati in caso di errore
set -e

echo "========================================"
echo "    Tao Widget - Build & Packaging      "
echo "========================================"

# 1. Pulizia e preparazione
PROJECT_DIR=$(pwd)
BUILD_DIR="${PROJECT_DIR}/build_cpp"
NATIVE_DIR="${PROJECT_DIR}/tao-widget/contents/ui/native"
SHADER_SRC_DIR="${PROJECT_DIR}/tao-widget/src/shaders"
SHADER_OUT_DIR="${NATIVE_DIR}/shaders"
QSB="/usr/lib/qt6/bin/qsb"

echo "[1/5] Pulizia vecchi file..."
rm -f tao-widget.plasmoid
rm -f "$NATIVE_DIR/libtaoplugin.so"
# Non rimuoviamo build_cpp/ per velocizzare le ricompilazioni successive (make è incrementale)

# 2. Compilazione shader
echo "[2/5] Compilazione shader GLSL → QSB..."
mkdir -p "$SHADER_OUT_DIR"

"$QSB" --glsl "100 es,120,150" --hlsl 50 --msl 12 \
    -o "$SHADER_OUT_DIR/particle.vert.qsb" \
    "$SHADER_SRC_DIR/particle.vert"

"$QSB" --glsl "100 es,120,150" --hlsl 50 --msl 12 \
    -o "$SHADER_OUT_DIR/particle.frag.qsb" \
    "$SHADER_SRC_DIR/particle.frag"

echo "✓ Shader compilati."

# 3. Compilazione del Plugin C++ Nativo
echo "[3/5] Compilazione del plugin nativo C++..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Puntiamo a tao-widget dove si trova il CMakeLists.txt
cmake "$PROJECT_DIR/tao-widget"
make clean
make -j$(nproc)

echo ""
echo "✓ Compilazione completata con successo."
echo ""

# 4. Preparazione Plugin per distribuzione Portable
echo "[4/5] Integrazione plugin nel pacchetto..."
mkdir -p "$NATIVE_DIR"
cp "$BUILD_DIR/bin/libtaoplugin.so" "$NATIVE_DIR/"

# 5. Creazione del pacchetto .plasmoid
cd "$PROJECT_DIR"
echo "[5/5] Generazione file tao-widget.plasmoid..."

# Impacchettiamo (incluso il plugin appena compilato in contents/ui/native)
zip -r tao-widget.plasmoid tao-widget/ \
    -x "tao-widget/.git/*" \
    -x "tao-widget/src/*" \
    -x "tao-widget/CMakeLists.txt" \
    -x "tao-widget/reference/*" \
    -x "tao-widget/screenshots/*"

# 6. Verifica finale
if [ -f "tao-widget.plasmoid" ]; then
    echo "========================================"
    echo "✓ OPERAZIONE COMPLETATA"
    echo "Il pacchetto è pronto per la distribuzione."
    echo ""
    echo "Il plugin nativo si trova dentro:"
    echo "  tao-widget/contents/ui/native/libtaoplugin.so"
    echo ""
    echo "Gli shader compilati si trovano in:"
    echo "  tao-widget/contents/ui/native/shaders/"
    echo ""
    echo "Comando per installare/aggiornare:"
    echo "  kpackagetool6 -t Plasma/Applet --install tao-widget.plasmoid"
    echo "========================================"
else
    echo "✗ Errore: Impossibile creare il file .plasmoid"
    exit 1
fi