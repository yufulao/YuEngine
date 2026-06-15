// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldScriptDispatchResult.h

#pragma once

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/World/WorldScriptDispatchStatus.h"
#include "YuEngine/World/WorldUpdatePhase.h"

namespace yuengine::world {
struct WorldScriptDispatchResult final {
    WorldScriptDispatchStatus status = WorldScriptDispatchStatus::Success;
    WorldUpdatePhase phase = WorldUpdatePhase::BeginFrame;
    yuengine::script::ScriptCallId call_id{};

    /**
     * @comment Creates a successful result.
     * @param phase Input world update phase.
     * @param call_id Input script call id.
     * @return Explicit operation result.
     */
    static WorldScriptDispatchResult Success(WorldUpdatePhase phase,
        yuengine::script::ScriptCallId call_id) {
        return WorldScriptDispatchResult{WorldScriptDispatchStatus::Success, phase, call_id};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input dispatch status.
     * @return Explicit operation result.
     */
    static WorldScriptDispatchResult Failure(WorldScriptDispatchStatus status) {
        return WorldScriptDispatchResult{status, WorldUpdatePhase::BeginFrame, yuengine::script::ScriptCallId{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the result succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldScriptDispatchStatus::Success;
    }
};
}
