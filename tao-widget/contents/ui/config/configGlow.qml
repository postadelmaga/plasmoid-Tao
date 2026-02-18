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
                to: 2
                stepSize: 0.05
            }

            QQC2.Label {
                text: glowSize1Slider.value.toFixed(2)
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
                to: 2
                stepSize: 0.05
            }

            QQC2.Label {
                text: glowSize2Slider.value.toFixed(2)
            }

        }

    }

}
