// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridge.h

#pragma once

#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffResult.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffSnapshot.h"

namespace yuengine::runtimeassetworldadapter {
class RuntimeAssetWorldObjectRestoreHandoffBridge final {
public:
    /**
     * @comment 将 RuntimeAssetWorldObject adapter records 交接给 World gate/proof/restore 链路。
     * @param request 调用方持有的输入、scratch 和输出数组。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectRestoreHandoffResult ApplyRestore(
        const RuntimeAssetWorldObjectRestoreHandoffRequest &request);

    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    RuntimeAssetWorldObjectRestoreHandoffSnapshot Snapshot() const;

private:
    RuntimeAssetWorldObjectRestoreHandoffResult RecordFailure(
        RuntimeAssetWorldObjectRestoreHandoffStatus status);
    RuntimeAssetWorldObjectRestoreHandoffResult RecordAdapterFailure(
        RuntimeAssetWorldObjectAdapterStatus adapter_status);
    RuntimeAssetWorldObjectRestoreHandoffResult RecordGateFailure(
        yuengine::world::WorldSceneActiveRestoreGateStatus gate_status,
        yuengine::world::WorldSceneApplyTimeRestoreProofStatus proof_status);
    RuntimeAssetWorldObjectRestoreHandoffResult RecordRestoreFailure(
        yuengine::world::WorldSceneObjectTransformRestoreStatus restore_status);
    RuntimeAssetWorldObjectRestoreHandoffResult RecordSuccess(
        const RuntimeAssetWorldObjectRestoreHandoffState &state);
    RuntimeAssetWorldObjectRestoreHandoffStatus ValidateRequest(
        const RuntimeAssetWorldObjectRestoreHandoffRequest &request) const;

    RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot_;
};
}
