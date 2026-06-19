// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneActiveRestoreGateBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateCleanupPolicy.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateDesc.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateRecord.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateResult.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateSnapshot.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofRecord.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofResult.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofSliceRecord.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofStatus.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"

namespace yuengine::object {
class ObjectRegistry;
}

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::world {
class WorldComponentAttachmentBridge;
class WorldComponentResourceBindingBridge;
class WorldInstance;
class WorldObjectIdentityBridge;
class WorldTransformBridge;

class WorldSceneActiveRestoreGateBridge final {
public:
    /**
     * @comment 构造 active restore gate bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldSceneActiveRestoreGateBridge(
        WorldSceneActiveRestoreGateDesc desc=WorldSceneActiveRestoreGateDesc{});

    /**
     * @comment 生成 active restore 前置 gate records，不修改 active world sidecars。
     * @param world 调用方持有的 world instance used only for membership queries。
     * @param object_registry 调用方持有的 object registry used only for const acquire preflight。
     * @param resource_registry 调用方持有的 resource registry used only for const acquire preflight。
     * @param identity_destination 当前 identity destination，只用于 preflight。
     * @param transform_destination 当前 transform destination，只用于 preflight。
     * @param attachment_destination 当前 attachment destination，只用于 preflight。
     * @param binding_destination 当前 binding destination，只用于 preflight。
     * @param input_identities 调用方持有的 identity input records。
     * @param input_identity_count 输入 identity record count。
     * @param input_transforms 调用方持有的 transform input records。
     * @param input_transform_count 输入 transform record count。
     * @param input_attachments 调用方持有的 attachment input records。
     * @param input_attachment_count 输入 attachment record count。
     * @param input_bindings 调用方持有的 binding input records。
     * @param input_binding_count 输入 binding record count。
     * @param plan_scratch 调用方持有的 decoded restore plan scratch。
     * @param plan_scratch_capacity plan scratch capacity。
     * @param proof_scratch 调用方持有的 apply-time proof scratch。
     * @param proof_scratch_capacity proof scratch capacity。
     * @param slice_scratch 调用方持有的 active-call slice scratch。
     * @param slice_scratch_capacity slice scratch capacity。
     * @param output_gates 调用方持有的 active restore gate output records。
     * @param output_gate_capacity output gate record capacity。
     * @return 显式操作结果。
     */
    WorldSceneActiveRestoreGateResult BuildGate(
        const WorldInstance *world,
        const yuengine::object::ObjectRegistry *object_registry,
        const yuengine::resource::ResourceRegistry *resource_registry,
        const WorldObjectIdentityBridge *identity_destination,
        const WorldTransformBridge *transform_destination,
        const WorldComponentAttachmentBridge *attachment_destination,
        const WorldComponentResourceBindingBridge *binding_destination,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count,
        WorldSceneDecodedRestorePlanRecord *plan_scratch,
        std::uint32_t plan_scratch_capacity,
        WorldSceneApplyTimeRestoreProofRecord *proof_scratch,
        std::uint32_t proof_scratch_capacity,
        WorldSceneApplyTimeRestoreProofSliceRecord *slice_scratch,
        std::uint32_t slice_scratch_capacity,
        WorldSceneActiveRestoreGateRecord *output_gates,
        std::uint32_t output_gate_capacity);

    /**
     * @comment 返回当前 gate bridge 状态快照。
     * @return 快照值。
     */
    WorldSceneActiveRestoreGateSnapshot Snapshot() const;

private:
    WorldSceneActiveRestoreGateResult RecordFailure(
        WorldSceneActiveRestoreGateStatus status);
    WorldSceneActiveRestoreGateResult RecordFailure(
        WorldSceneActiveRestoreGateStatus status,
        WorldSceneApplyTimeRestoreProofStatus proof_status);
    WorldSceneActiveRestoreGateResult RecordRejectedFailure(
        WorldSceneActiveRestoreGateStatus status);
    WorldSceneActiveRestoreGateResult RecordProofFailure(
        const WorldSceneApplyTimeRestoreProofResult &proof_result);
    WorldSceneActiveRestoreGateResult RecordSuccess(
        const WorldSceneActiveRestoreGateState &state);
    WorldSceneActiveRestoreGateStatus ValidateBridgeCapacity() const;
    WorldSceneActiveRestoreGateStatus ValidateInputs(
        const WorldInstance *world,
        const yuengine::object::ObjectRegistry *object_registry,
        const yuengine::resource::ResourceRegistry *resource_registry,
        const WorldObjectIdentityBridge *identity_destination,
        const WorldTransformBridge *transform_destination,
        const WorldComponentAttachmentBridge *attachment_destination,
        const WorldComponentResourceBindingBridge *binding_destination,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        const WorldSceneDecodedRestorePlanRecord *plan_scratch,
        const WorldSceneApplyTimeRestoreProofRecord *proof_scratch,
        const WorldSceneApplyTimeRestoreProofSliceRecord *slice_scratch,
        const WorldSceneActiveRestoreGateRecord *output_gates) const;
    WorldSceneActiveRestoreGateStatus ValidateCounts(
        std::uint32_t input_identity_count,
        std::uint32_t input_transform_count,
        std::uint32_t input_attachment_count,
        std::uint32_t input_binding_count,
        std::uint32_t plan_scratch_capacity,
        std::uint32_t proof_scratch_capacity,
        std::uint32_t slice_scratch_capacity,
        std::uint32_t output_gate_capacity,
        std::uint32_t *out_record_count) const;
    WorldSceneActiveRestoreGateStatus ValidateSlices(
        const WorldSceneApplyTimeRestoreProofSliceRecord *slice_scratch,
        std::uint32_t slice_record_count) const;
    WorldSceneActiveRestoreGateCleanupPolicy MapCleanupPolicy(
        WorldSceneApplyTimeRestoreProofFamily family) const;
    WorldSceneActiveRestoreGateStatus MapProofStatus(
        const WorldSceneApplyTimeRestoreProofResult &proof_result) const;
    void WriteGateRecords(
        const WorldSceneApplyTimeRestoreProofSliceRecord *slice_scratch,
        std::uint32_t slice_record_count,
        WorldSceneActiveRestoreGateRecord *output_gates,
        WorldSceneActiveRestoreGateState *state) const;

    std::uint32_t identity_capacity_;
    std::uint32_t transform_capacity_;
    std::uint32_t attachment_capacity_;
    std::uint32_t binding_capacity_;
    std::uint32_t plan_scratch_capacity_;
    std::uint32_t proof_scratch_capacity_;
    std::uint32_t slice_scratch_capacity_;
    std::uint32_t gate_output_capacity_;
    WorldSceneActiveRestoreGateSnapshot snapshot_;
};
}
