import QtQuick
import QtQuick.Controls as QQC2
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

    width: Kirigami.Units.gridUnit * 20
    height: Kirigami.Units.gridUnit * 20
    preferredRepresentation: fullRepresentation
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    // Contenitore circolare trasparente senza bordo
    Rectangle {
        id: clipCircle

        anchors.fill: parent
        radius: width / 2
        color: "transparent"
        clip: true

        WebEngineView {
            id: webView

            function updateHtml() {
                const particles = root.particleCount;
                const speed = root.rotationSpeed / 1000;
                const direction = root.clockwise ? 1 : -1;
                runJavaScript("if (window.updateSettings) window.updateSettings(" + particles + ", " + speed + ", " + direction + ");");
            }

            url: Qt.resolvedUrl("tao.html")
            anchors.fill: parent
            backgroundColor: "transparent"
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

                target: root
            }

        }

    }

}
