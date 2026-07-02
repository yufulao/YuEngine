// Module: YuEngine Input
// File: Src/YuEngine/Input/Include/YuEngine/Input/InputActionStateSnapshotResult.h

#pragma once

#include <cstddef>

#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputActionStateSnapshotResult final {
    InputStatus status = InputStatus::Success;
    std::size_t output_capacity = 0U;
    std::size_t required_action_count = 0U;
    std::size_t written_action_count = 0U;
};
}
