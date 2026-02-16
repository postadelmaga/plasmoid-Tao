#include "TaoPlugin.h"
#include "TaoQGraphHybrid.h"
#include <qqml.h>

void TaoPlugin::registerTypes(const char *uri)
{
    // Backend Hybrid (Async + Parallel + SceneGraph)
    qmlRegisterType<TaoQGraphHybrid>(uri, 1, 0, "TaoAnimationHybrid");
}
