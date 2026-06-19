// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptRuntimePhaseDispatchBinding.h

#pragma once

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/Script/ScriptRuntimePhase.h"

namespace yuengine::script {
struct ScriptRuntimePhaseDispatchBinding final {
    ScriptRuntimePhase phase = ScriptRuntimePhase::BeginFrame;
    ScriptCallId call_id{};
    bool is_bound = false;
};
}
