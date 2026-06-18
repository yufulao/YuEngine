// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h

#pragma once

#include <span>

#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFixturePassResult.h"

namespace yuengine::rendercore {
class RenderFixturePass;

/**
 * @comment 描述 一个 调用方持有 RenderCore submission batch 操作.
 */
struct RenderSubmissionBatchFixtureRequest final {
    RenderFixturePass *pass = nullptr;
    std::span<const RenderFixturePassRequest> pass_requests{};
    std::span<RenderFixturePassResult> pass_results{};
};
}
