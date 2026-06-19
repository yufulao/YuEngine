// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptRuntimePhaseDispatchAdapterDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Script/ScriptConstants.h"

namespace yuengine::script {
struct ScriptRuntimePhaseDispatchAdapterDesc final {
    std::uint32_t binding_capacity = MAX_SCRIPT_RUNTIME_PHASE_DISPATCH_BINDING_COUNT;
    std::uint32_t trace_capacity = MAX_SCRIPT_RUNTIME_PHASE_TRACE_COUNT;
};
}
