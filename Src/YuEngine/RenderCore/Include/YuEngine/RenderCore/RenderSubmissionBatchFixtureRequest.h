// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h

#pragma once

#include <span>

#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFixturePassResult.h"

namespace yuengine::rendercore {
class RenderFixturePass;

/**
 * @comment Describes one caller-owned RenderCore submission batch operation.
 */
struct RenderSubmissionBatchFixtureRequest final {
    RenderFixturePass *pass = nullptr;
    std::span<const RenderFixturePassRequest> pass_requests{};
    std::span<RenderFixturePassResult> pass_results{};
};
}
