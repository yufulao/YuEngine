// 模块: YuEngine UiRenderCoreBridge
// 文件: Src/YuEngine/UiRenderCoreBridge/Include/YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeRequest.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFixturePassResult.h"
#include "YuEngine/UiCore/UiDrawElement.h"

namespace yuengine::rendercore {
class RenderFixturePass;
class RenderSubmissionBatchFixture;
}

namespace yuengine::uirendercorebridge {
/**
 * @comment 描述一次 UiCore draw element 到 RenderCore fixture 的提交请求。
 */
struct UiRenderCoreBridgeRequest final {
    yuengine::rendercore::RenderFixturePass *pass = nullptr;
    yuengine::rendercore::RenderSubmissionBatchFixture *submission_batch = nullptr;
    std::span<const yuengine::uicore::UiDrawElement> draw_elements{};
    yuengine::rendercore::RenderFixturePassRequest template_pass_request{};
    std::span<yuengine::rendercore::RenderFixturePassRequest> pass_requests{};
    std::span<yuengine::rendercore::RenderFixturePassResult> pass_results{};
    std::uint32_t pass_id_base = 1U;
};
}
