// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonDependencyDeclaration.h

#pragma once

#include <cstdint>

namespace yuengine::rendercore {
/**
 * @comment 声明 一个 调用方持有 dependency edge 用于 一个 RenderCore render graph skeleton。
 */
struct RenderGraphSkeletonDependencyDeclaration final {
    std::uint32_t before_pass_id = 0U;
    std::uint32_t after_pass_id = 0U;
};
}
