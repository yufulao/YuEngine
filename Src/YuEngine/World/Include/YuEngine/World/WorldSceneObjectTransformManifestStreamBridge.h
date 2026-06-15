// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformManifestStreamBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldSceneObjectTransformManifestStreamDesc.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamResult.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamSnapshot.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"

namespace yuengine::serialize {
class SerializeReader;
class SerializeWriter;
}

namespace yuengine::world {
class WorldSceneObjectTransformManifestStreamBridge final {
public:
    /**
     * @comment Constructs a world scene object-transform manifest stream bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldSceneObjectTransformManifestStreamBridge(
        WorldSceneObjectTransformManifestStreamDesc desc=
            WorldSceneObjectTransformManifestStreamDesc{});

    /**
     * @comment Writes caller-owned object-transform records to a deterministic manifest stream.
     * @param writer Caller-owned serialize writer.
     * @param input_identities Caller-owned identity restore records.
     * @param input_identity_count Input identity record count.
     * @param input_transforms Caller-owned transform restore records.
     * @param input_transform_count Input transform record count.
     * @return Explicit operation result.
     */
    WorldSceneObjectTransformManifestStreamResult WriteManifest(
        yuengine::serialize::SerializeWriter *writer,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count);
    /**
     * @comment Reads caller-owned object-transform records from a deterministic manifest stream.
     * @param reader Caller-owned serialize reader.
     * @param output_identities Caller-owned identity output records.
     * @param output_identity_capacity Identity output capacity.
     * @param out_identity_count Output identity record count.
     * @param output_transforms Caller-owned transform output records.
     * @param output_transform_capacity Transform output capacity.
     * @param out_transform_count Output transform record count.
     * @return Explicit operation result.
     */
    WorldSceneObjectTransformManifestStreamResult ReadManifest(
        yuengine::serialize::SerializeReader *reader,
        WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
        std::uint32_t output_identity_capacity,
        std::uint32_t *out_identity_count,
        WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
        std::uint32_t output_transform_capacity,
        std::uint32_t *out_transform_count);
    /**
     * @comment Writes caller-owned object-transform records to a deterministic manifest stream.
     * @param writer Caller-owned serialize writer.
     * @param input_identities Caller-owned identity restore records.
     * @param input_identity_count Input identity record count.
     * @param input_transforms Caller-owned transform restore records.
     * @param input_transform_count Input transform record count.
     * @return Explicit operation result.
     */
    WorldSceneObjectTransformManifestStreamResult WriteSnapshot(
        yuengine::serialize::SerializeWriter *writer,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count);
    /**
     * @comment Reads caller-owned object-transform records from a deterministic manifest stream.
     * @param reader Caller-owned serialize reader.
     * @param output_identities Caller-owned identity output records.
     * @param output_identity_capacity Identity output capacity.
     * @param out_identity_count Output identity record count.
     * @param output_transforms Caller-owned transform output records.
     * @param output_transform_capacity Transform output capacity.
     * @param out_transform_count Output transform record count.
     * @return Explicit operation result.
     */
    WorldSceneObjectTransformManifestStreamResult ReadSnapshot(
        yuengine::serialize::SerializeReader *reader,
        WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
        std::uint32_t output_identity_capacity,
        std::uint32_t *out_identity_count,
        WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
        std::uint32_t output_transform_capacity,
        std::uint32_t *out_transform_count);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldSceneObjectTransformManifestStreamSnapshot Snapshot() const;

private:
    WorldSceneObjectTransformManifestStreamResult RecordFailure(
        WorldSceneObjectTransformManifestStreamStatus status);
    WorldSceneObjectTransformManifestStreamResult RecordRejectedFailure(
        WorldSceneObjectTransformManifestStreamStatus status);
    WorldSceneObjectTransformManifestStreamResult RecordSerializeFailure(
        yuengine::serialize::SerializeStatus status);
    WorldSceneObjectTransformManifestStreamResult RecordWriteSuccess(
        const WorldSceneObjectTransformManifestStreamState &state);
    WorldSceneObjectTransformManifestStreamResult RecordReadSuccess(
        const WorldSceneObjectTransformManifestStreamState &state);
    WorldSceneObjectTransformManifestStreamStatus ValidateBridgeCapacity() const;
    WorldSceneObjectTransformManifestStreamStatus ValidateWriteInputs(
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count) const;
    WorldSceneObjectTransformManifestStreamStatus ValidateReadOutputs(
        WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
        std::uint32_t *out_identity_count,
        WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
        std::uint32_t *out_transform_count) const;
    WorldSceneObjectTransformManifestStreamStatus ValidateIdentityRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_count) const;
    WorldSceneObjectTransformManifestStreamStatus ValidateIdentityRecord(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_index) const;
    WorldSceneObjectTransformManifestStreamStatus ValidateTransformRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
        std::uint32_t identity_record_count,
        const WorldSceneObjectTransformRestoreTransformRecord *transform_records,
        std::uint32_t transform_record_count) const;
    WorldSceneObjectTransformManifestStreamStatus ValidateTransformRecord(
        const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
        std::uint32_t identity_record_count,
        const WorldSceneObjectTransformRestoreTransformRecord *transform_records,
        std::uint32_t record_index) const;
    bool HasDuplicateIdentityWorldObjectId(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateIdentityObjectHandle(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateTransformWorldObjectId(
        const WorldSceneObjectTransformRestoreTransformRecord *records,
        std::uint32_t record_index) const;
    bool HasIdentityRecord(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_count,
        WorldObjectId world_object_id) const;
    void CopyOutputs(
        const WorldSceneObjectTransformRestoreIdentityRecord *decoded_identities,
        std::uint32_t decoded_identity_count,
        WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
        std::uint32_t *out_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *decoded_transforms,
        std::uint32_t decoded_transform_count,
        WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
        std::uint32_t *out_transform_count) const;

    std::uint32_t identity_capacity_;
    std::uint32_t transform_capacity_;
    WorldSceneObjectTransformManifestStreamSnapshot snapshot_;
};
}
