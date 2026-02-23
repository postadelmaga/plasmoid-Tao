import QtQuick
import QtWebEngine

WebEngineView {
    id: webView

    // RICEVIAMO L'OGGETTO SETTINGS DAL MAIN (setting name gives issue)
    property QtObject objsettings

    // Funzione chiamata dalla MouseArea del main
    function updateMouse(x, y, inside) {
        runJavaScript(`if(window.updateMouse)window.updateMouse(${x},${y},${inside});`);
    }

    function updateHtml() {
        // Controllo di sicurezza
        if (!objsettings)
            return ;

        // Accediamo alle proprietà tramite l'oggetto settings
        runJavaScript(`if(window.updateSettings)window.updateSettings({
            particleCount: ${objsettings.particleCount},
            rotationSpeed: ${objsettings.rotationSpeed},
            clockwise: ${objsettings.clockwise},
            showClock: ${objsettings.showClock},
            hourHandColor: "${objsettings.hourHandColor}",
            minuteHandColor: "${objsettings.minuteHandColor}",
            secondHandColor: "${objsettings.secondHandColor}",
            glowColor1: "${objsettings.glowColor1}",
            glowSize1: ${objsettings.glowSize1},
            glowColor2: "${objsettings.glowColor2}",
            glowSize2: ${objsettings.glowSize2},
            particleColor1: "${objsettings.particleColor1}",
            particleColor2: "${objsettings.particleColor2}"
        });`);
    }

    Timer {
        id: updateTimer

        interval: 32 // ~30fps max for bridge updates (don't flood the process)
        repeat: false
        onTriggered: updateHtml()
    }

    function requestUpdate() {
        if (!updateTimer.running)
            updateTimer.start();

    }

    anchors.fill: parent
    backgroundColor: "transparent"
    url: Qt.resolvedUrl("webgl.html")
    onLoadingChanged: (loadingInfo) => {
        if (loadingInfo.status === WebEngineView.LoadSucceededStatus)
            updateHtml();

    }

    // Usiamo Connections per ascoltare i cambiamenti DENTRO l'oggetto settings
    Connections {
        function onParticleCountChanged() {
            requestUpdate();
        }

        function onRotationSpeedChanged() {
            requestUpdate();
        }

        function onClockwiseChanged() {
            requestUpdate();
        }

        function onShowClockChanged() {
            requestUpdate();
        }

        function onHourHandColorChanged() {
            requestUpdate();
        }

        function onMinuteHandColorChanged() {
            requestUpdate();
        }

        function onSecondHandColorChanged() {
            requestUpdate();
        }

        function onGlowColor1Changed() {
            requestUpdate();
        }

        function onGlowSize1Changed() {
            requestUpdate();
        }

        function onGlowColor2Changed() {
            requestUpdate();
        }

        function onGlowSize2Changed() {
            requestUpdate();
        }

        function onParticleColor1Changed() {
            requestUpdate();
        }

        function onParticleColor2Changed() {
            requestUpdate();
        }

        target: objsettings
        ignoreUnknownSignals: true
    }

}
