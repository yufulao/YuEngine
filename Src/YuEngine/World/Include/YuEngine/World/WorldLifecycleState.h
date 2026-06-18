// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldLifecycleState.h

#pragma once

namespace yuengine::world {
enum class WorldLifecycleState {
    Created,
    Starting,
    Running,
    Stopping,
    Stopped,
    Failed
};
}
