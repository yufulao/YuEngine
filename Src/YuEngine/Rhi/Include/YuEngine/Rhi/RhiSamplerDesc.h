// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSamplerDesc.h

#pragma once

namespace yuengine::rhi {
struct RhiSamplerDesc final {
    bool linear_filter = false;
    bool clamp_to_edge = true;
};
}
