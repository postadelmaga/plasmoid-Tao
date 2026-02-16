#ifndef PLUGIN_H
#define PLUGIN_H

#include <QQmlExtensionPlugin>

class TaoPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // PLUGIN_H
