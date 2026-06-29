// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffResult.h

#pragma once

#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterStatus.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffState.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffStatus.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreStatus.h"

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectRestoreHandoffResult final {
    RuntimeAssetWorldObjectRestoreHandoffStatus status =
        RuntimeAssetWorldObjectRestoreHandoffStatus::Success;
    RuntimeAssetWorldObjectAdapterStatus adapter_status =
        RuntimeAssetWorldObjectAdapterStatus::Success;
    yuengine::world::WorldSceneActiveRestoreGateStatus gate_status =
        yuengine::world::WorldSceneActiveRestoreGateStatus::Success;
    yuengine::world::WorldSceneApplyTimeRestoreProofStatus proof_status =
        yuengine::world::WorldSceneApplyTimeRestoreProofStatus::Success;
    yuengine::world::WorldSceneObjectTransformRestoreStatus restore_status =
        yuengine::world::WorldSceneObjectTransformRestoreStatus::Success;
    RuntimeAssetWorldObjectRestoreHandoffState state{};

    /**
     * @comment 创建成功结果。
     * @param state 输出 handoff 状态。
     * @return 成功结果。
     */
    static RuntimeAssetWorldObjectRestoreHandoffResult Success(
        RuntimeAssetWorldObjectRestoreHandoffState state);

    /**
     * @comment 创建失败结果。
     * @param status 显式失败状态。
     * @param adapter_status Adapter build 状态。
     * @param gate_status World active gate 状态。
     * @param proof_status World proof 状态。
     * @param restore_status World restore 状态。
     * @return 失败结果。
     */
    static RuntimeAssetWorldObjectRestoreHandoffResult Failure(
        RuntimeAssetWorldObjectRestoreHandoffStatus status,
        RuntimeAssetWorldObjectAdapterStatus adapter_status=
            RuntimeAssetWorldObjectAdapterStatus::Success,
        yuengine::world::WorldSceneActiveRestoreGateStatus gate_status=
            yuengine::world::WorldSceneActiveRestoreGateStatus::Success,
        yuengine::world::WorldSceneApplyTimeRestoreProofStatus proof_status=
            yuengine::world::WorldSceneApplyTimeRestoreProofStatus::Success,
        yuengine::world::WorldSceneObjectTransformRestoreStatus restore_status=
            yuengine::world::WorldSceneObjectTransformRestoreStatus::Success);

    /**
     * @comment 检查结果是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const;
};
}
