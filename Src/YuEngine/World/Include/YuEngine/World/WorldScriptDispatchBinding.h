// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldScriptDispatchBinding.h

#pragma once

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/World/WorldUpdatePhase.h"

namespace yuengine::world {
struct WorldScriptDispatchBinding final {
    WorldUpdatePhase phase = WorldUpdatePhase::BeginFrame;
    yuengine::script::ScriptCallId call_id{};
    bool is_bound = false;
};
}
