import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

KCM.SimpleKCM {
    property alias cfg_lowCpuMode: lowCpuCheckBox.checked
    property alias cfg_renderEngine: engineCombo.currentIndex
    property alias cfg_transparentBackground: transparentBgCheckBox.checked

    Kirigami.FormLayout {
        anchors.fill: parent

        QQC2.ComboBox {
            id: engineCombo

            Kirigami.FormData.label: i18n("Render Engine:")
            model: [i18n("WebGL (Browser)"), i18n("Zen (Native C++)")]
        }

        QQC2.CheckBox {
            id: lowCpuCheckBox

            Kirigami.FormData.label: i18n("Performance:")
            text: i18n("Low CPU Mode (disables shadows/glows)")
        }

        QQC2.CheckBox {
            id: transparentBgCheckBox

            Kirigami.FormData.label: i18n("Background:")
            text: i18n("Transparent background")
        }

    }

}
