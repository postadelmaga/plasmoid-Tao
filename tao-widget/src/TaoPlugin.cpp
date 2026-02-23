#include "TaoPlugin.h"
#include "TaoNew.h"
#include <qqml.h>

void TaoPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<TaoNew>(uri, 1, 0, "TaoNew");
}