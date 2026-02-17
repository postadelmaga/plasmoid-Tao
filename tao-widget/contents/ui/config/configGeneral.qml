import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    id: page

    property alias cfg_lowCpuMode: lowCpuCheckBox.checked
    property alias cfg_renderEngine: engineCombo.currentIndex
    property alias cfg_transparentBackground: transparentBgCheckBox.checked
    property bool cfg_lowCpuModeDefault: false
    property int cfg_renderEngineDefault: 1
    property bool cfg_transparentBackgroundDefault: true

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
