// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputBridgeSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Input/InputBackendKind.h"
#include "YuEngine/Input/InputBridgeEvent.h"
#include "YuEngine/Input/InputGamepadConnection.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputBridgeSnapshot final {
    InputBackendKind backend = InputBackendKind::Replay;
    std::size_t event_capacity = 0U;
    std::size_t queued_event_count = 0U;
    std::size_t required_output_event_count = 0U;
    std::size_t failed_output_event_index = 0U;
    InputBridgeEventType failed_output_event_type = InputBridgeEventType::None;
    InputDeviceKind failed_output_device_kind = InputDeviceKind::Unknown;
    InputDeviceId failed_output_device{};
    InputControlId failed_output_control{};
    std::size_t accepted_event_count = 0U;
    std::size_t rejected_event_count = 0U;
    std::size_t drained_event_count = 0U;
    std::size_t overflow_count = 0U;
    std::size_t focus_lost_count = 0U;
    std::size_t focus_gained_count = 0U;
    std::size_t unavailable_count = 0U;
    std::size_t unsupported_backend_count = 0U;
    std::size_t failed_operation_count = 0U;
    std::size_t max_queued_event_count = 0U;
    std::size_t gamepad_poll_count = 0U;
    std::size_t gamepad_connected_poll_count = 0U;
    std::size_t gamepad_unavailable_poll_count = 0U;
    std::size_t gamepad_event_count = 0U;
    std::uint32_t last_gamepad_packet_number = 0U;
    std::size_t last_failed_gamepad_event_capacity = 0U;
    std::size_t last_failed_gamepad_event_count = 0U;
    std::size_t last_required_gamepad_event_count = 0U;
    InputDeviceId last_failed_gamepad_device{};
    InputGamepadConnection last_failed_gamepad_connection = InputGamepadConnection::Unavailable;
    std::uint32_t last_failed_gamepad_packet_number = 0U;
    std::uint16_t last_failed_gamepad_button_bits = 0U;
    std::uint8_t last_failed_gamepad_left_trigger = 0U;
    std::uint8_t last_failed_gamepad_right_trigger = 0U;
    std::int32_t last_failed_gamepad_left_thumb_x = 0;
    std::int32_t last_failed_gamepad_left_thumb_y = 0;
    std::int32_t last_failed_gamepad_right_thumb_x = 0;
    std::int32_t last_failed_gamepad_right_thumb_y = 0;
    InputGamepadConnection gamepad_connection = InputGamepadConnection::Unavailable;
    InputStatus last_status = InputStatus::NotInitialized;
    bool initialized = false;
    bool focused = false;
};
}
