// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptRuntimePhaseDispatchSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/Script/ScriptRuntimePhase.h"
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
    ScriptRuntimePhase last_failed_phase = ScriptRuntimePhase::BeginFrame;
    ScriptCallId last_failed_call_id{};
    std::uint32_t last_failed_binding_capacity = 0U;
    std::uint32_t last_failed_binding_count = 0U;
    std::uint32_t last_required_binding_count = 0U;
    std::uint32_t last_failed_trace_capacity = 0U;
    std::uint32_t last_failed_trace_count = 0U;
    std::uint32_t last_failed_trace_index = 0U;
    std::uint32_t last_required_trace_count = 0U;
};
}
