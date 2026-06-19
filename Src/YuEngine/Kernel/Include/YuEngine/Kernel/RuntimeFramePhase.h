// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/RuntimeFramePhase.h

#pragma once

namespace yuengine::kernel {
enum class RuntimeFramePhase {
    BeginFrame,
    PollPlatform,
    PollInput,
    LoadOrCommitResources,
    UpdateWorld,
    PrepareRender,
    SubmitAudio,
    SubmitRender,
    Present,
    EndFrame
};
}
