// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptRuntimePhase.h

#pragma once

namespace yuengine::script {
enum class ScriptRuntimePhase {
    BeginFrame,
    FixedStep,
    FrameStep,
    EndFrame
};
}
