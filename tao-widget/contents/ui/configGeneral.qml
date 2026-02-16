import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    id: configRoot

    property alias cfg_particleCount: particleCountSlider.value
    property alias cfg_rotationSpeed: rotationSpeedSlider.value
    property alias cfg_clockwise: clockwiseRadio.checked
    property alias cfg_showClock: showClockCheckBox.checked
    property alias cfg_lowCpuMode: lowCpuCheckBox.checked
    property alias cfg_useNativeRenderer: nativeRendererCheckBox.checked
    // Detection logic for the native plugin
    property bool nativeBackendAvailable: nativeChecker.status === Loader.Ready
    // Silence Plasma warnings for missing properties
    property string title: ""
    property int cfg_particleCountDefault: 80
    property int cfg_rotationSpeedDefault: 5
    property bool cfg_clockwiseDefault: true
    property bool cfg_showClockDefault: false
    property bool cfg_lowCpuModeDefault: false
    property bool cfg_useNativeRendererDefault: true

    Loader {
        id: nativeChecker

        source: "NativeRenderer.qml"
        active: true
        visible: false
    }

    // --- General Section ---
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18n("General")
    }

    QQC2.CheckBox {
        id: nativeRendererCheckBox

        text: i18n("Use Native C++ Backend (Faster)")
        Kirigami.FormData.label: i18n("Rendering:")
        enabled: configRoot.nativeBackendAvailable
    }

    QQC2.Label {
        text: configRoot.nativeBackendAvailable ? i18n("✓ Hybrid backend (Parallel/SceneGraph) available") : i18n("✗ Plugin not found (using Web fallback)")
        font.pixelSize: (Kirigami.Units.fontMetrics ? Kirigami.Units.fontMetrics.font.pixelSize : 12) * 0.8
        color: configRoot.nativeBackendAvailable ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
    }

    QQC2.CheckBox {
        id: showClockCheckBox

        Kirigami.FormData.label: i18n("Clock:")
        text: i18n("Show clock")
    }

    QQC2.CheckBox {
        id: lowCpuCheckBox

        Kirigami.FormData.label: i18n("Performance:")
        text: i18n("Low CPU Mode (disables shadows/glows)")
    }

    // --- Particle Section ---
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18n("Particle Effects")
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Amount:")
        spacing: Kirigami.Units.smallSpacing

        QQC2.Slider {
            id: particleCountSlider

            Layout.fillWidth: true
            from: 0
            to: 216
            stepSize: 1
            snapMode: QQC2.Slider.SnapAlways
        }

        QQC2.Label {
            text: Math.round(particleCountSlider.value)
            font.bold: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 2
            horizontalAlignment: Text.AlignRight
        }

    }

    // --- Movement Section ---
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18n("Movement Configuration")
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Speed:")
        spacing: Kirigami.Units.smallSpacing

        QQC2.Slider {
            id: rotationSpeedSlider

            Layout.fillWidth: true
            from: 0
            to: 10
            stepSize: 1
            snapMode: QQC2.Slider.SnapAlways
        }

        QQC2.Label {
            text: Math.round(rotationSpeedSlider.value)
            font.bold: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 2
            horizontalAlignment: Text.AlignRight
        }

    }

    Item {
        Kirigami.FormData.isSection: true
    }

    ColumnLayout {
        Kirigami.FormData.label: i18n("Rotation direction:")
        spacing: Kirigami.Units.smallSpacing

        QQC2.RadioButton {
            id: clockwiseRadio

            text: i18n("Clockwise")
            checked: true
        }

        QQC2.RadioButton {
            id: counterClockwiseRadio

            text: i18n("Counter-clockwise")
            checked: !clockwiseRadio.checked
        }

    }

    // Footer info
    QQC2.Label {
        Layout.fillWidth: true
        text: i18n("Changes will be applied instantly to the animation.")
        font.pixelSize: (Kirigami.Units.fontMetrics ? Kirigami.Units.fontMetrics.font.pixelSize : 12) * 0.9
        opacity: 0.6
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignLeft
    }

}
