import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols as KQuickControls

Kirigami.FormLayout {
    id: page

    property alias cfg_particleCount: particleCountSlider.value
    property alias cfg_rotationSpeed: rotationSpeedSlider.value
    property alias cfg_clockwise: clockwiseRadio.checked
    property alias cfg_particleColor1: particleColor1Button.color
    property alias cfg_particleColor2: particleColor2Button.color
    property int cfg_particleCountDefault: 80
    property int cfg_rotationSpeedDefault: 5
    property bool cfg_clockwiseDefault: true
    property color cfg_particleColor1Default: "#7fcdff"
    property color cfg_particleColor2Default: "#ffaa00"

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18n("Particle Physics")
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Amount:")

        QQC2.Slider {
            id: particleCountSlider

            Layout.fillWidth: true
            from: 0
            to: 3000
            stepSize: 10
        }

        QQC2.Label {
            text: Math.round(particleCountSlider.value)
        }

    }

    RowLayout {
        Kirigami.FormData.label: i18n("Speed:")

        QQC2.Slider {
            id: rotationSpeedSlider

            Layout.fillWidth: true
            from: 0
            to: 100
            stepSize: 1
        }

        QQC2.Label {
            text: Math.round(rotationSpeedSlider.value)
        }

    }

    QQC2.CheckBox {
        id: clockwiseRadio

        Kirigami.FormData.label: i18n("Direction:")
        text: i18n("Clockwise")
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18n("Particle Aesthetics")
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Main Color:")

        KQuickControls.ColorButton {
            id: particleColor1Button

            Layout.fillWidth: true
        }

    }

    RowLayout {
        Kirigami.FormData.label: i18n("Special/Fire Color:")

        KQuickControls.ColorButton {
            id: particleColor2Button

            Layout.fillWidth: true
        }

    }

}
