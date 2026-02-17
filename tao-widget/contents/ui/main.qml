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
                // Carica solo se engine è 1
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

            // 1. Web Renderer (Web Engine / WebGL) - ORA DENTRO UN LOADER
            Loader {
                id: webLoader
                anchors.fill: parent

                // Active controlla la CREAZIONE dell'oggetto.
                // Si crea solo se engine è 0 OPPURE se l'engine nativo è andato in errore.
                active: (root.renderEngine === 0 || (root.renderEngine === 1 && nativeLoader.status === Loader.Error)) && root.isActuallyVisible
                visible: active
                source: "WebRenderer.qml"

                Binding {
                    target: webLoader.item
                    property: "particleCount"
                    value: root.particleCount
                    when: webLoader.status === Loader.Ready
                }
                Binding {
                    target: webLoader.item
                    property: "rotationSpeed"
                    value: root.rotationSpeed
                    when: webLoader.status === Loader.Ready
                }
                Binding {
                    target: webLoader.item
                    property: "clockwise"
                    value: root.clockwise
                    when: webLoader.status === Loader.Ready
                }
                Binding {
                    target: webLoader.item
                    property: "showClock"
                    value: root.showClock
                    when: webLoader.status === Loader.Ready
                }
                Binding {
                    target: webLoader.item
                    property: "lowCpuMode"
                    value: root.lowCpuMode
                    when: webLoader.status === Loader.Ready
                }
                Binding {
                    target: webLoader.item
                    property: "isActuallyVisible"
                    value: root.isActuallyVisible
                    when: webLoader.status === Loader.Ready
                }

            }

            // Mouse Area for both engines
            MouseArea {
                function updateMouse(x, y, inside) {
                    // Gestione Native
                    if (nativeLoader.item)
                        nativeLoader.item.mousePos = inside ? Qt.point(x, y) : Qt.point(-1000, -1000);

                    // Gestione Web: controlliamo se webLoader ha caricato l'item
                    if (webLoader.item) {
                        // Nota: non possiamo chiamare webView direttamente per ID dall'esterno del Component
                        // Usiamo webLoader.item per accedere all'oggetto caricato
                        webLoader.item.runJavaScript(`if(window.updateMouse)window.updateMouse(${x},${y},${inside});`);
                    }
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
