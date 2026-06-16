// Module: YuEngine Input
// File: Src/YuEngine/Input/Include/YuEngine/Input/InputBridgeSnapshot.h

#pragma once

#include <cstddef>

#include "YuEngine/Input/InputBackendKind.h"
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
    InputStatus last_status = InputStatus::NotInitialized;
    bool initialized = false;
    bool focused = false;
};
}
