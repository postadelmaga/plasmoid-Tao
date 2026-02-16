#!/bin/bash
# Esegui il build per assicurarti di avere l'ultima versione
# ./build.sh
echo "Avvio Test Performance..."
echo "Usa l'overlay per monitorare i frame 'STALL' o i tempi di swap."
echo ""
# Lancio con overlay di sincronizzazione e statistiche
QSG_VISUALIZE=changes QSG_RENDER_TIMING=1 plasmoidviewer -a tao-widget