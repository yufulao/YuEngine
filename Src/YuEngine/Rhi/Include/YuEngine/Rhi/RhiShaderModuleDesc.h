// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiShaderModuleDesc.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Rhi/RhiShaderStage.h"

namespace yuengine::rhi {
struct RhiShaderModuleDesc final {
    RhiShaderStage stage = RhiShaderStage::Unsupported;
    std::span<const std::uint8_t> bytecode{};
};
}
