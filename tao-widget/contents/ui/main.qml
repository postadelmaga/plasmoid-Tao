import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtWebEngine
import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

// Native Import - ensure you build the plugin!
PlasmoidItem {
    id: root

    property int particleCount: plasmoid.configuration.particleCount
    property int rotationSpeed: plasmoid.configuration.rotationSpeed
    property bool clockwise: plasmoid.configuration.clockwise
    property bool showClock: plasmoid.configuration.showClock
    property bool lowCpuMode: plasmoid.configuration.lowCpuMode
    property bool useNativeRenderer: plasmoid.configuration.useNativeRenderer
    property bool nativeBackendAvailable: nativeLoader.status === Loader.Ready
    // Silence Plasma warnings
    property bool cfg_clockwiseDefault: true
    property bool cfg_lowCpuModeDefault: false
    property int cfg_particleCountDefault: 80
    property int cfg_rotationSpeedDefault: 5
    property bool cfg_showClockDefault: false
    property bool cfg_useNativeRendererDefault: true

    width: Math.max(100, (Kirigami.Units.gridUnit || 18) * 20)
    height: width

    // Circular container
    Rectangle {
        id: mainContainer

        anchors.fill: parent
        radius: width / 2
        color: "transparent"
        clip: true
        layer.enabled: width > 0 && height > 0

        // 2. Native C++ Renderer (Loaded dynamically)
        Loader {
            id: nativeLoader

            anchors.fill: parent
            visible: root.useNativeRenderer && status === Loader.Ready
            source: "NativeRenderer.qml"
        }

        // 1. Web Renderer (Default, Fallback or WebGL Backend)
        WebEngineView {
            id: webView

            function updateHtml() {
                if (!visible)
                    return ;

                var p = root.particleCount;
                var s = root.rotationSpeed / 1000;
                var d = root.clockwise ? 1 : -1;
                var c = root.showClock;
                var l = root.lowCpuMode;
                runJavaScript("if(window.updateSettings)window.updateSettings(" + p + "," + s + "," + d + "," + c + "," + l + ");");
            }

            // Visible if NOT using native OR if native has failed
            visible: !root.useNativeRenderer || nativeLoader.status === Loader.Error
            enabled: visible
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

                function onLowCpuModeChanged() {
                    webView.updateHtml();
                }

                target: root
                enabled: webView.visible
            }

        }

        // Interattività Mouse
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onPositionChanged: {
                if (nativeLoader.item && nativeLoader.item.setMousePos)
                    nativeLoader.item.setMousePos(Qt.point(mouseX, mouseY));

            }
        }

    }

}
