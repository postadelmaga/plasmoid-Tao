#!/bin/bash

# Fermati in caso di errore
set -e

echo "========================================"
echo "    Tao Widget - Build & Packaging      "
echo "========================================"

# 1. Pulizia e preparazione
PROJECT_DIR=$(pwd)
BUILD_DIR="${PROJECT_DIR}/tao-widget/build"
NATIVE_DIR="${PROJECT_DIR}/tao-widget/contents/ui/native"

echo "[1/4] Pulizia vecchi file..."
rm -f tao-widget.plasmoid
rm -f "$NATIVE_DIR/libtaoplugin.so"
# Non rimuoviamo build/ per velocizzare le ricompilazioni successive (make è incrementale)

# 2. Compilazione del Plugin C++ Nativo
echo "[2/4] Compilazione del plugin nativo C++..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake ..
make -j$(nproc)

echo ""
echo "✓ Compilazione completata con successo."
echo ""

# 3. Preparazione Plugin per distribuzione Portable
echo "[3/4] Integrazione plugin nel pacchetto..."
mkdir -p "$NATIVE_DIR"
cp "$BUILD_DIR/bin/libtaoplugin.so" "$NATIVE_DIR/"

# 4. Creazione del pacchetto .plasmoid
cd "$PROJECT_DIR"
echo "[4/4] Generazione file tao-widget.plasmoid..."

# Impacchettiamo (incluso il plugin appena compilato in contents/ui/native)
zip -r tao-widget.plasmoid tao-widget/ \
    -x "tao-widget/.git/*" \
    -x "tao-widget/build/*" \
    -x "tao-widget/src/*" \
    -x "tao-widget/CMakeLists.txt" \
    -x "tao-widget/reference/*" \
    -x "tao-widget/screenshots/*"

# 5. Verifica finale
if [ -f "tao-widget.plasmoid" ]; then
    echo "========================================"
    echo "✓ OPERAZIONE COMPLETATA"
    echo "Il pacchetto è pronto per la distribuzione."
    echo ""
    echo "Il plugin nativo si trova dentro:"
    echo "  tao-widget/contents/ui/native/libtaoplugin.so"
    echo ""
    echo "Comando per installare/aggiornare:"
    echo "  kpackagetool6 -t Plasma/Applet --install tao-widget.plasmoid"
    echo "========================================"
else
    echo "✗ Errore: Impossibile creare il file .plasmoid"
    exit 1
fi