// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldUpdatePhase.h

#pragma once

namespace yuengine::world {
enum class WorldUpdatePhase {
    BeginFrame,
    FixedStep,
    FrameStep,
    EndFrame
};
}
