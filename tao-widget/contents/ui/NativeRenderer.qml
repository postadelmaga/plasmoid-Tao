import QtQuick
import "native" as TaoNative

Item {
    id: renderer

    // 1. Riceviamo l'oggetto unificato 'settings' dal main.qml
    property QtObject settings: null
    // 2. Manteniamo la proprietà interna per il mouse (che serve al componente ibrido)
    property point mousePos: Qt.point(-1000, -1000)

    // 3. Funzione pubblica per aggiornare il mouse (chiamata dal main.qml)
    function updateMouse(x, y, inside) {
        renderer.mousePos = inside ? Qt.point(x, y) : Qt.point(-1000, -1000);
    }

    anchors.fill: parent

    // Backend Nativo/Ibrido
    TaoNative.TaoAnimationHybrid {
        // 4. Colleghiamo le proprietà del componente ai valori dentro 'settings'.
        // Il controllo (renderer.settings ? ... : default) serve per evitare errori
        // nel millisecondo in cui il Loader crea l'oggetto ma il binding non è ancora arrivato.

        anchors.fill: parent
        particleCount: renderer.settings ? renderer.settings.particleCount : 0
        rotationSpeed: renderer.settings ? renderer.settings.rotationSpeed : 0
        clockwise: renderer.settings ? renderer.settings.clockwise : false
        showClock: renderer.settings ? renderer.settings.showClock : false
        lowCpuMode: renderer.settings ? renderer.settings.lowCpuMode : false
        // Il mouse usa la proprietà locale aggiornata dalla funzione sopra
        mousePos: renderer.mousePos
    }

}
