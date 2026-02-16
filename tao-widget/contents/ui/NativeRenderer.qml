import QtQuick
import "native" as TaoNative

Item {
    id: renderer
    property int particleCount: 0
    property int rotationSpeed: 0
    property bool clockwise: false
    property bool showClock: false
    property bool lowCpuMode: false
    property point mousePos: Qt.point(0, 0)

    anchors.fill: parent

    // Solo backend Hybrid disponibile
    TaoNative.TaoAnimationHybrid {
        anchors.fill: parent
        particleCount: renderer.particleCount
        rotationSpeed: renderer.rotationSpeed
        clockwise: renderer.clockwise
        showClock: renderer.showClock
        lowCpuMode: renderer.lowCpuMode
        mousePos: renderer.mousePos
    }

}
