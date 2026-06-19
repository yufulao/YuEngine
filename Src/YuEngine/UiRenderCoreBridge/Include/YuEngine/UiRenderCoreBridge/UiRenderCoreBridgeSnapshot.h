// 模块: YuEngine UiRenderCoreBridge
// 文件: Src/YuEngine/UiRenderCoreBridge/Include/YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeStatus.h"

namespace yuengine::uirendercorebridge {
/**
 * @comment 记录 bridge 的确定性计数器和最后一次提交状态。
 */
struct UiRenderCoreBridgeSnapshot final {
    std::uint64_t accepted_draw_count = 0U;
    std::uint64_t submitted_draw_count = 0U;
    std::uint64_t completed_draw_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t submission_failure_count = 0U;
    std::size_t last_draw_element_count = 0U;
    std::size_t last_completed_entry_count = 0U;
    std::size_t last_failed_entry_index = 0U;
    yuengine::uicore::UiNodeId last_failed_node_id{};
    UiRenderCoreBridgeStatus last_status = UiRenderCoreBridgeStatus::InvalidArgument;
    yuengine::rendercore::RenderSubmissionBatchFixtureStatus last_submission_status =
        yuengine::rendercore::RenderSubmissionBatchFixtureStatus::InvalidArgument;
    yuengine::rendercore::RenderFixturePassStatus last_pass_status =
        yuengine::rendercore::RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
};
}
