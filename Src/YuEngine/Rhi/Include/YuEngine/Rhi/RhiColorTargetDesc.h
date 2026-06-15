// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiColorTargetDesc.h

#pragma once

#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiFormat.h"

namespace yuengine::rhi {
struct RhiColorTargetDesc final {
    RhiFormat format = RhiFormat::Rgba8Unorm;
    RhiExtent2D extent{};
};
}
