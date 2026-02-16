import QtQuick
import "native" as TaoNative

Item {
    property point mousePos: Qt.point(0, 0)

    anchors.fill: parent

    // Solo backend Hybrid disponibile
    TaoNative.TaoAnimationHybrid {
        anchors.fill: parent
        particleCount: root.particleCount || 0
        rotationSpeed: root.rotationSpeed || 0
        clockwise: !!root.clockwise
        showClock: !!root.showClock
        lowCpuMode: !!root.lowCpuMode
        mousePos: parent.mousePos
    }

}
