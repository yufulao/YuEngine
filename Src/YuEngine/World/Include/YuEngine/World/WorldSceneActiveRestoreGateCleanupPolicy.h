// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneActiveRestoreGateCleanupPolicy.h

#pragma once

namespace yuengine::world {
enum class WorldSceneActiveRestoreGateCleanupPolicy {
    None,
    ReleaseObjectIdentity,
    ClearTransformRecord,
    RemoveComponentAttachment,
    ReleaseResourceBinding
};
}
