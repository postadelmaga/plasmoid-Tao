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
            updateHtml();
        }

        function onRotationSpeedChanged() {
            updateHtml();
        }

        function onClockwiseChanged() {
            updateHtml();
        }

        function onShowClockChanged() {
            updateHtml();
        }

        function onHourHandColorChanged() {
            updateHtml();
        }

        function onMinuteHandColorChanged() {
            updateHtml();
        }

        function onSecondHandColorChanged() {
            updateHtml();
        }

        function onGlowColor1Changed() {
            updateHtml();
        }

        function onGlowSize1Changed() {
            updateHtml();
        }

        function onGlowColor2Changed() {
            updateHtml();
        }

        function onGlowSize2Changed() {
            updateHtml();
        }

        function onParticleColor1Changed() {
            updateHtml();
        }

        function onParticleColor2Changed() {
            updateHtml();
        }

        target: objsettings
        ignoreUnknownSignals: true
    }

}
