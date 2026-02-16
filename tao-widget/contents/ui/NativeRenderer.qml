import QtQuick
import "native" as TaoNative

Item {
    anchors.fill: parent

    TaoNative.TaoAnimation {
        anchors.fill: parent
        // Use the 'root' ID from main.qml for direct property binding
        particleCount: root.particleCount
        rotationSpeed: root.rotationSpeed
        clockwise: root.clockwise
        showClock: root.showClock
        lowCpuMode: root.lowCpuMode
    }

}
