import QtQuick
import QtWebEngine

WebEngineView {
    id: webView

    // RICEVIAMO L'OGGETTO SETTINGS DAL MAIN
    property QtObject viewSettings

    // Funzione chiamata dalla MouseArea del main
    function updateMouse(x, y, inside) {
        runJavaScript(`if(window.updateMouse)window.updateMouse(${x},${y},${inside});`);
    }

    function updateHtml() {
        // Controllo di sicurezza
        if (!viewSettings)
            return ;

        // Accediamo alle proprietà tramite l'oggetto settings
        runJavaScript(`if(window.updateSettings)window.updateSettings(${viewSettings.particleCount},${viewSettings.rotationSpeed},${viewSettings.clockwise},${viewSettings.showClock},${viewSettings.lowCpuMode});`);
    }

    anchors.fill: parent
    backgroundColor: "transparent"
    url: Qt.resolvedUrl("tao.html")
    onLoadingChanged: (loadingInfo) => {
        if (loadingInfo.status === WebEngineView.LoadSucceededStatus)
            updateHtml();

    }

    // Usiamo Connections per ascoltare i cambiamenti DENTRO l'oggetto settings
    Connections {
        // Sintassi: onNomeProprietaChanged
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

        function onLowCpuModeChanged() {
            updateHtml();
        }

        target: viewSettings
    }

}
