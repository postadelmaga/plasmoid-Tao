import QtQuick
import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
        name: i18n("General")
        icon: "preferences-system"
        source: "config/configGeneral.qml"
    }

    ConfigCategory {
        name: i18n("Tao")
        icon: "preferences-desktop-color"
        source: "config/configTao.qml"
    }

    ConfigCategory {
        name: i18n("Particles")
        icon: "view-media-playlist-video"
        source: "config/configParticles.qml"
    }

    ConfigCategory {
        name: i18n("Clock")
        icon: "office-calendar"
        source: "config/configClock.qml"
    }

}
