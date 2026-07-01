// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/Rhi/RhiCommandType.h"
#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 结果 的 一个 fixture pass execution。
 */
struct RenderFixturePassResult final {
    RenderFixturePassStatus status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    std::size_t recorded_command_count = 0U;
    std::size_t command_capacity = 0U;
    std::size_t current_command_count = 0U;
    std::size_t required_command_count = 0U;
    std::size_t pass_record_capacity = 0U;
    std::size_t current_pass_record_count = 0U;
    std::size_t required_pass_record_count = 0U;
    std::size_t failed_command_index = 0U;
    yuengine::rhi::RhiCommandType failed_command_type = yuengine::rhi::RhiCommandType::BeginFrame;
    std::size_t failed_pass_record_index = 0U;
    std::size_t capture_bytes_written = 0U;
    yuengine::rhi::RhiExtent2D capture_extent{};
    std::uint32_t pass_id = 0U;
};
}
