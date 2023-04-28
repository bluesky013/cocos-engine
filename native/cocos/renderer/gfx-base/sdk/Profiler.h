#pragma once

#include "gfx-base/sdk/perfetto.h"
// The set of track event categories that the example is using.
PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("engine")
        .SetDescription("cocos engine event"),
    perfetto::Category("rendering")
        .SetDescription("cocos render event"));
