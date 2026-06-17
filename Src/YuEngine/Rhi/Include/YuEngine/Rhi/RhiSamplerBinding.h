// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSamplerBinding.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiSamplerHandle.h"

namespace yuengine::rhi {
struct RhiSamplerBinding final {
    RhiSamplerHandle sampler{};
    std::uint32_t slot = 0U;
};
}
