// Module: YuEngine Script
// File: Src/YuEngine/Script/Include/YuEngine/Script/ScriptNativeBinding.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/Script/ScriptConstants.h"
#include "YuEngine/Script/ScriptStatus.h"
#include "YuEngine/Script/ScriptValue.h"
#include "YuEngine/Script/ScriptValueType.h"

namespace yuengine::script {
using ScriptNativeFunction = ScriptStatus (*)(const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count);

struct ScriptNativeBinding final {
    ScriptCallId call_id{};
    ScriptNativeFunction function = nullptr;
    std::uint32_t argument_count = 0U;
    std::array<ScriptValueType, MAX_SCRIPT_ARGUMENT_COUNT> argument_types{};
    std::uint32_t result_count = 0U;
    std::array<ScriptValueType, MAX_SCRIPT_RESULT_COUNT> result_types{};
};
}
