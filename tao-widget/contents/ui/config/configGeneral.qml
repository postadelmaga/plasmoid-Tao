import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

KCM.SimpleKCM {
    property alias cfg_renderEngine: engineCombo.currentIndex
    property alias cfg_transparentBackground: transparentBgCheckBox.checked

    Kirigami.FormLayout {
        anchors.fill: parent

        QQC2.ComboBox {
            id: engineCombo

            Kirigami.FormData.label: i18n("Render Engine:")
            model: [i18n("WebGL (Browser)"), i18n("Zen (Native C++)")]
        }

        // Zen Engine Status Message
        Kirigami.InlineMessage {
            id: zenStatusMessage

            property bool nativeAvailable: false

            Layout.fillWidth: true
            type: nativeTestLoader.status === Loader.Ready ? Kirigami.MessageType.Information : Kirigami.MessageType.Warning
            text: nativeTestLoader.status === Loader.Ready ? i18n("Zen engine is available.") : i18n("Zen engine is NOT available (using WebGL fallback).")
            visible: nativeTestLoader.status !== Loader.Ready

            // In configGeneral.qml
            Loader {
                id: nativeTestLoader

                active: true
                source: "ZenCheck.qml"
                // Distruggi l'istanza appena abbiamo il risultato
                onStatusChanged: {
                    if (status === Loader.Ready || status === Loader.Error) {
                        nativeAvailable = (status === Loader.Ready);
                        active = false; // distrugge l'istanza immediatamente
                    }
                }
            }

        }

        QQC2.CheckBox {
            id: transparentBgCheckBox

            Kirigami.FormData.label: i18n("Background:")
            text: i18n("Transparent background")
        }

    }

}
