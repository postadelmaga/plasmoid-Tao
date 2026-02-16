import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtWebEngine
import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

PlasmoidItem {
    id: root

    property int particleCount: plasmoid.configuration.particleCount
    property int rotationSpeed: plasmoid.configuration.rotationSpeed
    property bool clockwise: plasmoid.configuration.clockwise
    property bool showClock: plasmoid.configuration.showClock
    property bool lowCpuMode: plasmoid.configuration.lowCpuMode
    property bool transparentBackground: plasmoid.configuration.transparentBackground
    property int renderEngine: plasmoid.configuration.renderEngine // 0: WebGL, 1: Native
    readonly property bool isActuallyVisible: (plasmoid.visible !== false) && (plasmoid.expanded !== false)

    // Settings for transparency and zero-padding
    Plasmoid.backgroundHints: root.transparentBackground ? PlasmaCore.Types.NoBackground : PlasmaCore.Types.DefaultBackground
    preferredRepresentation: fullRepresentation

    fullRepresentation: Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.preferredWidth: (Kirigami.Units.gridUnit || 18) * 20
        Layout.preferredHeight: Layout.preferredWidth

        Rectangle {
            id: mainContainer

            anchors.fill: parent
            color: "transparent"
            clip: true

            // 2. Native C++ Renderer (Zen Engine)
            Loader {
                id: nativeLoader

                anchors.fill: parent
                active: root.renderEngine === 1 && root.isActuallyVisible
                visible: active && status === Loader.Ready
                source: "NativeRenderer.qml"

                Binding {
                    target: nativeLoader.item
                    property: "particleCount"
                    value: root.particleCount
                    when: nativeLoader.status === Loader.Ready
                }

                Binding {
                    target: nativeLoader.item
                    property: "rotationSpeed"
                    value: root.rotationSpeed
                    when: nativeLoader.status === Loader.Ready
                }

                Binding {
                    target: nativeLoader.item
                    property: "clockwise"
                    value: root.clockwise
                    when: nativeLoader.status === Loader.Ready
                }

                Binding {
                    target: nativeLoader.item
                    property: "showClock"
                    value: root.showClock
                    when: nativeLoader.status === Loader.Ready
                }

                Binding {
                    target: nativeLoader.item
                    property: "lowCpuMode"
                    value: root.lowCpuMode
                    when: nativeLoader.status === Loader.Ready
                }

            }

            // 1. Web Renderer (Web Engine / WebGL)
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
                    var v = root.isActuallyVisible;
                    runJavaScript(`if(window.updateSettings)window.updateSettings(${p},${s},${d},${c},${l},${v});`);
                }

                visible: root.renderEngine === 0 || (root.renderEngine === 1 && nativeLoader.status === Loader.Error)
                enabled: visible && root.isActuallyVisible
                anchors.fill: parent
                backgroundColor: "transparent"
                url: root.isActuallyVisible ? Qt.resolvedUrl("tao.html") : "about:blank"
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

            // Mouse Area for both engines
            MouseArea {
                function updateMouse(x, y, inside) {
                    if (nativeLoader.item)
                        nativeLoader.item.mousePos = inside ? Qt.point(x, y) : Qt.point(-1000, -1000);

                    if (webView.visible)
                        webView.runJavaScript(`if(window.updateMouse)window.updateMouse(${x},${y},${inside});`);

                }

                anchors.fill: parent
                hoverEnabled: true
                enabled: root.isActuallyVisible
                onPositionChanged: (mouse) => {
                    return updateMouse(mouse.x, mouse.y, true);
                }
                onEntered: updateMouse(mouseX, mouseY, true)
                onExited: updateMouse(-1000, -1000, false)
            }

        }

    }

}
