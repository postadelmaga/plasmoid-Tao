import QtQuick
import "native" as TaoNative

Item {
    anchors.fill: parent

    // Backend selection logic
    Loader {
        anchors.fill: parent
        sourceComponent: {
            switch (root.nativeBackendType) {
            case 1:
                return geminiComponent;
            case 2:
                return claudeComponent;
            case 3:
                return hybridComponent;
            default:
                return standardComponent;
            }
        }
    }

    Component {
        id: standardComponent

        TaoNative.TaoAnimation {
            particleCount: root.particleCount
            rotationSpeed: root.rotationSpeed
            clockwise: root.clockwise
            showClock: root.showClock
            lowCpuMode: root.lowCpuMode
        }

    }

    Component {
        id: geminiComponent

        TaoNative.TaoAnimationGemini {
            particleCount: root.particleCount
            rotationSpeed: root.rotationSpeed
            clockwise: root.clockwise
            showClock: root.showClock
            lowCpuMode: root.lowCpuMode
        }

    }

    Component {
        id: claudeComponent

        TaoNative.TaoAnimationClaude {
            particleCount: root.particleCount
            rotationSpeed: root.rotationSpeed
            clockwise: root.clockwise
            showClock: root.showClock
            lowCpuMode: root.lowCpuMode
        }

    }

    Component {
        id: hybridComponent

        TaoNative.TaoAnimationHybrid {
            particleCount: root.particleCount
            rotationSpeed: root.rotationSpeed
            clockwise: root.clockwise
            showClock: root.showClock
            lowCpuMode: root.lowCpuMode
        }

    }

}
