import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

PlasmoidItem {
    id: root

    // --- Proprietà del Plasmoide ---
    property int particleCount: plasmoid.configuration.particleCount
    property int rotationSpeed: plasmoid.configuration.rotationSpeed
    property bool clockwise: plasmoid.configuration.clockwise
    property bool showClock: plasmoid.configuration.showClock
    property bool lowCpuMode: plasmoid.configuration.lowCpuMode
    property bool transparentBackground: plasmoid.configuration.transparentBackground
    property int renderEngine: plasmoid.configuration.renderEngine // 0: WebGL, 1: Native
    // Clock Colors
    property color hourHandColor: plasmoid.configuration.hourHandColor
    property color minuteHandColor: plasmoid.configuration.minuteHandColor
    property color secondHandColor: plasmoid.configuration.secondHandColor
    // Appearance (Glow)
    property color glowColor1: plasmoid.configuration.glowColor1
    property double glowSize1: plasmoid.configuration.glowSize1
    property color glowColor2: plasmoid.configuration.glowColor2
    property double glowSize2: plasmoid.configuration.glowSize2
    // Particles
    property color particleColor1: plasmoid.configuration.particleColor1
    property color particleColor2: plasmoid.configuration.particleColor2

    Plasmoid.backgroundHints: root.transparentBackground ? PlasmaCore.Types.NoBackground : PlasmaCore.Types.DefaultBackground
    preferredRepresentation: fullRepresentation
    width: Kirigami.Units.gridUnit * 20
    height: Kirigami.Units.gridUnit * 15

    fullRepresentation: Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.preferredWidth: (Kirigami.Units.gridUnit || 18) * 20
        Layout.preferredHeight: Layout.preferredWidth

        // --- 1. SETTINGS OBJECT (Il "Pacchetto" di dati) ---
        // Raggruppiamo tutte le configurazioni qui.
        // I file esterni leggeranno solo "settings.nomeProprietà"
        QtObject {
            id: objsettings

            // Usiamo readonly per sicurezza, tanto cambiano solo se cambia root
            readonly property int particleCount: root.particleCount
            readonly property int rotationSpeed: root.rotationSpeed
            readonly property bool clockwise: root.clockwise
            readonly property bool showClock: root.showClock
            readonly property bool lowCpuMode: root.lowCpuMode
            // Clock
            readonly property color hourHandColor: root.hourHandColor
            readonly property color minuteHandColor: root.minuteHandColor
            readonly property color secondHandColor: root.secondHandColor
            // Glow
            readonly property color glowColor1: root.glowColor1
            readonly property double glowSize1: root.glowSize1
            readonly property color glowColor2: root.glowColor2
            readonly property double glowSize2: root.glowSize2
            // Particles
            readonly property color particleColor1: root.particleColor1
            readonly property color particleColor2: root.particleColor2
        }

        Rectangle {
            id: mainContainer

            anchors.fill: parent
            color: "transparent"
            clip: true

            // --- 2. NATIVE LOADER ---
            Loader {
                id: nativeLoader

                anchors.fill: parent
                // Attivo SOLO se engine == 1
                active: root.renderEngine === 1
                visible: active && status === Loader.Ready
                source: "NativeRenderer.qml"

                // Un solo binding per passare TUTTO!
                Binding {
                    target: nativeLoader.item
                    property: "objsettings"
                    value: objsettings
                    when: nativeLoader.status === Loader.Ready
                }

            }

            // --- 3. WEB LOADER ---
            Loader {
                id: webLoader

                anchors.fill: parent
                // Attivo se engine == 0 OPPURE se il nativo fallisce
                active: (root.renderEngine === 0 || (root.renderEngine === 1 && nativeLoader.status === Loader.Error))
                visible: active
                source: "WebRenderer.qml"

                // Un solo binding anche qui
                Binding {
                    target: webLoader.item
                    property: "objsettings"
                    value: objsettings
                    when: webLoader.status === Loader.Ready
                }

            }

            // --- 4. MOUSE AREA UNIFICATA ---
            MouseArea {
                // Funzione helper per smistare l'evento al loader attivo
                function dispatchMouse(x, y, inside) {
                    var item = nativeLoader.item || webLoader.item;
                    if (item)
                        item.updateMouse(x, y, inside);

                }

                anchors.fill: parent
                hoverEnabled: true
                enabled: true
                onPositionChanged: (mouse) => {
                    return dispatchMouse(mouse.x, mouse.y, true);
                }
                onEntered: dispatchMouse(mouseX, mouseY, true)
                onExited: dispatchMouse(-1000, -1000, false)
            }

        }

    }

}
