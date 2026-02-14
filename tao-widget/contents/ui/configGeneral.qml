import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    id: configRoot
    
    property alias cfg_particleCount: particleCountSlider.value
    property alias cfg_rotationSpeed: rotationSpeedSlider.value
    property alias cfg_clockwise: clockwiseRadio.checked

    // Numero di particelle
    RowLayout {
        Kirigami.FormData.label: i18n("Numero di particelle:")
        
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
            Layout.minimumWidth: Kirigami.Units.gridUnit * 2
        }
    }
    
    // Velocità di rotazione
    RowLayout {
        Kirigami.FormData.label: i18n("Velocità di rotazione:")
        
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
            Layout.minimumWidth: Kirigami.Units.gridUnit * 2
        }
    }
    
    // Direzione di rotazione
    QQC2.RadioButton {
        id: clockwiseRadio
        Kirigami.FormData.label: i18n("Direzione di rotazione:")
        text: i18n("Senso orario")
    }
    
    QQC2.RadioButton {
        id: counterClockwiseRadio
        text: i18n("Senso antiorario")
        checked: !clockwiseRadio.checked
        onToggled: {
            if (checked) {
                clockwiseRadio.checked = false
            }
        }
    }
}


