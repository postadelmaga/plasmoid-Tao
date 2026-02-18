import QtQuick
import "native" as TaoNative

Item {
    id: renderer

    // 1. Riceviamo l'oggetto unificato 'settings' dal main.qml
    property QtObject objsettings: null
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
        particleCount: renderer.objsettings ? renderer.objsettings.particleCount : 0
        rotationSpeed: renderer.objsettings ? renderer.objsettings.rotationSpeed : 0
        clockwise: renderer.objsettings ? renderer.objsettings.clockwise : false
        showClock: renderer.objsettings ? renderer.objsettings.showClock : false
        // Clock Colors
        hourHandColor: renderer.objsettings ? renderer.objsettings.hourHandColor : "white"
        minuteHandColor: renderer.objsettings ? renderer.objsettings.minuteHandColor : "blue"
        secondHandColor: renderer.objsettings ? renderer.objsettings.secondHandColor : "red"
        // Glow
        glowColor1: renderer.objsettings ? renderer.objsettings.glowColor1 : "white"
        glowSize1: renderer.objsettings ? renderer.objsettings.glowSize1 : 0
        glowColor2: renderer.objsettings ? renderer.objsettings.glowColor2 : "white"
        glowSize2: renderer.objsettings ? renderer.objsettings.glowSize2 : 0
        // Particle Colors
        particleColor1: renderer.objsettings ? renderer.objsettings.particleColor1 : "white"
        particleColor2: renderer.objsettings ? renderer.objsettings.particleColor2 : "white"
        // Il mouse usa la proprietà locale aggiornata dalla funzione sopra
        mousePos: renderer.mousePos
    }

}
