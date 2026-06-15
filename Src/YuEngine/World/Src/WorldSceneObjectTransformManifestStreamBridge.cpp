// Module: YuEngine World
// File: Src/YuEngine/World/Src/WorldSceneObjectTransformManifestStreamBridge.cpp

#include "YuEngine/World/WorldSceneObjectTransformManifestStreamBridge.h"

#include <array>
#include <cstring>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamConstants.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::serialize::SerializeReader;
using yuengine::serialize::SerializeRecordId;
using yuengine::serialize::SerializeSnapshot;
using yuengine::serialize::SerializeStatus;
using yuengine::serialize::SerializeWriter;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity) {
    if (requested_capacity > MAX_WORLD_OBJECT_COUNT) {
        return MAX_WORLD_OBJECT_COUNT;
    }

    return requested_capacity;
}

std::uint32_t CalculateIdentityChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY;
}

std::uint32_t CalculateTransformChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY;
}

std::uint32_t GetIdentityChunkRecordCount(std::uint32_t record_count, std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY) {
        return WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

std::uint32_t GetTransformChunkRecordCount(std::uint32_t record_count, std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY) {
        return WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

SerializeRecordId IdentityChunkRecordId(std::uint32_t chunk_index) {
    return SerializeRecordId{WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_ID_BASE + chunk_index};
}

SerializeRecordId TransformChunkRecordId(std::uint32_t chunk_index) {
    return SerializeRecordId{WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_ID_BASE + chunk_index};
}

void EncodeUInt32(std::uint8_t *bytes, std::uint32_t value) {
    bytes[0U] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

std::uint32_t DecodeUInt32(const std::uint8_t *bytes) {
    const std::uint32_t byte0 = static_cast<std::uint32_t>(bytes[0U]);
    const std::uint32_t byte1 = static_cast<std::uint32_t>(bytes[1U]);
    const std::uint32_t byte2 = static_cast<std::uint32_t>(bytes[2U]);
    const std::uint32_t byte3 = static_cast<std::uint32_t>(bytes[3U]);
    return byte0 | (byte1 << 8U) | (byte2 << 16U) | (byte3 << 24U);
}

std::uint32_t EncodeFloatBits(float value) {
    std::uint32_t result = 0U;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}

float DecodeFloatBits(std::uint32_t bits) {
    float result = 0.0F;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

void EncodeFloat32(std::uint8_t *bytes, float value) {
    const std::uint32_t bits = EncodeFloatBits(value);
    EncodeUInt32(bytes, bits);
}

float DecodeFloat32(const std::uint8_t *bytes) {
    const std::uint32_t bits = DecodeUInt32(bytes);
    return DecodeFloatBits(bits);
}

void EncodeIdentityRecord(
    std::uint8_t *bytes,
    const WorldSceneObjectTransformRestoreIdentityRecord &record) {
    EncodeUInt32(bytes, record.world_object_id.value);
    EncodeUInt32(bytes + 4U, record.object_handle.slot);
    EncodeUInt32(bytes + 8U, record.object_handle.generation);
}

WorldSceneObjectTransformRestoreIdentityRecord DecodeIdentityRecord(const std::uint8_t *bytes) {
    WorldSceneObjectTransformRestoreIdentityRecord record{};
    record.world_object_id.value = DecodeUInt32(bytes);
    record.object_handle.slot = DecodeUInt32(bytes + 4U);
    record.object_handle.generation = DecodeUInt32(bytes + 8U);
    return record;
}

void EncodeTransformRecord(
    std::uint8_t *bytes,
    const WorldSceneObjectTransformRestoreTransformRecord &record) {
    EncodeUInt32(bytes, record.world_object_id.value);
    EncodeFloat32(bytes + 4U, record.transform_state.translation_x);
    EncodeFloat32(bytes + 8U, record.transform_state.translation_y);
    EncodeFloat32(bytes + 12U, record.transform_state.translation_z);
    EncodeFloat32(bytes + 16U, record.transform_state.rotation_x);
    EncodeFloat32(bytes + 20U, record.transform_state.rotation_y);
    EncodeFloat32(bytes + 24U, record.transform_state.rotation_z);
    EncodeFloat32(bytes + 28U, record.transform_state.rotation_w);
    EncodeFloat32(bytes + 32U, record.transform_state.scale_x);
    EncodeFloat32(bytes + 36U, record.transform_state.scale_y);
    EncodeFloat32(bytes + 40U, record.transform_state.scale_z);
}

WorldSceneObjectTransformRestoreTransformRecord DecodeTransformRecord(const std::uint8_t *bytes) {
    WorldSceneObjectTransformRestoreTransformRecord record{};
    record.world_object_id.value = DecodeUInt32(bytes);
    record.transform_state.translation_x = DecodeFloat32(bytes + 4U);
    record.transform_state.translation_y = DecodeFloat32(bytes + 8U);
    record.transform_state.translation_z = DecodeFloat32(bytes + 12U);
    record.transform_state.rotation_x = DecodeFloat32(bytes + 16U);
    record.transform_state.rotation_y = DecodeFloat32(bytes + 20U);
    record.transform_state.rotation_z = DecodeFloat32(bytes + 24U);
    record.transform_state.rotation_w = DecodeFloat32(bytes + 28U);
    record.transform_state.scale_x = DecodeFloat32(bytes + 32U);
    record.transform_state.scale_y = DecodeFloat32(bytes + 36U);
    record.transform_state.scale_z = DecodeFloat32(bytes + 40U);
    return record;
}

std::uint32_t RequiredIdentityChunkPayloadByteCount(std::uint32_t chunk_record_count) {
    return chunk_record_count * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_RECORD_BYTE_COUNT;
}

std::uint32_t RequiredTransformChunkPayloadByteCount(std::uint32_t chunk_record_count) {
    return chunk_record_count * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_RECORD_BYTE_COUNT;
}

std::uint32_t RequiredWriteRecordCount(
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count) {
    return 1U +
        CalculateIdentityChunkCount(identity_record_count) +
        CalculateTransformChunkCount(transform_record_count);
}

std::uint32_t RequiredWriteFieldCount(
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count) {
    return WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_METADATA_FIELD_COUNT +
        CalculateIdentityChunkCount(identity_record_count) +
        CalculateTransformChunkCount(transform_record_count);
}

std::uint32_t RequiredChunkWriteByteCount(std::uint32_t chunk_payload_byte_count) {
    return yuengine::serialize::RECORD_HEADER_BYTE_COUNT +
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIXED_BYTES_FIELD_HEADER_BYTE_COUNT +
        chunk_payload_byte_count;
}

std::uint32_t RequiredWriteByteCount(
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count) {
    std::uint32_t result = WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_METADATA_RECORD_BYTE_COUNT;
    const std::uint32_t identity_chunk_count = CalculateIdentityChunkCount(identity_record_count);
    std::uint32_t identity_chunk_index = 0U;
    while (identity_chunk_index < identity_chunk_count) {
        const std::uint32_t chunk_record_count = GetIdentityChunkRecordCount(
            identity_record_count,
            identity_chunk_index);
        const std::uint32_t payload_byte_count =
            RequiredIdentityChunkPayloadByteCount(chunk_record_count);
        result += RequiredChunkWriteByteCount(payload_byte_count);
        ++identity_chunk_index;
    }

    const std::uint32_t transform_chunk_count = CalculateTransformChunkCount(transform_record_count);
    std::uint32_t transform_chunk_index = 0U;
    while (transform_chunk_index < transform_chunk_count) {
        const std::uint32_t chunk_record_count = GetTransformChunkRecordCount(
            transform_record_count,
            transform_chunk_index);
        const std::uint32_t payload_byte_count =
            RequiredTransformChunkPayloadByteCount(chunk_record_count);
        result += RequiredChunkWriteByteCount(payload_byte_count);
        ++transform_chunk_index;
    }

    return result;
}

bool IsEmptyWriterSnapshot(const SerializeSnapshot &snapshot) {
    if (snapshot.committed_byte_count != 0U) {
        return false;
    }

    if (snapshot.record_count != 0U) {
        return false;
    }

    return snapshot.field_count == 0U;
}

SerializeStatus ValidateOpenedWriterBudget(
    const SerializeWriter &writer,
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count) {
    const SerializeSnapshot writer_snapshot = writer.Snapshot();
    if (writer_snapshot.major_version != yuengine::serialize::STREAM_MAJOR_VERSION) {
        return SerializeStatus::InvalidHeader;
    }

    if (writer_snapshot.minor_version != yuengine::serialize::STREAM_MINOR_VERSION) {
        return SerializeStatus::InvalidHeader;
    }

    if (writer_snapshot.committed_byte_count < yuengine::serialize::STREAM_HEADER_BYTE_COUNT) {
        return SerializeStatus::InvalidHeader;
    }

    if (writer_snapshot.committed_byte_count > yuengine::serialize::MAX_STREAM_BYTE_COUNT) {
        return SerializeStatus::BufferTooSmall;
    }

    const std::uint32_t required_record_count = RequiredWriteRecordCount(
        identity_record_count,
        transform_record_count);
    if (writer_snapshot.record_count > yuengine::serialize::MAX_RECORDS_PER_STREAM) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t remaining_record_count =
        yuengine::serialize::MAX_RECORDS_PER_STREAM - writer_snapshot.record_count;
    if (required_record_count > remaining_record_count) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t required_field_count = RequiredWriteFieldCount(
        identity_record_count,
        transform_record_count);
    if (writer_snapshot.field_count > yuengine::serialize::MAX_FIELDS_PER_STREAM) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t remaining_field_count =
        yuengine::serialize::MAX_FIELDS_PER_STREAM - writer_snapshot.field_count;
    if (required_field_count > remaining_field_count) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t required_byte_count = RequiredWriteByteCount(
        identity_record_count,
        transform_record_count);
    if (required_byte_count > writer.GetRemainingByteCapacity()) {
        return SerializeStatus::BufferTooSmall;
    }

    if (!writer.CanCommitByteCount(required_byte_count)) {
        return SerializeStatus::BufferTooSmall;
    }

    return SerializeStatus::Success;
}

SerializeStatus ValidateEmptyWriterBudget(
    const SerializeWriter &writer,
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count) {
    const std::uint32_t required_byte_count =
        yuengine::serialize::STREAM_HEADER_BYTE_COUNT +
        RequiredWriteByteCount(identity_record_count, transform_record_count);
    if (required_byte_count > writer.GetByteCapacity()) {
        return SerializeStatus::BufferTooSmall;
    }

    if (!writer.CanCommitByteCount(required_byte_count)) {
        return SerializeStatus::BufferTooSmall;
    }

    return SerializeStatus::Success;
}

SerializeStatus ValidateWriteBudget(
    const SerializeWriter &writer,
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count,
    bool *out_begin_stream) {
    *out_begin_stream = false;
    const SerializeSnapshot writer_snapshot = writer.Snapshot();
    if (IsEmptyWriterSnapshot(writer_snapshot)) {
        *out_begin_stream = true;
        return ValidateEmptyWriterBudget(writer, identity_record_count, transform_record_count);
    }

    return ValidateOpenedWriterBudget(writer, identity_record_count, transform_record_count);
}

SerializeStatus WriteMetadataRecord(
    SerializeWriter &writer,
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count) {
    SerializeStatus status = writer.BeginRecord(WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_METADATA_RECORD_ID);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIELD_SCHEMA_VERSION,
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_STREAM_SCHEMA_VERSION);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIELD_IDENTITY_RECORD_COUNT,
        identity_record_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIELD_TRANSFORM_RECORD_COUNT,
        transform_record_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIELD_IDENTITY_CHUNK_COUNT,
        CalculateIdentityChunkCount(identity_record_count));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteUInt32(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIELD_TRANSFORM_CHUNK_COUNT,
        CalculateTransformChunkCount(transform_record_count));
}

SerializeStatus WriteIdentityChunkRecord(
    SerializeWriter &writer,
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t identity_record_count,
    std::uint32_t chunk_index) {
    std::array<std::uint8_t, WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_PAYLOAD_BYTE_COUNT>
        payload{};
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY;
    const std::uint32_t chunk_record_count = GetIdentityChunkRecordCount(
        identity_record_count,
        chunk_index);
    std::uint32_t record_index = 0U;
    while (record_index < chunk_record_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &record =
            records[first_record_index + record_index];
        const std::uint32_t payload_offset =
            record_index * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_RECORD_BYTE_COUNT;
        EncodeIdentityRecord(payload.data() + payload_offset, record);
        ++record_index;
    }

    SerializeStatus status = writer.BeginRecord(IdentityChunkRecordId(chunk_index));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteFixedBytes(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_CHUNK_FIELD_RECORD_BYTES,
        payload.data(),
        RequiredIdentityChunkPayloadByteCount(chunk_record_count));
}

SerializeStatus WriteTransformChunkRecord(
    SerializeWriter &writer,
    const WorldSceneObjectTransformRestoreTransformRecord *records,
    std::uint32_t transform_record_count,
    std::uint32_t chunk_index) {
    std::array<std::uint8_t, WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_PAYLOAD_BYTE_COUNT>
        payload{};
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY;
    const std::uint32_t chunk_record_count = GetTransformChunkRecordCount(
        transform_record_count,
        chunk_index);
    std::uint32_t record_index = 0U;
    while (record_index < chunk_record_count) {
        const WorldSceneObjectTransformRestoreTransformRecord &record =
            records[first_record_index + record_index];
        const std::uint32_t payload_offset =
            record_index * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_RECORD_BYTE_COUNT;
        EncodeTransformRecord(payload.data() + payload_offset, record);
        ++record_index;
    }

    SerializeStatus status = writer.BeginRecord(TransformChunkRecordId(chunk_index));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteFixedBytes(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_CHUNK_FIELD_RECORD_BYTES,
        payload.data(),
        RequiredTransformChunkPayloadByteCount(chunk_record_count));
}
}

WorldSceneObjectTransformManifestStreamBridge::WorldSceneObjectTransformManifestStreamBridge(
    WorldSceneObjectTransformManifestStreamDesc desc)
    : identity_capacity_(ClampCapacity(desc.identity_capacity)),
      transform_capacity_(ClampCapacity(desc.transform_capacity)),
      snapshot_{
          ClampCapacity(desc.identity_capacity),
          ClampCapacity(desc.transform_capacity),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          SerializeStatus::Success,
          WorldSceneObjectTransformManifestStreamStatus::Success} {
    if (desc.identity_capacity == 0U) {
        snapshot_.last_status = WorldSceneObjectTransformManifestStreamStatus::InvalidBridgeCapacity;
        return;
    }

    if (desc.transform_capacity == 0U) {
        snapshot_.last_status = WorldSceneObjectTransformManifestStreamStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldSceneObjectTransformManifestStreamResult WorldSceneObjectTransformManifestStreamBridge::WriteManifest(
    SerializeWriter *writer,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count) {
    const WorldSceneObjectTransformManifestStreamStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldSceneObjectTransformManifestStreamStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (writer == nullptr) {
        return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::InvalidWriter);
    }

    WorldSceneObjectTransformManifestStreamStatus status = ValidateWriteInputs(
        input_identities,
        input_identity_count,
        input_transforms,
        input_transform_count);
    if (status != WorldSceneObjectTransformManifestStreamStatus::Success) {
        return RecordFailure(status);
    }

    status = ValidateIdentityRecords(input_identities, input_identity_count);
    if (status != WorldSceneObjectTransformManifestStreamStatus::Success) {
        return RecordRejectedFailure(status);
    }

    status = ValidateTransformRecords(
        input_identities,
        input_identity_count,
        input_transforms,
        input_transform_count);
    if (status != WorldSceneObjectTransformManifestStreamStatus::Success) {
        return RecordRejectedFailure(status);
    }

    bool begin_stream = false;
    SerializeStatus serialize_status = ValidateWriteBudget(
        *writer,
        input_identity_count,
        input_transform_count,
        &begin_stream);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    if (begin_stream) {
        serialize_status = writer->BeginStream();
        if (serialize_status != SerializeStatus::Success) {
            return RecordSerializeFailure(serialize_status);
        }
    }

    serialize_status = WriteMetadataRecord(*writer, input_identity_count, input_transform_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    const std::uint32_t identity_chunk_count = CalculateIdentityChunkCount(input_identity_count);
    std::uint32_t identity_chunk_index = 0U;
    while (identity_chunk_index < identity_chunk_count) {
        serialize_status = WriteIdentityChunkRecord(
            *writer,
            input_identities,
            input_identity_count,
            identity_chunk_index);
        if (serialize_status != SerializeStatus::Success) {
            return RecordSerializeFailure(serialize_status);
        }

        ++identity_chunk_index;
    }

    const std::uint32_t transform_chunk_count = CalculateTransformChunkCount(input_transform_count);
    std::uint32_t transform_chunk_index = 0U;
    while (transform_chunk_index < transform_chunk_count) {
        serialize_status = WriteTransformChunkRecord(
            *writer,
            input_transforms,
            input_transform_count,
            transform_chunk_index);
        if (serialize_status != SerializeStatus::Success) {
            return RecordSerializeFailure(serialize_status);
        }

        ++transform_chunk_index;
    }

    WorldSceneObjectTransformManifestStreamState state{};
    state.identity_record_count = input_identity_count;
    state.transform_record_count = input_transform_count;
    state.identity_chunk_count = identity_chunk_count;
    state.transform_chunk_count = transform_chunk_count;
    state.committed_byte_count = writer->Snapshot().committed_byte_count;
    return RecordWriteSuccess(state);
}

WorldSceneObjectTransformManifestStreamResult WorldSceneObjectTransformManifestStreamBridge::ReadManifest(
    SerializeReader *reader,
    WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
    std::uint32_t output_identity_capacity,
    std::uint32_t *out_identity_count,
    WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
    std::uint32_t output_transform_capacity,
    std::uint32_t *out_transform_count) {
    const WorldSceneObjectTransformManifestStreamStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldSceneObjectTransformManifestStreamStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (reader == nullptr) {
        return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::InvalidReader);
    }

    WorldSceneObjectTransformManifestStreamStatus output_status = ValidateReadOutputs(
        output_identities,
        out_identity_count,
        output_transforms,
        out_transform_count);
    if (output_status != WorldSceneObjectTransformManifestStreamStatus::Success) {
        return RecordFailure(output_status);
    }

    SerializeStatus serialize_status = reader->OpenStream();
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    std::uint32_t schema_version = 0U;
    serialize_status = reader->ReadUInt32(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_METADATA_RECORD_ID,
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIELD_SCHEMA_VERSION,
        schema_version);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    if (schema_version != WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_STREAM_SCHEMA_VERSION) {
        return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::UnsupportedVersion);
    }

    std::uint32_t identity_record_count = 0U;
    serialize_status = reader->ReadUInt32(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_METADATA_RECORD_ID,
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIELD_IDENTITY_RECORD_COUNT,
        identity_record_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    std::uint32_t transform_record_count = 0U;
    serialize_status = reader->ReadUInt32(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_METADATA_RECORD_ID,
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIELD_TRANSFORM_RECORD_COUNT,
        transform_record_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    std::uint32_t identity_chunk_count = 0U;
    serialize_status = reader->ReadUInt32(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_METADATA_RECORD_ID,
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIELD_IDENTITY_CHUNK_COUNT,
        identity_chunk_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    std::uint32_t transform_chunk_count = 0U;
    serialize_status = reader->ReadUInt32(
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_METADATA_RECORD_ID,
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIELD_TRANSFORM_CHUNK_COUNT,
        transform_chunk_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    if (identity_record_count > identity_capacity_) {
        return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::MalformedRecordCount);
    }

    if (transform_record_count > transform_capacity_) {
        return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::MalformedRecordCount);
    }

    if (identity_record_count > output_identity_capacity) {
        return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::OutputCapacityExceeded);
    }

    if (transform_record_count > output_transform_capacity) {
        return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::OutputCapacityExceeded);
    }

    if (identity_chunk_count != CalculateIdentityChunkCount(identity_record_count)) {
        return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::MalformedRecordCount);
    }

    if (transform_chunk_count != CalculateTransformChunkCount(transform_record_count)) {
        return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::MalformedRecordCount);
    }

    std::array<WorldSceneObjectTransformRestoreIdentityRecord, MAX_WORLD_OBJECT_COUNT> decoded_identities{};
    std::array<std::uint8_t, WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_PAYLOAD_BYTE_COUNT>
        identity_payload{};
    std::uint32_t identity_chunk_index = 0U;
    while (identity_chunk_index < identity_chunk_count) {
        const std::uint32_t chunk_record_count = GetIdentityChunkRecordCount(
            identity_record_count,
            identity_chunk_index);
        const std::uint32_t expected_byte_count =
            RequiredIdentityChunkPayloadByteCount(chunk_record_count);
        std::uint32_t payload_byte_count = 0U;
        serialize_status = reader->ReadFixedBytes(
            IdentityChunkRecordId(identity_chunk_index),
            WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_CHUNK_FIELD_RECORD_BYTES,
            identity_payload.data(),
            WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_PAYLOAD_BYTE_COUNT,
            payload_byte_count);
        if (serialize_status != SerializeStatus::Success) {
            return RecordSerializeFailure(serialize_status);
        }

        if (payload_byte_count != expected_byte_count) {
            return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::MalformedRecordCount);
        }

        std::uint32_t record_index = 0U;
        while (record_index < chunk_record_count) {
            const std::uint32_t payload_offset =
                record_index * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_RECORD_BYTE_COUNT;
            const std::uint32_t output_record_index =
                (identity_chunk_index *
                    WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY) +
                record_index;
            decoded_identities[output_record_index] =
                DecodeIdentityRecord(identity_payload.data() + payload_offset);
            ++record_index;
        }

        ++identity_chunk_index;
    }

    std::array<WorldSceneObjectTransformRestoreTransformRecord, MAX_WORLD_OBJECT_COUNT> decoded_transforms{};
    std::array<std::uint8_t, WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_PAYLOAD_BYTE_COUNT>
        transform_payload{};
    std::uint32_t transform_chunk_index = 0U;
    while (transform_chunk_index < transform_chunk_count) {
        const std::uint32_t chunk_record_count = GetTransformChunkRecordCount(
            transform_record_count,
            transform_chunk_index);
        const std::uint32_t expected_byte_count =
            RequiredTransformChunkPayloadByteCount(chunk_record_count);
        std::uint32_t payload_byte_count = 0U;
        serialize_status = reader->ReadFixedBytes(
            TransformChunkRecordId(transform_chunk_index),
            WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_CHUNK_FIELD_RECORD_BYTES,
            transform_payload.data(),
            WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_PAYLOAD_BYTE_COUNT,
            payload_byte_count);
        if (serialize_status != SerializeStatus::Success) {
            return RecordSerializeFailure(serialize_status);
        }

        if (payload_byte_count != expected_byte_count) {
            return RecordFailure(WorldSceneObjectTransformManifestStreamStatus::MalformedRecordCount);
        }

        std::uint32_t record_index = 0U;
        while (record_index < chunk_record_count) {
            const std::uint32_t payload_offset =
                record_index * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_RECORD_BYTE_COUNT;
            const std::uint32_t output_record_index =
                (transform_chunk_index *
                    WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY) +
                record_index;
            decoded_transforms[output_record_index] =
                DecodeTransformRecord(transform_payload.data() + payload_offset);
            ++record_index;
        }

        ++transform_chunk_index;
    }

    WorldSceneObjectTransformManifestStreamStatus record_status = ValidateIdentityRecords(
        decoded_identities.data(),
        identity_record_count);
    if (record_status != WorldSceneObjectTransformManifestStreamStatus::Success) {
        return RecordRejectedFailure(record_status);
    }

    record_status = ValidateTransformRecords(
        decoded_identities.data(),
        identity_record_count,
        decoded_transforms.data(),
        transform_record_count);
    if (record_status != WorldSceneObjectTransformManifestStreamStatus::Success) {
        return RecordRejectedFailure(record_status);
    }

    CopyOutputs(
        decoded_identities.data(),
        identity_record_count,
        output_identities,
        out_identity_count,
        decoded_transforms.data(),
        transform_record_count,
        output_transforms,
        out_transform_count);

    WorldSceneObjectTransformManifestStreamState state{};
    state.identity_record_count = identity_record_count;
    state.transform_record_count = transform_record_count;
    state.identity_chunk_count = identity_chunk_count;
    state.transform_chunk_count = transform_chunk_count;
    state.committed_byte_count = reader->Snapshot().committed_byte_count;
    return RecordReadSuccess(state);
}

WorldSceneObjectTransformManifestStreamResult WorldSceneObjectTransformManifestStreamBridge::WriteSnapshot(
    SerializeWriter *writer,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count) {
    return WriteManifest(
        writer,
        input_identities,
        input_identity_count,
        input_transforms,
        input_transform_count);
}

WorldSceneObjectTransformManifestStreamResult WorldSceneObjectTransformManifestStreamBridge::ReadSnapshot(
    SerializeReader *reader,
    WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
    std::uint32_t output_identity_capacity,
    std::uint32_t *out_identity_count,
    WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
    std::uint32_t output_transform_capacity,
    std::uint32_t *out_transform_count) {
    return ReadManifest(
        reader,
        output_identities,
        output_identity_capacity,
        out_identity_count,
        output_transforms,
        output_transform_capacity,
        out_transform_count);
}

WorldSceneObjectTransformManifestStreamSnapshot
WorldSceneObjectTransformManifestStreamBridge::Snapshot() const {
    return snapshot_;
}

WorldSceneObjectTransformManifestStreamResult
WorldSceneObjectTransformManifestStreamBridge::RecordFailure(
    WorldSceneObjectTransformManifestStreamStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = status;
    return WorldSceneObjectTransformManifestStreamResult::Failure(status);
}

WorldSceneObjectTransformManifestStreamResult
WorldSceneObjectTransformManifestStreamBridge::RecordRejectedFailure(
    WorldSceneObjectTransformManifestStreamStatus status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status);
}

WorldSceneObjectTransformManifestStreamResult
WorldSceneObjectTransformManifestStreamBridge::RecordSerializeFailure(
    SerializeStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_serialize_status = status;
    snapshot_.last_status = WorldSceneObjectTransformManifestStreamStatus::SerializeFailure;
    return WorldSceneObjectTransformManifestStreamResult::Failure(
        WorldSceneObjectTransformManifestStreamStatus::SerializeFailure,
        status);
}

WorldSceneObjectTransformManifestStreamResult
WorldSceneObjectTransformManifestStreamBridge::RecordWriteSuccess(
    const WorldSceneObjectTransformManifestStreamState &state) {
    ++snapshot_.write_count;
    snapshot_.written_identity_count += state.identity_record_count;
    snapshot_.written_transform_count += state.transform_record_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = WorldSceneObjectTransformManifestStreamStatus::Success;
    return WorldSceneObjectTransformManifestStreamResult::Success(state);
}

WorldSceneObjectTransformManifestStreamResult
WorldSceneObjectTransformManifestStreamBridge::RecordReadSuccess(
    const WorldSceneObjectTransformManifestStreamState &state) {
    ++snapshot_.read_count;
    snapshot_.read_identity_count += state.identity_record_count;
    snapshot_.read_transform_count += state.transform_record_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = WorldSceneObjectTransformManifestStreamStatus::Success;
    return WorldSceneObjectTransformManifestStreamResult::Success(state);
}

WorldSceneObjectTransformManifestStreamStatus
WorldSceneObjectTransformManifestStreamBridge::ValidateBridgeCapacity() const {
    if (identity_capacity_ == 0U) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidBridgeCapacity;
    }

    if (transform_capacity_ == 0U) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidBridgeCapacity;
    }

    return WorldSceneObjectTransformManifestStreamStatus::Success;
}

WorldSceneObjectTransformManifestStreamStatus
WorldSceneObjectTransformManifestStreamBridge::ValidateWriteInputs(
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count) const {
    if (input_identity_count > identity_capacity_) {
        return WorldSceneObjectTransformManifestStreamStatus::InputCountExceeded;
    }

    if (input_transform_count > transform_capacity_) {
        return WorldSceneObjectTransformManifestStreamStatus::InputCountExceeded;
    }

    if (input_identity_count > 0U && input_identities == nullptr) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidIdentityInput;
    }

    if (input_transform_count > 0U && input_transforms == nullptr) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidTransformInput;
    }

    return WorldSceneObjectTransformManifestStreamStatus::Success;
}

WorldSceneObjectTransformManifestStreamStatus
WorldSceneObjectTransformManifestStreamBridge::ValidateReadOutputs(
    WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
    std::uint32_t *out_identity_count,
    WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
    std::uint32_t *out_transform_count) const {
    if (output_identities == nullptr) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidIdentityOutput;
    }

    if (out_identity_count == nullptr) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidIdentityOutputCount;
    }

    if (output_transforms == nullptr) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidTransformOutput;
    }

    if (out_transform_count == nullptr) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidTransformOutputCount;
    }

    return WorldSceneObjectTransformManifestStreamStatus::Success;
}

WorldSceneObjectTransformManifestStreamStatus
WorldSceneObjectTransformManifestStreamBridge::ValidateIdentityRecords(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        const WorldSceneObjectTransformManifestStreamStatus status = ValidateIdentityRecord(
            records,
            record_index);
        if (status != WorldSceneObjectTransformManifestStreamStatus::Success) {
            return status;
        }

        ++record_index;
    }

    return WorldSceneObjectTransformManifestStreamStatus::Success;
}

WorldSceneObjectTransformManifestStreamStatus
WorldSceneObjectTransformManifestStreamBridge::ValidateIdentityRecord(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
    if (!record.world_object_id.IsValid()) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidWorldObjectId;
    }

    if (!record.object_handle.IsValid()) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidObjectHandle;
    }

    if (HasDuplicateIdentityWorldObjectId(records, record_index)) {
        return WorldSceneObjectTransformManifestStreamStatus::DuplicateIdentityWorldObjectId;
    }

    if (HasDuplicateIdentityObjectHandle(records, record_index)) {
        return WorldSceneObjectTransformManifestStreamStatus::DuplicateIdentityObjectHandle;
    }

    return WorldSceneObjectTransformManifestStreamStatus::Success;
}

WorldSceneObjectTransformManifestStreamStatus
WorldSceneObjectTransformManifestStreamBridge::ValidateTransformRecords(
    const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
    std::uint32_t identity_record_count,
    const WorldSceneObjectTransformRestoreTransformRecord *transform_records,
    std::uint32_t transform_record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < transform_record_count) {
        const WorldSceneObjectTransformManifestStreamStatus status = ValidateTransformRecord(
            identity_records,
            identity_record_count,
            transform_records,
            record_index);
        if (status != WorldSceneObjectTransformManifestStreamStatus::Success) {
            return status;
        }

        ++record_index;
    }

    return WorldSceneObjectTransformManifestStreamStatus::Success;
}

WorldSceneObjectTransformManifestStreamStatus
WorldSceneObjectTransformManifestStreamBridge::ValidateTransformRecord(
    const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
    std::uint32_t identity_record_count,
    const WorldSceneObjectTransformRestoreTransformRecord *transform_records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreTransformRecord &record = transform_records[record_index];
    if (!record.world_object_id.IsValid()) {
        return WorldSceneObjectTransformManifestStreamStatus::InvalidWorldObjectId;
    }

    if (HasDuplicateTransformWorldObjectId(transform_records, record_index)) {
        return WorldSceneObjectTransformManifestStreamStatus::DuplicateTransformWorldObjectId;
    }

    if (!HasIdentityRecord(identity_records, identity_record_count, record.world_object_id)) {
        return WorldSceneObjectTransformManifestStreamStatus::MissingIdentityForTransform;
    }

    return WorldSceneObjectTransformManifestStreamStatus::Success;
}

bool WorldSceneObjectTransformManifestStreamBridge::HasDuplicateIdentityWorldObjectId(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneObjectTransformRestoreIdentityRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value == record.world_object_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneObjectTransformManifestStreamBridge::HasDuplicateIdentityObjectHandle(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneObjectTransformRestoreIdentityRecord &compare_record = records[compare_index];
        if (compare_record.object_handle.slot != record.object_handle.slot) {
            ++compare_index;
            continue;
        }

        if (compare_record.object_handle.generation == record.object_handle.generation) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneObjectTransformManifestStreamBridge::HasDuplicateTransformWorldObjectId(
    const WorldSceneObjectTransformRestoreTransformRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreTransformRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneObjectTransformRestoreTransformRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value == record.world_object_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneObjectTransformManifestStreamBridge::HasIdentityRecord(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_count,
    WorldObjectId world_object_id) const {
    std::uint32_t identity_index = 0U;
    while (identity_index < record_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &identity = records[identity_index];
        if (identity.world_object_id.value == world_object_id.value) {
            return true;
        }

        ++identity_index;
    }

    return false;
}

void WorldSceneObjectTransformManifestStreamBridge::CopyOutputs(
    const WorldSceneObjectTransformRestoreIdentityRecord *decoded_identities,
    std::uint32_t decoded_identity_count,
    WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
    std::uint32_t *out_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *decoded_transforms,
    std::uint32_t decoded_transform_count,
    WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
    std::uint32_t *out_transform_count) const {
    std::uint32_t identity_index = 0U;
    while (identity_index < decoded_identity_count) {
        output_identities[identity_index] = decoded_identities[identity_index];
        ++identity_index;
    }

    std::uint32_t transform_index = 0U;
    while (transform_index < decoded_transform_count) {
        output_transforms[transform_index] = decoded_transforms[transform_index];
        ++transform_index;
    }

    *out_identity_count = decoded_identity_count;
    *out_transform_count = decoded_transform_count;
}
}
