// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraShaderConstantsSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderCameraShaderConstantsStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 camera shader constant writer 计数器 和 last 状态 值。
 */
struct RenderCameraShaderConstantsSnapshot final {
    std::uint64_t accepted_write_count = 0U;
    std::uint64_t rejected_write_count = 0U;
    RenderCameraShaderConstantsStatus last_status = RenderCameraShaderConstantsStatus::InvalidArgument;
};
}
