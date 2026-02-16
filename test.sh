#!/bin/bash
# Esegui il build per assicurarti di avere l'ultima versione
# ./build.sh
echo "Avvio Test Performance..."
echo "Usa l'overlay per monitorare i frame 'STALL' o i tempi di swap."
echo ""
# Lancio con overlay di sincronizzazione e statistiche

QT_LOGGING_RULES="qml.debug=true" plasmoidviewer -a tao-widget