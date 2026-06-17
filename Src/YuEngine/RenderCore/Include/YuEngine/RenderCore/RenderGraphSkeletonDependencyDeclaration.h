// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonDependencyDeclaration.h

#pragma once

#include <cstdint>

namespace yuengine::rendercore {
/**
 * @comment Declares one caller-owned dependency edge for a RenderCore render graph skeleton.
 */
struct RenderGraphSkeletonDependencyDeclaration final {
    std::uint32_t before_pass_id = 0U;
    std::uint32_t after_pass_id = 0U;
};
}
