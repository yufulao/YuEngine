// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPrimitiveRetirementDrainRequest.h

#pragma once

#include <cstddef>

namespace yuengine::rhi {
struct RhiPrimitiveRetirementDrainRequest final {
    std::size_t max_retirements = 0U;
};
}
