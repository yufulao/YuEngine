// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Src/ScriptRuntimePhaseDispatchAdapter.cpp

#include "YuEngine/Script/ScriptRuntimePhaseDispatchAdapter.h"

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Script/ScriptNativeRegistry.h"
#include "YuEngine/Script/ScriptValue.h"

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::script {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

ScriptRuntimePhaseDispatchAdapter::ScriptRuntimePhaseDispatchAdapter(
    ScriptRuntimePhaseDispatchAdapterDesc desc)
    : bindings_{},
      snapshot_{
          ClampCapacity(desc.binding_capacity, MAX_SCRIPT_RUNTIME_PHASE_DISPATCH_BINDING_COUNT),
          ClampCapacity(desc.trace_capacity, MAX_SCRIPT_RUNTIME_PHASE_TRACE_COUNT),
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ScriptStatus::Success,
          ScriptRuntimePhaseDispatchStatus::Success} {
    if (desc.binding_capacity == 0U) {
        snapshot_.last_status = ScriptRuntimePhaseDispatchStatus::InvalidBindingCapacity;
        return;
    }

    if (desc.trace_capacity == 0U) {
        snapshot_.last_status = ScriptRuntimePhaseDispatchStatus::InvalidTraceCapacity;
        return;
    }
}

ScriptRuntimePhaseDispatchResult ScriptRuntimePhaseDispatchAdapter::Bind(
    ScriptRuntimePhase phase,
    ScriptCallId call_id) {
    const ScriptRuntimePhaseDispatchStatus capacity_status = ValidateAdapterCapacity();
    if (capacity_status != ScriptRuntimePhaseDispatchStatus::Success) {
        return ScriptRuntimePhaseDispatchResult::Failure(RecordBindFailure(capacity_status));
    }

    if (!IsPhaseValid(phase)) {
        return ScriptRuntimePhaseDispatchResult::Failure(
            RecordBindFailure(ScriptRuntimePhaseDispatchStatus::InvalidPhase));
    }

    if (!call_id.IsValid()) {
        return ScriptRuntimePhaseDispatchResult::Failure(
            RecordBindFailure(ScriptRuntimePhaseDispatchStatus::InvalidCallId));
    }

    if (FindBindingByPhase(phase) != nullptr) {
        return ScriptRuntimePhaseDispatchResult::Failure(
            RecordBindFailure(ScriptRuntimePhaseDispatchStatus::DuplicatePhase));
    }

    if (snapshot_.binding_count >= snapshot_.binding_capacity) {
        return ScriptRuntimePhaseDispatchResult::Failure(
            RecordBindFailure(ScriptRuntimePhaseDispatchStatus::CapacityExceeded));
    }

    ScriptRuntimePhaseDispatchBinding *binding = FindFreeBinding();
    if (binding == nullptr) {
        return ScriptRuntimePhaseDispatchResult::Failure(
            RecordBindFailure(ScriptRuntimePhaseDispatchStatus::CapacityExceeded));
    }

    binding->phase = phase;
    binding->call_id = call_id;
    binding->is_bound = true;
    ++snapshot_.binding_count;
    RecordSuccess(ScriptStatus::Success);
    return ScriptRuntimePhaseDispatchResult::Success(phase, call_id);
}

ScriptRuntimePhaseDispatchStatus ScriptRuntimePhaseDispatchAdapter::DispatchTrace(
    ScriptNativeRegistry &registry,
    const ScriptRuntimePhaseTrace *phase_trace,
    std::uint32_t phase_trace_count,
    const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count) {
    const ScriptRuntimePhaseDispatchStatus input_status = ValidateDispatchInputs(
        phase_trace,
        phase_trace_count,
        arguments,
        argument_count,
        results,
        result_count);
    if (input_status != ScriptRuntimePhaseDispatchStatus::Success) {
        return RecordDispatchFailure(input_status);
    }

    std::uint32_t trace_index = 0U;
    while (trace_index < phase_trace_count) {
        const ScriptRuntimePhaseTrace &trace = phase_trace[trace_index];
        const ScriptRuntimePhaseDispatchBinding *binding = FindBindingByPhase(trace.phase);
        if (binding == nullptr) {
            ++snapshot_.skipped_phase_count;
            ++trace_index;
            continue;
        }

        const ScriptStatus script_status = registry.Invoke(
            binding->call_id,
            arguments,
            argument_count,
            results,
            result_count);
        if (script_status != ScriptStatus::Success) {
            const ScriptRuntimePhaseDispatchStatus mapped_status = MapScriptFailure(script_status);
            return RecordDispatchFailure(mapped_status, script_status);
        }

        ++snapshot_.dispatched_call_count;
        ++trace_index;
    }

    RecordSuccess(ScriptStatus::Success);
    return ScriptRuntimePhaseDispatchStatus::Success;
}

ScriptRuntimePhaseDispatchSnapshot ScriptRuntimePhaseDispatchAdapter::Snapshot() const {
    return snapshot_;
}

ScriptRuntimePhaseDispatchStatus ScriptRuntimePhaseDispatchAdapter::RecordBindFailure(
    ScriptRuntimePhaseDispatchStatus status) {
    snapshot_.last_status = status;
    return status;
}

ScriptRuntimePhaseDispatchStatus ScriptRuntimePhaseDispatchAdapter::RecordDispatchFailure(
    ScriptRuntimePhaseDispatchStatus status) {
    ++snapshot_.failed_dispatch_count;
    snapshot_.last_status = status;
    return status;
}

ScriptRuntimePhaseDispatchStatus ScriptRuntimePhaseDispatchAdapter::RecordDispatchFailure(
    ScriptRuntimePhaseDispatchStatus status,
    ScriptStatus script_status) {
    ++snapshot_.failed_dispatch_count;
    snapshot_.last_status = status;
    snapshot_.last_script_status = script_status;
    return status;
}

void ScriptRuntimePhaseDispatchAdapter::RecordSuccess(ScriptStatus script_status) {
    snapshot_.last_status = ScriptRuntimePhaseDispatchStatus::Success;
    snapshot_.last_script_status = script_status;
}

ScriptRuntimePhaseDispatchStatus ScriptRuntimePhaseDispatchAdapter::ValidateAdapterCapacity() const {
    if (snapshot_.binding_capacity == 0U) {
        return ScriptRuntimePhaseDispatchStatus::InvalidBindingCapacity;
    }

    if (snapshot_.trace_capacity == 0U) {
        return ScriptRuntimePhaseDispatchStatus::InvalidTraceCapacity;
    }

    return ScriptRuntimePhaseDispatchStatus::Success;
}

ScriptRuntimePhaseDispatchStatus ScriptRuntimePhaseDispatchAdapter::ValidateDispatchInputs(
    const ScriptRuntimePhaseTrace *phase_trace,
    std::uint32_t phase_trace_count,
    const ScriptValue *arguments,
    std::uint32_t argument_count,
    const ScriptValue *results,
    std::uint32_t result_count) const {
    const ScriptRuntimePhaseDispatchStatus capacity_status = ValidateAdapterCapacity();
    if (capacity_status != ScriptRuntimePhaseDispatchStatus::Success) {
        return capacity_status;
    }

    if (phase_trace_count > snapshot_.trace_capacity) {
        return ScriptRuntimePhaseDispatchStatus::TraceCapacityExceeded;
    }

    if ((phase_trace_count > 0U) && (phase_trace == nullptr)) {
        return ScriptRuntimePhaseDispatchStatus::InvalidTraceBuffer;
    }

    if ((argument_count > 0U) && (arguments == nullptr)) {
        return ScriptRuntimePhaseDispatchStatus::InvalidArgumentBuffer;
    }

    if ((result_count > 0U) && (results == nullptr)) {
        return ScriptRuntimePhaseDispatchStatus::InvalidResultBuffer;
    }

    std::uint32_t trace_index = 0U;
    while (trace_index < phase_trace_count) {
        const ScriptRuntimePhaseTrace &trace = phase_trace[trace_index];
        if (!IsPhaseValid(trace.phase)) {
            return ScriptRuntimePhaseDispatchStatus::InvalidPhase;
        }

        ++trace_index;
    }

    return ScriptRuntimePhaseDispatchStatus::Success;
}

ScriptRuntimePhaseDispatchStatus ScriptRuntimePhaseDispatchAdapter::MapScriptFailure(
    ScriptStatus script_status) const {
    if (script_status == ScriptStatus::InvalidCallId) {
        return ScriptRuntimePhaseDispatchStatus::MissingCall;
    }

    if (script_status == ScriptStatus::ArgumentCountMismatch) {
        return ScriptRuntimePhaseDispatchStatus::InvalidScriptSlot;
    }

    if (script_status == ScriptStatus::ArgumentTypeMismatch) {
        return ScriptRuntimePhaseDispatchStatus::InvalidScriptSlot;
    }

    if (script_status == ScriptStatus::ResultCountMismatch) {
        return ScriptRuntimePhaseDispatchStatus::InvalidScriptSlot;
    }

    if (script_status == ScriptStatus::ResultTypeMismatch) {
        return ScriptRuntimePhaseDispatchStatus::InvalidScriptSlot;
    }

    if (script_status == ScriptStatus::InvalidArgumentBuffer) {
        return ScriptRuntimePhaseDispatchStatus::InvalidScriptSlot;
    }

    if (script_status == ScriptStatus::InvalidResultBuffer) {
        return ScriptRuntimePhaseDispatchStatus::InvalidScriptSlot;
    }

    return ScriptRuntimePhaseDispatchStatus::ScriptCallFailed;
}

bool ScriptRuntimePhaseDispatchAdapter::IsPhaseValid(ScriptRuntimePhase phase) const {
    if (phase == ScriptRuntimePhase::BeginFrame) {
        return true;
    }

    if (phase == ScriptRuntimePhase::FixedStep) {
        return true;
    }

    if (phase == ScriptRuntimePhase::FrameStep) {
        return true;
    }

    if (phase == ScriptRuntimePhase::EndFrame) {
        return true;
    }

    return false;
}

ScriptRuntimePhaseDispatchBinding *ScriptRuntimePhaseDispatchAdapter::FindFreeBinding() {
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        ScriptRuntimePhaseDispatchBinding &binding = bindings_[index];
        if (binding.is_bound) {
            continue;
        }

        return &binding;
    }

    return nullptr;
}

const ScriptRuntimePhaseDispatchBinding *ScriptRuntimePhaseDispatchAdapter::FindBindingByPhase(
    ScriptRuntimePhase phase) const {
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        const ScriptRuntimePhaseDispatchBinding &binding = bindings_[index];
        if (!binding.is_bound) {
            continue;
        }

        if (binding.phase == phase) {
            return &binding;
        }
    }

    return nullptr;
}
}
