// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldScriptDispatchBridge.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/Script/ScriptStatus.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldPhaseTrace.h"
#include "YuEngine/World/WorldScriptDispatchBinding.h"
#include "YuEngine/World/WorldScriptDispatchBridgeDesc.h"
#include "YuEngine/World/WorldScriptDispatchConstants.h"
#include "YuEngine/World/WorldScriptDispatchResult.h"
#include "YuEngine/World/WorldScriptDispatchSnapshot.h"
#include "YuEngine/World/WorldScriptDispatchStatus.h"
#include "YuEngine/World/WorldUpdatePhase.h"

namespace yuengine::script {
class ScriptNativeRegistry;
struct ScriptValue;
}

namespace yuengine::world {
class WorldScriptDispatchBridge final {
public:
    /**
     * @comment Constructs a world-script dispatch bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldScriptDispatchBridge(WorldScriptDispatchBridgeDesc desc=WorldScriptDispatchBridgeDesc{});

    /**
     * @comment Binds one world update phase to one script call id.
     * @param phase Input world update phase.
     * @param call_id Input script call id.
     * @return Explicit operation result.
     */
    WorldScriptDispatchResult Bind(WorldUpdatePhase phase,
        yuengine::script::ScriptCallId call_id);
    /**
     * @comment Dispatches bound world phases in trace order.
     * @param registry Script native registry used for dispatch.
     * @param phase_trace Input world phase trace records.
     * @param phase_trace_count Input trace record count.
     * @param arguments Caller-owned argument slots.
     * @param argument_count Input argument slot count.
     * @param results Caller-owned result slots.
     * @param result_count Input result slot count.
     * @return Explicit operation status.
     */
    WorldScriptDispatchStatus DispatchTrace(
        yuengine::script::ScriptNativeRegistry &registry,
        const WorldPhaseTrace *phase_trace,
        std::uint32_t phase_trace_count,
        const yuengine::script::ScriptValue *arguments,
        std::uint32_t argument_count,
        yuengine::script::ScriptValue *results,
        std::uint32_t result_count);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldScriptDispatchSnapshot Snapshot() const;

private:
    WorldScriptDispatchStatus RecordBindFailure(WorldScriptDispatchStatus status);
    WorldScriptDispatchStatus RecordDispatchFailure(WorldScriptDispatchStatus status);
    WorldScriptDispatchStatus RecordDispatchFailure(WorldScriptDispatchStatus status,
        yuengine::script::ScriptStatus script_status);
    void RecordSuccess(yuengine::script::ScriptStatus script_status);
    WorldScriptDispatchStatus ValidateBridgeCapacity() const;
    WorldScriptDispatchStatus ValidateDispatchInputs(
        const WorldPhaseTrace *phase_trace,
        std::uint32_t phase_trace_count,
        const yuengine::script::ScriptValue *arguments,
        std::uint32_t argument_count,
        const yuengine::script::ScriptValue *results,
        std::uint32_t result_count) const;
    bool IsPhaseValid(WorldUpdatePhase phase) const;
    WorldScriptDispatchBinding *FindFreeBinding();
    const WorldScriptDispatchBinding *FindBindingByPhase(WorldUpdatePhase phase) const;

    std::array<WorldScriptDispatchBinding, MAX_WORLD_SCRIPT_DISPATCH_BINDING_COUNT> bindings_;
    WorldScriptDispatchSnapshot snapshot_;
};
}
