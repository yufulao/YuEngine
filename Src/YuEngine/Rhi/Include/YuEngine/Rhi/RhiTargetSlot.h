// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiTargetSlot.h

#pragma once

#include <cstdint>
#include <vector>

#include "YuEngine/Rhi/RhiColorTargetDesc.h"

namespace yuengine::rhi {
struct RhiTargetSlot final {
    bool is_active = false;
    std::uint32_t generation = 1U;
    RhiColorTargetDesc desc{};
    std::vector<std::uint8_t> bytes{};
};
}
