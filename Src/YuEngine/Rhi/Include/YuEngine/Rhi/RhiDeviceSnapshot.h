// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Rhi/RhiAccountingStatus.h"
#include "YuEngine/Rhi/RhiResourceSnapshot.h"
#include "YuEngine/Rhi/RhiSwapchainSnapshot.h"

namespace yuengine::rhi {
struct RhiDeviceSnapshot final {
    std::size_t color_target_capacity = 0U;
    std::size_t color_target_count = 0U;
    std::size_t command_storage_capacity_before_frame = 0U;
    std::size_t command_storage_capacity_after_last_frame = 0U;
    std::uint64_t created_target_count = 0U;
    std::uint64_t destroyed_target_count = 0U;
    std::uint64_t recorded_command_count = 0U;
    std::uint64_t submitted_draw_count = 0U;
    std::uint64_t submitted_indexed_draw_count = 0U;
    std::uint64_t submitted_sampled_texture_bind_count = 0U;
    std::uint64_t submitted_sampler_bind_count = 0U;
    std::uint64_t rejected_indexed_draw_count = 0U;
    std::uint64_t rejected_sampled_texture_bind_count = 0U;
    std::uint64_t rejected_sampler_bind_count = 0U;
    std::uint64_t submit_count = 0U;
    std::uint64_t present_count = 0U;
    std::uint64_t capture_count = 0U;
    std::uint64_t failed_operation_count = 0U;
    std::uint32_t last_draw_vertex_count = 0U;
    std::uint32_t last_indexed_draw_index_count = 0U;
    std::uint32_t last_bound_sampled_texture_slot = 0U;
    std::uint32_t last_bound_sampler_slot = 0U;
    std::size_t last_bound_index_buffer_offset_bytes = 0U;
    std::size_t last_bound_index_buffer_size_bytes = 0U;
    std::size_t last_capture_bytes_written = 0U;
    RhiSwapchainSnapshot swapchain{};
    RhiResourceSnapshot resources{};
    RhiAccountingStatus allocation_accounting_status = RhiAccountingStatus::DeferredUntilYuMemoryIntegration;
};
}
