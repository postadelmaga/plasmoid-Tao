import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols as KQuickControls

KCM.SimpleKCM {
    property alias cfg_glowColor1: glowColor1Button.color
    property alias cfg_glowSize1: glowSize1Slider.value
    property alias cfg_glowColor2: glowColor2Button.color
    property alias cfg_glowSize2: glowSize2Slider.value
    property color cfg_glowColor1Default: "#64c8ff"
    property real cfg_glowSize1Default: 1.3
    property color cfg_glowColor2Default: "#64c8ff"
    property real cfg_glowSize2Default: 0

    Kirigami.FormLayout {
        anchors.fill: parent

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Glow 1 (Main)")
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Color:")

            KQuickControls.ColorButton {
                id: glowColor1Button

                Layout.fillWidth: true
            }

        }

        RowLayout {
            Kirigami.FormData.label: i18n("Size:")

            QQC2.Slider {
                id: glowSize1Slider

                Layout.fillWidth: true
                from: 0
                to: 4
                stepSize: 0.1
            }

        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Glow 2 (Optional)")
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Color:")

            KQuickControls.ColorButton {
                id: glowColor2Button

                Layout.fillWidth: true
            }

        }

        RowLayout {
            Kirigami.FormData.label: i18n("Size:")

            QQC2.Slider {
                id: glowSize2Slider

                Layout.fillWidth: true
                from: 0
                to: 4
                stepSize: 0.1
            }

        }

    }

}
