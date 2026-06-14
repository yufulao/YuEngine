#pragma once

#include "yuengine/rhi/RhiExtent2D.h"
#include "yuengine/rhi/RhiFormat.h"

namespace yuengine::rhi {
struct RhiColorTargetDesc final {
    RhiFormat Format = RhiFormat::Rgba8Unorm;
    RhiExtent2D Extent{};
};
}
