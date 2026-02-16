import QtQuick
import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
        name: i18n("General")
        icon: "preferences-system"
        source: "configGeneral.qml"
    }

    ConfigCategory {
        name: i18n("Appearance")
        icon: "preferences-desktop-color"
        source: "configAppearance.qml"
    }

    ConfigCategory {
        name: i18n("Particles")
        icon: "view-media-playlist-video"
        source: "configParticles.qml"
    }

    ConfigCategory {
        name: i18n("Clock")
        icon: "office-calendar"
        source: "configClock.qml"
    }

}
