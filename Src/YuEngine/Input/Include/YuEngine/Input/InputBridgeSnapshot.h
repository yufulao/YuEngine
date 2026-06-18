// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputBridgeSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Input/InputBackendKind.h"
#include "YuEngine/Input/InputGamepadConnection.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputBridgeSnapshot final {
    InputBackendKind backend = InputBackendKind::Replay;
    std::size_t event_capacity = 0U;
    std::size_t queued_event_count = 0U;
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
    InputGamepadConnection gamepad_connection = InputGamepadConnection::Unavailable;
    InputStatus last_status = InputStatus::NotInitialized;
    bool initialized = false;
    bool focused = false;
};
}
