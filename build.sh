#!/bin/bash

# Creazione del file .plasmoid
echo "Creazione di tao-widget.plasmoid..."

zip -r tao-widget.plasmoid tao-widget/ \
  -x "tao-widget/.git/*"

# Verifica che il file sia stato creato
# Verifica che il file sia stato creato
if [ -f "tao-widget.plasmoid" ]; then
    echo "✓ File tao-widget.plasmoid creato con successo!"
    echo ""
    echo "NOTA: Hai attivato il supporto C++ Native."
    echo "Per usare il backend nativo, devi compilare e installare il plugin:"
    echo "  cd tao-widget"
    echo "  mkdir build && cd build"
    echo "  cmake .. -DCMAKE_INSTALL_PREFIX=/usr  # o ~/.local"
    echo "  make"
    echo "  sudo make install"
    echo ""
else
    echo "✗ Errore nella creazione del file .plasmoid"
    exit 1
fi