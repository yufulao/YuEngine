// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/World/WorldLifecycleState.h"
#include "YuEngine/World/WorldStatus.h"

namespace yuengine::world {
struct WorldSnapshot final {
    std::uint32_t object_capacity = 0U;
    std::uint32_t phase_trace_capacity = 0U;
    std::uint32_t registered_object_count = 0U;
    std::uint32_t active_object_count = 0U;
    std::uint64_t frame_count = 0U;
    std::uint64_t phase_execution_count = 0U;
    std::uint64_t skipped_object_count = 0U;
    std::uint64_t last_frame_index = 0U;
    std::uint64_t last_fixed_step_duration = 0U;
    std::uint64_t last_frame_delta_duration = 0U;
    std::uint32_t phase_trace_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    WorldLifecycleState lifecycle_state = WorldLifecycleState::Created;
    WorldStatus last_status = WorldStatus::Success;
};
}
