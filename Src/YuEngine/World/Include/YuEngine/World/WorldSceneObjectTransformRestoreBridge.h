// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformRestoreBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreBridgeDesc.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreResult.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreSnapshot.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::object {
class ObjectRegistry;
}

namespace yuengine::world {
class WorldInstance;
class WorldObjectIdentityBridge;
class WorldTransformBridge;

class WorldSceneObjectTransformRestoreBridge final {
public:
    /**
     * @comment Constructs a world scene object-transform restore bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldSceneObjectTransformRestoreBridge(
        WorldSceneObjectTransformRestoreBridgeDesc desc=
            WorldSceneObjectTransformRestoreBridgeDesc{});

    /**
     * @comment Applies caller-owned object identity and transform records after full preflight.
     * @param world Caller-owned world instance.
     * @param object_registry Caller-owned object registry.
     * @param identity_destination Caller-owned identity destination bridge.
     * @param transform_destination Caller-owned transform destination bridge.
     * @param input_identities Caller-owned identity input records.
     * @param input_identity_count Input identity record count.
     * @param input_transforms Caller-owned transform input records.
     * @param input_transform_count Input transform record count.
     * @return Explicit operation result.
     */
    WorldSceneObjectTransformRestoreResult Restore(
        WorldInstance *world,
        yuengine::object::ObjectRegistry *object_registry,
        WorldObjectIdentityBridge *identity_destination,
        WorldTransformBridge *transform_destination,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldSceneObjectTransformRestoreSnapshot Snapshot() const;

private:
    WorldSceneObjectTransformRestoreResult RecordFailure(
        WorldSceneObjectTransformRestoreStatus status);
    WorldSceneObjectTransformRestoreResult RecordFailure(
        WorldSceneObjectTransformRestoreStatus status,
        WorldObjectIdentityStatus identity_status);
    WorldSceneObjectTransformRestoreResult RecordFailure(
        WorldSceneObjectTransformRestoreStatus status,
        WorldTransformStatus transform_status);
    WorldSceneObjectTransformRestoreResult RecordFailure(
        WorldSceneObjectTransformRestoreStatus status,
        yuengine::object::ObjectStatus object_status);
    WorldSceneObjectTransformRestoreResult RecordFailure(
        WorldSceneObjectTransformRestoreStatus status,
        WorldObjectIdentityStatus identity_status,
        WorldTransformStatus transform_status,
        yuengine::object::ObjectStatus object_status);
    WorldSceneObjectTransformRestoreResult RecordRejectedFailure(
        WorldSceneObjectTransformRestoreStatus status);
    WorldSceneObjectTransformRestoreResult RecordRejectedFailure(
        WorldSceneObjectTransformRestoreStatus status,
        yuengine::object::ObjectStatus object_status);
    WorldSceneObjectTransformRestoreResult RecordSuccess(
        const WorldSceneObjectTransformRestoreState &state);
    WorldSceneObjectTransformRestoreStatus ValidateBridgeCapacity() const;
    WorldSceneObjectTransformRestoreStatus ValidateDestinations(
        const WorldObjectIdentityBridge &identity_destination,
        const WorldTransformBridge &transform_destination,
        std::uint32_t input_identity_count,
        std::uint32_t input_transform_count,
        WorldObjectIdentityStatus *out_identity_status,
        WorldTransformStatus *out_transform_status) const;
    WorldSceneObjectTransformRestoreStatus ValidateIdentityRecords(
        const WorldInstance &world,
        const yuengine::object::ObjectRegistry &object_registry,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        yuengine::object::ObjectStatus *out_object_status) const;
    WorldSceneObjectTransformRestoreStatus ValidateIdentityRecord(
        const WorldInstance &world,
        const yuengine::object::ObjectRegistry &object_registry,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        std::uint32_t record_index,
        yuengine::object::ObjectStatus *out_object_status) const;
    WorldSceneObjectTransformRestoreStatus ValidateTransformRecords(
        const WorldInstance &world,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count) const;
    WorldSceneObjectTransformRestoreStatus ValidateTransformRecord(
        const WorldInstance &world,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count,
        std::uint32_t record_index) const;
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
    bool HasIdentityRecord(
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        WorldObjectId world_object_id) const;
    std::uint32_t CountProjectedAcquire(
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        std::uint32_t record_index) const;
    WorldSceneObjectTransformRestoreStatus MapObjectStatus(
        yuengine::object::ObjectStatus object_status) const;
    WorldSceneObjectTransformRestoreStatus MapIdentityStatus(
        WorldObjectIdentityStatus identity_status) const;
    WorldSceneObjectTransformRestoreStatus MapTransformStatus(
        WorldTransformStatus transform_status) const;
    WorldSceneObjectTransformRestoreResult ApplyIdentities(
        WorldObjectIdentityBridge *identity_destination,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        WorldSceneObjectTransformRestoreState *state);
    WorldSceneObjectTransformRestoreResult ApplyTransforms(
        WorldTransformBridge *transform_destination,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count,
        WorldSceneObjectTransformRestoreState *state);

    std::uint32_t identity_capacity_;
    std::uint32_t transform_capacity_;
    WorldSceneObjectTransformRestoreSnapshot snapshot_;
};
}
