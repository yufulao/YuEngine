// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptRuntimePhaseTrace.h

#pragma once

#include <cstdint>

#include "YuEngine/Script/ScriptRuntimePhase.h"

namespace yuengine::script {
struct ScriptRuntimePhaseTrace final {
    ScriptRuntimePhase phase = ScriptRuntimePhase::BeginFrame;
    std::uint64_t frame_index = 0U;
    std::uint32_t active_object_count = 0U;
    std::uint32_t skipped_object_count = 0U;
};
}
