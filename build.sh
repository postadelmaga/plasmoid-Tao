#!/bin/bash

# Creazione del file .plasmoid
echo "Creazione di tao-widget.plasmoid..."

zip -r tao-widget.plasmoid tao-widget/ \
  -x "tao-widget/.git/*"

# Verifica che il file sia stato creato
if [ -f "tao-widget.plasmoid" ]; then
    echo "✓ File tao-widget.plasmoid creato con successo!"
else
    echo "✗ Errore nella creazione del file .plasmoid"
    exit 1
fi