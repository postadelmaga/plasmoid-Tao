import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Effects // Importante per il blur nativo in Qt 6
import QtQuick.Layouts
import QtWebEngine
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

PlasmoidItem {
    id: root

    // Proprietà configurabili
    property int particleCount: plasmoid.configuration.particleCount
    property int rotationSpeed: plasmoid.configuration.rotationSpeed
    property bool clockwise: plasmoid.configuration.clockwise
    property bool showClock: plasmoid.configuration.showClock

    width: Kirigami.Units.gridUnit * 20
    height: Kirigami.Units.gridUnit * 20
    // Per Plasma 6, usiamo l'enumerazione corretta di PlasmaCore.
    // NoBackground permette a noi di gestire interamente lo sfondo (trasparenza e blur) tramite QML.
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    // Contenitore circolare principale
    Rectangle {
        id: mainContainer

        anchors.fill: parent
        radius: width / 2
        color: Qt.rgba(0, 0, 0, 0) // Fondo scuro semitrasparente
        clip: true
        // layer.enabled è necessario per forzare il clipping tondo sul WebEngineView
        layer.enabled: true

        WebEngineView {
            id: webView

            function updateHtml() {
                const particles = root.particleCount;
                const speed = root.rotationSpeed / 1000;
                const direction = root.clockwise ? 1 : -1;
                const clock = root.showClock;
                runJavaScript("if (window.updateSettings) window.updateSettings(" + particles + ", " + speed + ", " + direction + ", " + clock + ");");
            }

            anchors.fill: parent
            backgroundColor: "transparent"
            url: Qt.resolvedUrl("tao.html")
            onLoadingChanged: (loadingInfo) => {
                if (loadingInfo.status === WebEngineView.LoadSucceededStatus)
                    updateHtml();

            }

            Connections {
                function onParticleCountChanged() {
                    webView.updateHtml();
                }

                function onRotationSpeedChanged() {
                    webView.updateHtml();
                }

                function onClockwiseChanged() {
                    webView.updateHtml();
                }

                function onShowClockChanged() {
                    webView.updateHtml();
                }

                target: root
            }

        }

    }

}
