// Module: YuEngine World
// File: Src/YuEngine/World/Src/WorldScriptDispatchBridge.cpp

#include "YuEngine/World/WorldScriptDispatchBridge.h"

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Script/ScriptNativeRegistry.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::script::ScriptStatus;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

WorldScriptDispatchBridge::WorldScriptDispatchBridge(WorldScriptDispatchBridgeDesc desc)
    : bindings_{},
      snapshot_{
          ClampCapacity(desc.binding_capacity, MAX_WORLD_SCRIPT_DISPATCH_BINDING_COUNT),
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ScriptStatus::Success,
          WorldScriptDispatchStatus::Success} {
    if (desc.binding_capacity == 0U) {
        snapshot_.last_status = WorldScriptDispatchStatus::InvalidBindingCapacity;
        return;
    }
}

WorldScriptDispatchResult WorldScriptDispatchBridge::Bind(WorldUpdatePhase phase,
    yuengine::script::ScriptCallId call_id) {
    const WorldScriptDispatchStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldScriptDispatchStatus::Success) {
        return WorldScriptDispatchResult::Failure(RecordBindFailure(capacity_status));
    }

    if (!IsPhaseValid(phase)) {
        return WorldScriptDispatchResult::Failure(RecordBindFailure(WorldScriptDispatchStatus::InvalidPhase));
    }

    if (!call_id.IsValid()) {
        return WorldScriptDispatchResult::Failure(RecordBindFailure(WorldScriptDispatchStatus::InvalidCallId));
    }

    if (FindBindingByPhase(phase) != nullptr) {
        return WorldScriptDispatchResult::Failure(RecordBindFailure(WorldScriptDispatchStatus::DuplicatePhase));
    }

    if (snapshot_.binding_count >= snapshot_.binding_capacity) {
        return WorldScriptDispatchResult::Failure(RecordBindFailure(WorldScriptDispatchStatus::CapacityExceeded));
    }

    WorldScriptDispatchBinding *binding = FindFreeBinding();
    if (binding == nullptr) {
        return WorldScriptDispatchResult::Failure(RecordBindFailure(WorldScriptDispatchStatus::CapacityExceeded));
    }

    binding->phase = phase;
    binding->call_id = call_id;
    binding->is_bound = true;
    ++snapshot_.binding_count;
    RecordSuccess(ScriptStatus::Success);
    return WorldScriptDispatchResult::Success(phase, call_id);
}

WorldScriptDispatchStatus WorldScriptDispatchBridge::DispatchTrace(
    yuengine::script::ScriptNativeRegistry &registry,
    const WorldPhaseTrace *phase_trace,
    std::uint32_t phase_trace_count,
    const yuengine::script::ScriptValue *arguments,
    std::uint32_t argument_count,
    yuengine::script::ScriptValue *results,
    std::uint32_t result_count) {
    const WorldScriptDispatchStatus input_status = ValidateDispatchInputs(
        phase_trace,
        phase_trace_count,
        arguments,
        argument_count,
        results,
        result_count);
    if (input_status != WorldScriptDispatchStatus::Success) {
        return RecordDispatchFailure(input_status);
    }

    std::uint32_t trace_index = 0U;
    while (trace_index < phase_trace_count) {
        const WorldPhaseTrace &trace = phase_trace[trace_index];
        const WorldScriptDispatchBinding *binding = FindBindingByPhase(trace.phase);
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
            return RecordDispatchFailure(WorldScriptDispatchStatus::ScriptCallFailed, script_status);
        }

        ++snapshot_.dispatched_call_count;
        ++trace_index;
    }

    RecordSuccess(ScriptStatus::Success);
    return WorldScriptDispatchStatus::Success;
}

WorldScriptDispatchSnapshot WorldScriptDispatchBridge::Snapshot() const {
    return snapshot_;
}

WorldScriptDispatchStatus WorldScriptDispatchBridge::RecordBindFailure(WorldScriptDispatchStatus status) {
    snapshot_.last_status = status;
    return status;
}

WorldScriptDispatchStatus WorldScriptDispatchBridge::RecordDispatchFailure(WorldScriptDispatchStatus status) {
    ++snapshot_.failed_dispatch_count;
    snapshot_.last_status = status;
    return status;
}

WorldScriptDispatchStatus WorldScriptDispatchBridge::RecordDispatchFailure(WorldScriptDispatchStatus status,
    yuengine::script::ScriptStatus script_status) {
    ++snapshot_.failed_dispatch_count;
    snapshot_.last_status = status;
    snapshot_.last_script_status = script_status;
    return status;
}

void WorldScriptDispatchBridge::RecordSuccess(yuengine::script::ScriptStatus script_status) {
    snapshot_.last_status = WorldScriptDispatchStatus::Success;
    snapshot_.last_script_status = script_status;
}

WorldScriptDispatchStatus WorldScriptDispatchBridge::ValidateBridgeCapacity() const {
    if (snapshot_.binding_capacity == 0U) {
        return WorldScriptDispatchStatus::InvalidBindingCapacity;
    }

    return WorldScriptDispatchStatus::Success;
}

WorldScriptDispatchStatus WorldScriptDispatchBridge::ValidateDispatchInputs(
    const WorldPhaseTrace *phase_trace,
    std::uint32_t phase_trace_count,
    const yuengine::script::ScriptValue *arguments,
    std::uint32_t argument_count,
    const yuengine::script::ScriptValue *results,
    std::uint32_t result_count) const {
    const WorldScriptDispatchStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldScriptDispatchStatus::Success) {
        return capacity_status;
    }

    if (phase_trace_count > MAX_WORLD_PHASE_TRACE_COUNT) {
        return WorldScriptDispatchStatus::TraceCapacityExceeded;
    }

    if (phase_trace_count > 0U && phase_trace == nullptr) {
        return WorldScriptDispatchStatus::InvalidTraceBuffer;
    }

    if (argument_count > 0U && arguments == nullptr) {
        return WorldScriptDispatchStatus::InvalidArgumentBuffer;
    }

    if (result_count > 0U && results == nullptr) {
        return WorldScriptDispatchStatus::InvalidResultBuffer;
    }

    std::uint32_t trace_index = 0U;
    while (trace_index < phase_trace_count) {
        const WorldPhaseTrace &trace = phase_trace[trace_index];
        if (!IsPhaseValid(trace.phase)) {
            return WorldScriptDispatchStatus::InvalidPhase;
        }

        ++trace_index;
    }

    return WorldScriptDispatchStatus::Success;
}

bool WorldScriptDispatchBridge::IsPhaseValid(WorldUpdatePhase phase) const {
    if (phase == WorldUpdatePhase::BeginFrame) {
        return true;
    }

    if (phase == WorldUpdatePhase::FixedStep) {
        return true;
    }

    if (phase == WorldUpdatePhase::FrameStep) {
        return true;
    }

    if (phase == WorldUpdatePhase::EndFrame) {
        return true;
    }

    return false;
}

WorldScriptDispatchBinding *WorldScriptDispatchBridge::FindFreeBinding() {
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        WorldScriptDispatchBinding &binding = bindings_[index];
        if (binding.is_bound) {
            continue;
        }

        return &binding;
    }

    return nullptr;
}

const WorldScriptDispatchBinding *WorldScriptDispatchBridge::FindBindingByPhase(WorldUpdatePhase phase) const {
    for (std::uint32_t index = 0U; index < snapshot_.binding_capacity; ++index) {
        const WorldScriptDispatchBinding &binding = bindings_[index];
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
