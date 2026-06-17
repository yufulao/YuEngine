// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonPassDeclaration.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassRequest.h"

namespace yuengine::rendercore {
/**
 * @comment Declares one caller-owned pass value for a RenderCore render graph skeleton.
 */
struct RenderGraphSkeletonPassDeclaration final {
    std::uint32_t pass_id = 0U;
    RenderFixturePassRequest pass_request{};
};
}
