// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputCommandMapperSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Input/InputContextFocusMode.h"
#include "YuEngine/Input/InputContextId.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputCommandMapperSnapshot final {
    std::size_t context_capacity = 0U;
    std::size_t binding_capacity = 0U;
    std::size_t command_capacity = 0U;
    std::size_t context_count = 0U;
    std::size_t binding_count = 0U;
    std::uint64_t build_count = 0U;
    std::uint64_t accepted_event_count = 0U;
    std::uint64_t rejected_event_count = 0U;
    std::uint64_t failed_operation_count = 0U;
    InputContextId active_context;
    InputContextFocusMode focus_mode = InputContextFocusMode::AcceptInput;
    InputStatus last_status = InputStatus::Success;
};
}
