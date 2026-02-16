#include "TaoPlugin.h"
#include "TaoQuickItem.h"
#include "TaoQGraphGemini.h"
#include "TaoQGraphClaude.h"
#include "TaoQGraphHybrid.h"
#include <qqml.h>

void TaoPlugin::registerTypes(const char *uri)
{
    // Backend standard (Painted)
    qmlRegisterType<TaoQuickItem>(uri, 1, 0, "TaoAnimation");
    
    // Backend Gemini (Scene Graph - GPU)
    qmlRegisterType<TaoQGraphGemini>(uri, 1, 0, "TaoAnimationGemini");
    
    // Backend Claude (Scene Graph - Multi-threaded)
    qmlRegisterType<TaoQGraphClaude>(uri, 1, 0, "TaoAnimationClaude");

    // Backend Hybrid (Async + Parallel)
    qmlRegisterType<TaoQGraphHybrid>(uri, 1, 0, "TaoAnimationHybrid");
}
