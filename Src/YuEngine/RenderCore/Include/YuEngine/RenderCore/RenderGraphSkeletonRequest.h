// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonRequest.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFixturePassResult.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonDependencyDeclaration.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonPassDeclaration.h"

namespace yuengine::rendercore {
class RenderFixturePass;

/**
 * @comment Describes one caller-owned RenderCore render graph skeleton prepare operation.
 */
struct RenderGraphSkeletonRequest final {
    std::uint32_t graph_id = 0U;
    RenderFixturePass *pass = nullptr;
    std::span<const RenderGraphSkeletonPassDeclaration> pass_declarations{};
    std::span<const RenderGraphSkeletonDependencyDeclaration> dependency_declarations{};
    std::span<RenderFixturePassRequest> prepared_pass_requests{};
    std::span<RenderFixturePassResult> pass_results{};
};
}
