import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    id: configGeneral
    
    // Numero di particelle
    QQC2.SpinBox {
        id: particleCountSpinBox
        Kirigami.FormData.label: "Numero di particelle:"
        from: 0
        to: 200
        value: plasmoid.configuration.particleCount
        stepSize: 10
        
        onValueChanged: {
            plasmoid.configuration.particleCount = value
        }
    }
    
    // Velocità di rotazione
    RowLayout {
        Kirigami.FormData.label: "Velocità di rotazione:"
        
        QQC2.Slider {
            id: rotationSpeedSlider
            Layout.fillWidth: true
            from: 0.0
            to: 0.02
            value: plasmoid.configuration.rotationSpeed
            stepSize: 0.001
            
            onValueChanged: {
                plasmoid.configuration.rotationSpeed = value
            }
        }
        
        QQC2.Label {
            text: rotationSpeedSlider.value.toFixed(3)
            Layout.minimumWidth: Kirigami.Units.gridUnit * 3
        }
    }
    
    // Direzione di rotazione
    QQC2.RadioButton {
        id: clockwiseRadio
        Kirigami.FormData.label: "Direzione di rotazione:"
        text: "Senso orario"
        checked: plasmoid.configuration.clockwise
        onCheckedChanged: {
            if (checked) {
                plasmoid.configuration.clockwise = true
            }
        }
    }
    
    QQC2.RadioButton {
        id: counterClockwiseRadio
        text: "Senso antiorario"
        checked: !plasmoid.configuration.clockwise
        onCheckedChanged: {
            if (checked) {
                plasmoid.configuration.clockwise = false
            }
        }
    }
    
    // Bottone per ripristinare i valori predefiniti
    Item {
        Kirigami.FormData.isSection: true
    }
    
    QQC2.Button {
        text: "Ripristina valori predefiniti"
        icon.name: "edit-reset"
        onClicked: {
            plasmoid.configuration.particleCount = 80
            plasmoid.configuration.rotationSpeed = 0.005
            plasmoid.configuration.clockwise = true
        }
    }
}
