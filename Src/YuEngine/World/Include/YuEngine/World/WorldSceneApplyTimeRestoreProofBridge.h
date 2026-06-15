// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneApplyTimeRestoreProofBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofBridgeDesc.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofRecord.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofResult.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofSnapshot.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofSliceRecord.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanRecord.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanResult.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldTransformStatus.h"

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

class WorldSceneApplyTimeRestoreProofBridge final {
public:
    /**
     * @comment Constructs an apply-time scene restore proof bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldSceneApplyTimeRestoreProofBridge(
        WorldSceneApplyTimeRestoreProofBridgeDesc desc=
            WorldSceneApplyTimeRestoreProofBridgeDesc{});

    /**
     * @comment Proves same-call restore readiness without active mutation.
     * @param world Caller-owned world instance used only for membership queries.
     * @param object_registry Caller-owned object registry used only for const acquire preflight.
     * @param resource_registry Caller-owned resource registry used only for const acquire preflight.
     * @param identity_destination Current identity destination used only for public snapshots.
     * @param transform_destination Current transform destination used only for public snapshots.
     * @param attachment_destination Current attachment destination used only for public preflight.
     * @param binding_destination Current binding destination used only for public preflight.
     * @param input_identities Caller-owned identity input records.
     * @param input_identity_count Input identity record count.
     * @param input_transforms Caller-owned transform input records.
     * @param input_transform_count Input transform record count.
     * @param input_attachments Caller-owned attachment input records.
     * @param input_attachment_count Input attachment record count.
     * @param input_bindings Caller-owned binding input records.
     * @param input_binding_count Input binding record count.
     * @param plan_scratch Caller-owned P3-GATE-021 plan scratch buffer.
     * @param plan_scratch_capacity Plan scratch buffer capacity.
     * @param output_proofs Caller-owned output proof buffer.
     * @param output_proof_capacity Output proof buffer capacity.
     * @param output_slices Caller-owned output active-call slice buffer.
     * @param output_slice_capacity Output active-call slice buffer capacity.
     * @return Explicit operation result.
     */
    WorldSceneApplyTimeRestoreProofResult Prove(
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
        WorldSceneApplyTimeRestoreProofRecord *output_proofs,
        std::uint32_t output_proof_capacity,
        WorldSceneApplyTimeRestoreProofSliceRecord *output_slices,
        std::uint32_t output_slice_capacity);
    /**
     * @comment Returns a snapshot of the current proof bridge state.
     * @return Snapshot value.
     */
    WorldSceneApplyTimeRestoreProofSnapshot Snapshot() const;

private:
    WorldSceneApplyTimeRestoreProofResult RecordFailure(
        WorldSceneApplyTimeRestoreProofStatus status);
    WorldSceneApplyTimeRestoreProofResult RecordFailure(
        WorldSceneApplyTimeRestoreProofStatus status,
        WorldSceneDecodedRestorePlanStatus plan_status,
        WorldObjectIdentityStatus identity_status,
        WorldTransformStatus transform_status,
        WorldComponentAttachmentStatus attachment_status,
        WorldComponentResourceBindingStatus binding_status,
        yuengine::object::ObjectStatus object_status,
        yuengine::resource::ResourceStatus resource_status);
    WorldSceneApplyTimeRestoreProofResult RecordRejectedFailure(
        WorldSceneApplyTimeRestoreProofStatus status);
    WorldSceneApplyTimeRestoreProofResult RecordPlanFailure(
        const WorldSceneDecodedRestorePlanResult &plan_result);
    WorldSceneApplyTimeRestoreProofResult RecordSuccess(
        const WorldSceneApplyTimeRestoreProofState &state);
    WorldSceneApplyTimeRestoreProofStatus ValidateBridgeCapacity() const;
    WorldSceneApplyTimeRestoreProofStatus ValidateInputs(
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
        const WorldSceneApplyTimeRestoreProofRecord *output_proofs,
        const WorldSceneApplyTimeRestoreProofSliceRecord *output_slices) const;
    WorldSceneApplyTimeRestoreProofStatus ValidateCounts(
        std::uint32_t input_identity_count,
        std::uint32_t input_transform_count,
        std::uint32_t input_attachment_count,
        std::uint32_t input_binding_count,
        std::uint32_t plan_scratch_capacity,
        std::uint32_t output_proof_capacity,
        std::uint32_t output_slice_capacity,
        std::uint32_t *out_record_count) const;
    WorldSceneApplyTimeRestoreProofStatus ValidatePlanRecords(
        const WorldSceneDecodedRestorePlanRecord *plan_scratch,
        std::uint32_t plan_record_count,
        std::uint32_t input_identity_count,
        std::uint32_t input_transform_count,
        std::uint32_t input_attachment_count,
        std::uint32_t input_binding_count) const;
    WorldSceneApplyTimeRestoreProofStatus ValidatePlanFamilyRange(
        const WorldSceneDecodedRestorePlanRecord *plan_scratch,
        std::uint32_t range_begin,
        std::uint32_t range_count,
        WorldSceneDecodedRestorePlanRecordFamily expected_family) const;
    WorldSceneApplyTimeRestoreProofStatus MapPlanStatus(
        WorldSceneDecodedRestorePlanStatus plan_status) const;
    WorldSceneApplyTimeRestoreProofFamily MapPlanFamily(
        WorldSceneDecodedRestorePlanRecordFamily family) const;
    void WriteProofOutputs(
        const WorldSceneDecodedRestorePlanRecord *plan_scratch,
        std::uint32_t plan_record_count,
        WorldSceneApplyTimeRestoreProofRecord *output_proofs,
        WorldSceneApplyTimeRestoreProofSliceRecord *output_slices,
        WorldSceneApplyTimeRestoreProofState *state) const;

    std::uint32_t identity_capacity_;
    std::uint32_t transform_capacity_;
    std::uint32_t attachment_capacity_;
    std::uint32_t binding_capacity_;
    std::uint32_t plan_scratch_capacity_;
    std::uint32_t proof_capacity_;
    std::uint32_t slice_capacity_;
    WorldSceneApplyTimeRestoreProofSnapshot snapshot_;
};
}
