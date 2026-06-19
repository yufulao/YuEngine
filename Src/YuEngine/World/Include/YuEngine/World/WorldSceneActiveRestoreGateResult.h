// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneActiveRestoreGateResult.h

#pragma once

#include "YuEngine/World/WorldSceneActiveRestoreGateState.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofStatus.h"

namespace yuengine::world {
struct WorldSceneActiveRestoreGateResult final {
    WorldSceneActiveRestoreGateStatus status =
        WorldSceneActiveRestoreGateStatus::Success;
    WorldSceneApplyTimeRestoreProofStatus proof_status =
        WorldSceneApplyTimeRestoreProofStatus::Success;
    WorldSceneActiveRestoreGateState state{};

    static WorldSceneActiveRestoreGateResult Success(
        const WorldSceneActiveRestoreGateState &state) {
        return WorldSceneActiveRestoreGateResult{
            WorldSceneActiveRestoreGateStatus::Success,
            WorldSceneApplyTimeRestoreProofStatus::Success,
            state};
    }

    static WorldSceneActiveRestoreGateResult Failure(
        WorldSceneActiveRestoreGateStatus status,
        WorldSceneApplyTimeRestoreProofStatus proof_status=
            WorldSceneApplyTimeRestoreProofStatus::Success) {
        return WorldSceneActiveRestoreGateResult{
            status,
            proof_status,
            WorldSceneActiveRestoreGateState{}};
    }

    bool Succeeded() const {
        return status == WorldSceneActiveRestoreGateStatus::Success;
    }
};
}
