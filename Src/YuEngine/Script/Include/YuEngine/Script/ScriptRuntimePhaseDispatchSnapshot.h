// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptRuntimePhaseDispatchSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchStatus.h"
#include "YuEngine/Script/ScriptStatus.h"

namespace yuengine::script {
struct ScriptRuntimePhaseDispatchSnapshot final {
    std::uint32_t binding_capacity = 0U;
    std::uint32_t trace_capacity = 0U;
    std::uint32_t binding_count = 0U;
    std::uint64_t dispatched_call_count = 0U;
    std::uint64_t skipped_phase_count = 0U;
    std::uint64_t failed_dispatch_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    ScriptStatus last_script_status = ScriptStatus::Success;
    ScriptRuntimePhaseDispatchStatus last_status = ScriptRuntimePhaseDispatchStatus::Success;
};
}
