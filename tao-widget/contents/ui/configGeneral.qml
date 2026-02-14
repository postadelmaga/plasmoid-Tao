import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    id: configRoot

    property alias cfg_particleCount: particleCountSlider.value
    property alias cfg_rotationSpeed: rotationSpeedSlider.value
    property alias cfg_clockwise: clockwiseRadio.checked

    // --- Sezione Particelle ---
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18n("Effetti Particelle")
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Quantità:")
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

    // --- Sezione Movimento ---
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18n("Configurazione Movimento")
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Velocità:")
        spacing: Kirigami.Units.smallSpacing

        QQC2.Slider {
            id: rotationSpeedSlider

            Layout.fillWidth: true
            from: 1
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

    // Spazio tra i sezionatori
    Item {
        Kirigami.FormData.isSection: true
    }

    ColumnLayout {
        Kirigami.FormData.label: i18n("Senso di rotazione:")
        spacing: Kirigami.Units.smallSpacing

        QQC2.RadioButton {
            id: clockwiseRadio

            text: i18n("Orario")
            checked: true
        }

        QQC2.RadioButton {
            id: counterClockwiseRadio

            text: i18n("Antiorario")
            checked: !clockwiseRadio.checked
        }

    }

    // Nota informativa a fondo pagina
    QQC2.Label {
        Layout.fillWidth: true
        text: i18n("Le modifiche verranno applicate istantaneamente all'animazione.")
        font.pixelSize: Kirigami.Units.fontMetrics.font.pixelSize * 0.9
        opacity: 0.6
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignLeft
    }

}
