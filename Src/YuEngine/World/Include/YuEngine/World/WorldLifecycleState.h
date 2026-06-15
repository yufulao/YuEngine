// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldLifecycleState.h

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
