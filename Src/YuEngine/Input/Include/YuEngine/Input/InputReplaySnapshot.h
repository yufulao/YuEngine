// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputReplaySnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Input/InputActionId.h"
#include "YuEngine/Input/InputAccountingStatus.h"
#include "YuEngine/Input/InputControlId.h"
#include "YuEngine/Input/InputDeviceId.h"
#include "YuEngine/Input/InputEventType.h"
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
    InputDeviceId last_failed_binding_device{};
    InputControlId last_failed_binding_control{};
    InputActionId last_failed_binding_action{};
    std::size_t last_failed_binding_capacity = 0U;
    std::size_t last_failed_binding_count = 0U;
    std::size_t last_required_binding_count = 0U;
    InputStatus last_status = InputStatus::Success;
    InputStatus last_apply_status = InputStatus::Success;
    std::size_t last_failed_frame_index = 0U;
    std::size_t last_failed_event_index = 0U;
    InputEventType last_failed_event_type = InputEventType::ButtonPressed;
    InputDeviceId last_failed_device{};
    InputControlId last_failed_control{};
    std::int32_t last_failed_axis_value = 0;
    std::size_t last_failed_event_capacity = 0U;
    std::size_t last_failed_event_count = 0U;
    std::size_t last_required_event_count = 0U;
    InputAccountingStatus allocation_accounting_status = InputAccountingStatus::DeferredUntilYuMemoryIntegration;
};
}
