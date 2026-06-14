#pragma once

#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiFormat.h"

namespace yuengine::rhi {
struct RhiColorTargetDesc final {
    RhiFormat Format = RhiFormat::Rgba8Unorm;
    RhiExtent2D Extent{};
};
}
