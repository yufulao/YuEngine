// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputReplaySnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Input/InputAccountingStatus.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputReplaySnapshot final {
    std::size_t device_capacity = 0U;
    std::size_t action_capacity = 0U;
    std::size_t binding_capacity = 0U;
    std::size_t replay_frame_capacity = 0U;
    std::size_t event_capacity_per_frame = 0U;
    std::size_t replay_storage_capacity_before_frame = 0U;
    std::size_t replay_storage_capacity_after_last_frame = 0U;
    std::size_t action_count = 0U;
    std::size_t binding_count = 0U;
    std::size_t changed_action_count = 0U;
    std::uint64_t accepted_event_count = 0U;
    std::uint64_t rejected_event_count = 0U;
    std::uint64_t apply_count = 0U;
    std::uint64_t reset_count = 0U;
    std::uint64_t failed_operation_count = 0U;
    InputStatus last_apply_status = InputStatus::Success;
    InputAccountingStatus allocation_accounting_status = InputAccountingStatus::DeferredUntilYuMemoryIntegration;
};
}
