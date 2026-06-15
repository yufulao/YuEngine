// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneDecodedRestorePlanBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanBridgeDesc.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanRecord.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanResult.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanSnapshot.h"
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

class WorldSceneDecodedRestorePlanBridge final {
public:
    /**
     * @comment Constructs a decoded scene restore plan bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldSceneDecodedRestorePlanBridge(
        WorldSceneDecodedRestorePlanBridgeDesc desc=
            WorldSceneDecodedRestorePlanBridgeDesc{});

    /**
     * @comment Builds a no-mutation decoded scene restore plan after full preflight.
     * @param world Caller-owned world instance used only for membership queries.
     * @param object_registry Caller-owned object registry used only for const acquire preflight.
     * @param resource_registry Caller-owned resource registry used only for const acquire preflight.
     * @param identity_destination Optional identity destination for public capacity checks.
     * @param transform_destination Optional transform destination for public capacity checks.
     * @param attachment_destination Optional attachment destination for public capacity checks.
     * @param binding_destination Optional binding destination for public capacity checks.
     * @param input_identities Caller-owned identity input records.
     * @param input_identity_count Input identity record count.
     * @param input_transforms Caller-owned transform input records.
     * @param input_transform_count Input transform record count.
     * @param input_attachments Caller-owned attachment input records.
     * @param input_attachment_count Input attachment record count.
     * @param input_bindings Caller-owned binding input records.
     * @param input_binding_count Input binding record count.
     * @param output_plan Caller-owned output plan buffer.
     * @param output_plan_capacity Output plan buffer capacity.
     * @return Explicit operation result.
     */
    WorldSceneDecodedRestorePlanResult Plan(
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
        WorldSceneDecodedRestorePlanRecord *output_plan,
        std::uint32_t output_plan_capacity);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldSceneDecodedRestorePlanSnapshot Snapshot() const;

private:
    WorldSceneDecodedRestorePlanResult RecordFailure(
        WorldSceneDecodedRestorePlanStatus status);
    WorldSceneDecodedRestorePlanResult RecordFailure(
        WorldSceneDecodedRestorePlanStatus status,
        WorldObjectIdentityStatus identity_status,
        WorldTransformStatus transform_status,
        WorldComponentAttachmentStatus attachment_status,
        WorldComponentResourceBindingStatus binding_status,
        yuengine::object::ObjectStatus object_status,
        yuengine::resource::ResourceStatus resource_status);
    WorldSceneDecodedRestorePlanResult RecordRejectedFailure(
        WorldSceneDecodedRestorePlanStatus status);
    WorldSceneDecodedRestorePlanResult RecordRejectedFailure(
        WorldSceneDecodedRestorePlanStatus status,
        yuengine::object::ObjectStatus object_status);
    WorldSceneDecodedRestorePlanResult RecordRejectedFailure(
        WorldSceneDecodedRestorePlanStatus status,
        yuengine::resource::ResourceStatus resource_status);
    WorldSceneDecodedRestorePlanResult RecordSuccess(
        const WorldSceneDecodedRestorePlanState &state);
    WorldSceneDecodedRestorePlanStatus ValidateBridgeCapacity() const;
    WorldSceneDecodedRestorePlanStatus ValidateInputCounts(
        std::uint32_t input_identity_count,
        std::uint32_t input_transform_count,
        std::uint32_t input_attachment_count,
        std::uint32_t input_binding_count,
        std::uint32_t output_plan_capacity,
        std::uint32_t *out_plan_record_count) const;
    WorldSceneDecodedRestorePlanStatus ValidateDestinations(
        const WorldObjectIdentityBridge *identity_destination,
        const WorldTransformBridge *transform_destination,
        const WorldComponentAttachmentBridge *attachment_destination,
        const WorldComponentResourceBindingBridge *binding_destination,
        std::uint32_t input_identity_count,
        std::uint32_t input_transform_count,
        std::uint32_t input_attachment_count,
        std::uint32_t input_binding_count,
        WorldObjectIdentityStatus *out_identity_status,
        WorldTransformStatus *out_transform_status,
        WorldComponentAttachmentStatus *out_attachment_status,
        WorldComponentResourceBindingStatus *out_binding_status) const;
    WorldSceneDecodedRestorePlanStatus ValidateIdentityRecords(
        const WorldInstance &world,
        const yuengine::object::ObjectRegistry &object_registry,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        yuengine::object::ObjectStatus *out_object_status) const;
    WorldSceneDecodedRestorePlanStatus ValidateTransformRecords(
        const WorldInstance &world,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count) const;
    WorldSceneDecodedRestorePlanStatus ValidateAttachmentRecords(
        const WorldInstance &world,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count) const;
    WorldSceneDecodedRestorePlanStatus ValidateBindingRecords(
        const WorldInstance &world,
        const yuengine::resource::ResourceRegistry &resource_registry,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count,
        yuengine::resource::ResourceStatus *out_resource_status) const;
    bool HasDuplicateIdentityWorldObjectId(
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        std::uint32_t record_index) const;
    bool HasDuplicateIdentityObjectHandle(
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        std::uint32_t record_index) const;
    bool HasDuplicateTransformWorldObjectId(
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count,
        std::uint32_t record_index) const;
    bool HasDuplicateAttachmentTuple(
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t record_index) const;
    bool HasDuplicateBindingTuple(
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t record_index) const;
    bool HasIdentityRecord(
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        WorldObjectId world_object_id) const;
    bool HasAttachmentTuple(
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord &binding) const;
    std::uint32_t CountProjectedObjectAcquire(
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        std::uint32_t record_index) const;
    std::uint32_t CountProjectedResourceAcquire(
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count,
        std::uint32_t record_index) const;
    WorldSceneDecodedRestorePlanStatus MapObjectStatus(
        yuengine::object::ObjectStatus object_status) const;
    WorldSceneDecodedRestorePlanStatus MapResourceStatus(
        yuengine::resource::ResourceStatus resource_status) const;
    WorldSceneDecodedRestorePlanStatus MapAttachmentDestinationStatus(
        WorldComponentAttachmentStatus attachment_status) const;
    WorldSceneDecodedRestorePlanStatus MapBindingDestinationStatus(
        WorldComponentResourceBindingStatus binding_status) const;
    void WritePlanRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count,
        WorldSceneDecodedRestorePlanRecord *output_plan,
        WorldSceneDecodedRestorePlanState *state) const;

    std::uint32_t identity_capacity_;
    std::uint32_t transform_capacity_;
    std::uint32_t attachment_capacity_;
    std::uint32_t binding_capacity_;
    std::uint32_t plan_capacity_;
    WorldSceneDecodedRestorePlanSnapshot snapshot_;
};
}
