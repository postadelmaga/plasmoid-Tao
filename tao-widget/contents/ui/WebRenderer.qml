import QtQuick
import QtWebEngine

WebEngineView {
    id: webView

    // Definiamo le proprietà che riceveremo dal main.qml
    property int particleCount
    property int rotationSpeed
    property bool clockwise
    property bool showClock
    property bool lowCpuMode
    property bool isActuallyVisible

    anchors.fill: parent
    backgroundColor: "transparent"
    url: Qt.resolvedUrl("tao.html")

    // Funzione interna per aggiornare l'HTML
    function updateHtml() {
        // Usiamo le proprietà locali (definite sopra) invece di root.xxx
        var p = particleCount;
        var s = rotationSpeed / 1000;
        var d = clockwise ? 1 : -1;
        var c = showClock;
        var l = lowCpuMode;
        var v = isActuallyVisible;

        runJavaScript(`if(window.updateSettings)window.updateSettings(${p},${s},${d},${c},${l},${v});`);
    }

    // Appena la pagina è caricata, aggiorniamo
    onLoadingChanged: (loadingInfo) => {
        if (loadingInfo.status === WebEngineView.LoadSucceededStatus)
            updateHtml();
    }

    // Quando le proprietà cambiano (ricevendo nuovi valori dal main), aggiorniamo
    onParticleCountChanged: updateHtml()
    onRotationSpeedChanged: updateHtml()
    onClockwiseChanged: updateHtml()
    onShowClockChanged: updateHtml()
    onLowCpuModeChanged: updateHtml()
    onIsActuallyVisibleChanged: updateHtml()
}
