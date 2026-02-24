import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols as KQuickControls

KCM.SimpleKCM {
    property alias cfg_particleCount: particleCountSlider.value
    property alias cfg_particleColor1: particleColor1Button.color
    property alias cfg_particleColor2: particleColor2Button.color
    property alias cfg_particleSize: particleSizeSlider.value
    property alias cfg_particleSizeRandom: particleSizeRandomSlider.value

    Kirigami.FormLayout {
        anchors.fill: parent

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Particles")
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
            Kirigami.FormData.label: i18n("Base Size:")

            QQC2.Slider {
                id: particleSizeSlider

                Layout.fillWidth: true
                from: 0.5
                to: 10
                stepSize: 0.1
            }

            QQC2.Label {
                text: particleSizeSlider.value.toFixed(1)
            }

        }

        RowLayout {
            Kirigami.FormData.label: i18n("Random Variation:")

            QQC2.Slider {
                id: particleSizeRandomSlider

                Layout.fillWidth: true
                from: 0
                to: 20
                stepSize: 0.5
            }

            QQC2.Label {
                text: particleSizeRandomSlider.value.toFixed(1)
            }

        }


        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Colors")
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

}
