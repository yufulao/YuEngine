// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonRecord.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderGraphSkeletonStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 一个 保留的 RenderCore render graph skeleton 记录。
 */
struct RenderGraphSkeletonRecord final {
    std::uint32_t graph_id = 0U;
    std::size_t pass_count = 0U;
    std::size_t dependency_count = 0U;
    std::uint32_t first_pass_id = 0U;
    std::uint32_t last_pass_id = 0U;
    RenderGraphSkeletonStatus status = RenderGraphSkeletonStatus::InvalidArgument;
};
}
