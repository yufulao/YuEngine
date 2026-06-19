// 模块: YuEngine UiRenderCoreBridge
// 文件: Src/YuEngine/UiRenderCoreBridge/Include/YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeResult.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeStatus.h"

namespace yuengine::uirendercorebridge {
/**
 * @comment 包含一次 bridge 提交的结果和 RenderCore 下游状态。
 */
struct UiRenderCoreBridgeResult final {
    UiRenderCoreBridgeStatus status = UiRenderCoreBridgeStatus::InvalidArgument;
    std::size_t draw_element_count = 0U;
    std::size_t submitted_entry_count = 0U;
    std::size_t completed_entry_count = 0U;
    std::size_t failed_entry_index = 0U;
    yuengine::uicore::UiNodeId failed_node_id{};
    yuengine::rendercore::RenderSubmissionBatchFixtureStatus submission_status =
        yuengine::rendercore::RenderSubmissionBatchFixtureStatus::InvalidArgument;
    yuengine::rendercore::RenderFixturePassStatus pass_status =
        yuengine::rendercore::RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;

    /**
     * @comment 判断 bridge 提交是否成功。
     * @return 成功返回 true。
     */
    bool Succeeded() const {
        return status == UiRenderCoreBridgeStatus::Success;
    }
};
}
