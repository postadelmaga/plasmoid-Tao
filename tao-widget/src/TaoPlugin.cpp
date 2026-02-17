#include "TaoPlugin.h"
#include "TaoQGraphHybrid.h"
#include <qqml.h>

void TaoPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<TaoQGraphHybrid>(uri, 1, 0, "TaoAnimationHybrid");
}