// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFramePacketFixtureSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 固定容量 RenderCore frame packet fixture 计数器 和 last 状态 值。
 */
struct RenderFramePacketFixtureSnapshot final {
    std::size_t frame_packet_record_capacity = 0U;
    std::size_t frame_packet_record_count = 0U;
    std::uint64_t accepted_packet_count = 0U;
    std::uint64_t completed_packet_count = 0U;
    std::uint64_t failed_packet_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t duplicate_frame_id_count = 0U;
    std::uint64_t packet_capacity_rejected_count = 0U;
    std::uint64_t submission_batch_failure_count = 0U;
    std::uint64_t completed_entry_count = 0U;
    std::uint64_t failed_entry_count = 0U;
    std::uint32_t last_frame_id = 0U;
    std::size_t last_entry_count = 0U;
    std::size_t last_completed_entry_count = 0U;
    std::size_t last_failed_entry_count = 0U;
    std::size_t last_failed_entry_index = 0U;
    std::uint32_t last_pass_id = 0U;
    std::uint32_t last_material_id = 0U;
    RenderFramePacketFixtureStatus last_status = RenderFramePacketFixtureStatus::InvalidArgument;
    RenderSubmissionBatchFixtureStatus last_batch_status = RenderSubmissionBatchFixtureStatus::InvalidArgument;
    RenderFixturePassStatus last_pass_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
};
}
