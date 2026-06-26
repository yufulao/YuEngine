// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiConstantBufferBinding.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiShaderStage.h"

namespace yuengine::rhi {
struct RhiConstantBufferBinding final {
    RhiBufferHandle buffer{};
    RhiShaderStage stage = RhiShaderStage::Unsupported;
    std::uint32_t slot = 0U;
};
}
