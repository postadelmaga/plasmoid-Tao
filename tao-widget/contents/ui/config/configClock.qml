import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols as KQuickControls

KCM.SimpleKCM {
    property alias cfg_showClock: showClockCheckBox.checked
    property alias cfg_hourHandColor: hourHandColorButton.color
    property alias cfg_minuteHandColor: minuteHandColorButton.color
    property alias cfg_secondHandColor: secondHandColorButton.color
    property bool cfg_showClockDefault: false
    property color cfg_hourHandColorDefault: "#ffffff"
    property color cfg_minuteHandColorDefault: "#88ccff"
    property color cfg_secondHandColorDefault: "#ffaa88"

    Kirigami.FormLayout {
        anchors.fill: parent

        QQC2.CheckBox {
            id: showClockCheckBox

            Kirigami.FormData.label: i18n("Visibility:")
            text: i18n("Show Clock")
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            visible: showClockCheckBox.checked
            Kirigami.FormData.label: i18n("Colors")
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Hour hand:")
            visible: showClockCheckBox.checked

            KQuickControls.ColorButton {
                id: hourHandColorButton

                Layout.fillWidth: true
            }

        }

        RowLayout {
            Kirigami.FormData.label: i18n("Minute hand:")
            visible: showClockCheckBox.checked

            KQuickControls.ColorButton {
                id: minuteHandColorButton

                Layout.fillWidth: true
            }

        }

        RowLayout {
            Kirigami.FormData.label: i18n("Second hand:")
            visible: showClockCheckBox.checked

            KQuickControls.ColorButton {
                id: secondHandColorButton

                Layout.fillWidth: true
            }

        }

    }

}
