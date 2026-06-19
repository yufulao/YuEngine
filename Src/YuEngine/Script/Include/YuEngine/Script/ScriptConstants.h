// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptConstants.h

#pragma once

#include <cstdint>

namespace yuengine::script {
inline constexpr std::uint32_t INVALID_SCRIPT_CALL_ID_VALUE = 0U;
inline constexpr std::uint32_t MAX_SCRIPT_NATIVE_BINDING_COUNT = 32U;
inline constexpr std::uint32_t MAX_SCRIPT_ARGUMENT_COUNT = 8U;
inline constexpr std::uint32_t MAX_SCRIPT_RESULT_COUNT = 4U;
inline constexpr std::uint32_t MAX_SCRIPT_RUNTIME_PHASE_DISPATCH_BINDING_COUNT = 4U;
inline constexpr std::uint32_t MAX_SCRIPT_RUNTIME_PHASE_TRACE_COUNT = 32U;
}
