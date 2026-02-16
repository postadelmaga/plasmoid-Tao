#include "TaoPlugin.h"
#include "TaoQuickItem.h"
#include <qqml.h>

void TaoPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<TaoQuickItem>(uri, 1, 0, "TaoAnimation");
}
