#pragma once

#include "yuengine/rhi/rhi_extent_2d.h"
#include "yuengine/rhi/rhi_format.h"

namespace yuengine::rhi {
struct RhiColorTargetDesc final {
    RhiFormat Format = RhiFormat::Rgba8Unorm;
    RhiExtent2D Extent{};
};
}
