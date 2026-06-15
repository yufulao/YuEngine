// Module: YuEngine World
// File: Src/YuEngine/World/Src/WorldSceneAssemblyManifestStreamBridge.cpp

#include "YuEngine/World/WorldSceneAssemblyManifestStreamBridge.h"

#include <array>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamConstants.h"

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

std::uint32_t CalculateAttachmentChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY;
}

std::uint32_t CalculateBindingChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY;
}

std::uint32_t GetAttachmentChunkRecordCount(std::uint32_t record_count, std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY) {
        return WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

std::uint32_t GetBindingChunkRecordCount(std::uint32_t record_count, std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY) {
        return WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

SerializeRecordId AttachmentChunkRecordId(std::uint32_t chunk_index) {
    return SerializeRecordId{WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_ID_BASE + chunk_index};
}

SerializeRecordId BindingChunkRecordId(std::uint32_t chunk_index) {
    return SerializeRecordId{WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_ID_BASE + chunk_index};
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

void EncodeAttachmentRecord(
    std::uint8_t *bytes,
    const WorldComponentAttachmentSnapshotRecord &record) {
    EncodeUInt32(bytes, record.world_object_id.value);
    EncodeUInt32(bytes + 4U, record.component_type_id.value);
    EncodeUInt32(bytes + 8U, record.component_slot_id.value);
}

WorldComponentAttachmentSnapshotRecord DecodeAttachmentRecord(const std::uint8_t *bytes) {
    WorldComponentAttachmentSnapshotRecord record{};
    record.world_object_id.value = DecodeUInt32(bytes);
    record.component_type_id.value = DecodeUInt32(bytes + 4U);
    record.component_slot_id.value = DecodeUInt32(bytes + 8U);
    return record;
}

void EncodeBindingRecord(
    std::uint8_t *bytes,
    const WorldComponentResourceBindingSnapshotRecord &record) {
    EncodeUInt32(bytes, record.world_object_id.value);
    EncodeUInt32(bytes + 4U, record.component_type_id.value);
    EncodeUInt32(bytes + 8U, record.component_slot_id.value);
    EncodeUInt32(bytes + 12U, record.resource_handle.slot);
    EncodeUInt32(bytes + 16U, record.resource_handle.generation);
    EncodeUInt32(bytes + 20U, record.expected_resource_type.value);
}

WorldComponentResourceBindingSnapshotRecord DecodeBindingRecord(const std::uint8_t *bytes) {
    WorldComponentResourceBindingSnapshotRecord record{};
    record.world_object_id.value = DecodeUInt32(bytes);
    record.component_type_id.value = DecodeUInt32(bytes + 4U);
    record.component_slot_id.value = DecodeUInt32(bytes + 8U);
    record.resource_handle.slot = DecodeUInt32(bytes + 12U);
    record.resource_handle.generation = DecodeUInt32(bytes + 16U);
    record.expected_resource_type.value = DecodeUInt32(bytes + 20U);
    return record;
}

std::uint32_t RequiredAttachmentChunkPayloadByteCount(std::uint32_t chunk_record_count) {
    return chunk_record_count * WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_RECORD_BYTE_COUNT;
}

std::uint32_t RequiredBindingChunkPayloadByteCount(std::uint32_t chunk_record_count) {
    return chunk_record_count * WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_RECORD_BYTE_COUNT;
}

std::uint32_t RequiredWriteRecordCount(
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    return 1U +
        CalculateAttachmentChunkCount(attachment_record_count) +
        CalculateBindingChunkCount(binding_record_count);
}

std::uint32_t RequiredWriteFieldCount(
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    return WORLD_SCENE_ASSEMBLY_MANIFEST_METADATA_FIELD_COUNT +
        CalculateAttachmentChunkCount(attachment_record_count) +
        CalculateBindingChunkCount(binding_record_count);
}

std::uint32_t RequiredChunkWriteByteCount(std::uint32_t chunk_payload_byte_count) {
    return yuengine::serialize::RECORD_HEADER_BYTE_COUNT +
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIXED_BYTES_FIELD_HEADER_BYTE_COUNT +
        chunk_payload_byte_count;
}

std::uint32_t RequiredWriteByteCount(
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    std::uint32_t result = WORLD_SCENE_ASSEMBLY_MANIFEST_METADATA_RECORD_BYTE_COUNT;
    const std::uint32_t attachment_chunk_count =
        CalculateAttachmentChunkCount(attachment_record_count);
    std::uint32_t attachment_chunk_index = 0U;
    while (attachment_chunk_index < attachment_chunk_count) {
        const std::uint32_t chunk_record_count = GetAttachmentChunkRecordCount(
            attachment_record_count,
            attachment_chunk_index);
        const std::uint32_t payload_byte_count =
            RequiredAttachmentChunkPayloadByteCount(chunk_record_count);
        result += RequiredChunkWriteByteCount(payload_byte_count);
        ++attachment_chunk_index;
    }

    const std::uint32_t binding_chunk_count = CalculateBindingChunkCount(binding_record_count);
    std::uint32_t binding_chunk_index = 0U;
    while (binding_chunk_index < binding_chunk_count) {
        const std::uint32_t chunk_record_count = GetBindingChunkRecordCount(
            binding_record_count,
            binding_chunk_index);
        const std::uint32_t payload_byte_count = RequiredBindingChunkPayloadByteCount(chunk_record_count);
        result += RequiredChunkWriteByteCount(payload_byte_count);
        ++binding_chunk_index;
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
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
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
        attachment_record_count,
        binding_record_count);
    if (writer_snapshot.record_count > yuengine::serialize::MAX_RECORDS_PER_STREAM) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t remaining_record_count =
        yuengine::serialize::MAX_RECORDS_PER_STREAM - writer_snapshot.record_count;
    if (required_record_count > remaining_record_count) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t required_field_count = RequiredWriteFieldCount(
        attachment_record_count,
        binding_record_count);
    if (writer_snapshot.field_count > yuengine::serialize::MAX_FIELDS_PER_STREAM) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t remaining_field_count =
        yuengine::serialize::MAX_FIELDS_PER_STREAM - writer_snapshot.field_count;
    if (required_field_count > remaining_field_count) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t required_byte_count = RequiredWriteByteCount(
        attachment_record_count,
        binding_record_count);
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
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    const std::uint32_t required_byte_count =
        yuengine::serialize::STREAM_HEADER_BYTE_COUNT +
        RequiredWriteByteCount(attachment_record_count, binding_record_count);
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
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count,
    bool *out_begin_stream) {
    *out_begin_stream = false;
    const SerializeSnapshot writer_snapshot = writer.Snapshot();
    if (IsEmptyWriterSnapshot(writer_snapshot)) {
        *out_begin_stream = true;
        return ValidateEmptyWriterBudget(writer, attachment_record_count, binding_record_count);
    }

    return ValidateOpenedWriterBudget(writer, attachment_record_count, binding_record_count);
}

SerializeStatus WriteMetadataRecord(
    SerializeWriter &writer,
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    SerializeStatus status = writer.BeginRecord(WORLD_SCENE_ASSEMBLY_MANIFEST_METADATA_RECORD_ID);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIELD_SCHEMA_VERSION,
        WORLD_SCENE_ASSEMBLY_MANIFEST_STREAM_SCHEMA_VERSION);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIELD_ATTACHMENT_RECORD_COUNT,
        attachment_record_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIELD_BINDING_RECORD_COUNT,
        binding_record_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIELD_ATTACHMENT_CHUNK_COUNT,
        CalculateAttachmentChunkCount(attachment_record_count));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteUInt32(
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIELD_BINDING_CHUNK_COUNT,
        CalculateBindingChunkCount(binding_record_count));
}

SerializeStatus WriteAttachmentChunkRecord(
    SerializeWriter &writer,
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t attachment_record_count,
    std::uint32_t chunk_index) {
    std::array<std::uint8_t, WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_PAYLOAD_BYTE_COUNT> payload{};
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY;
    const std::uint32_t chunk_record_count = GetAttachmentChunkRecordCount(
        attachment_record_count,
        chunk_index);
    std::uint32_t record_index = 0U;
    while (record_index < chunk_record_count) {
        const WorldComponentAttachmentSnapshotRecord &record =
            records[first_record_index + record_index];
        const std::uint32_t payload_offset =
            record_index * WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_RECORD_BYTE_COUNT;
        EncodeAttachmentRecord(payload.data() + payload_offset, record);
        ++record_index;
    }

    SerializeStatus status = writer.BeginRecord(AttachmentChunkRecordId(chunk_index));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteFixedBytes(
        WORLD_SCENE_ASSEMBLY_MANIFEST_CHUNK_FIELD_RECORD_BYTES,
        payload.data(),
        RequiredAttachmentChunkPayloadByteCount(chunk_record_count));
}

SerializeStatus WriteBindingChunkRecord(
    SerializeWriter &writer,
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t binding_record_count,
    std::uint32_t chunk_index) {
    std::array<std::uint8_t, WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_PAYLOAD_BYTE_COUNT> payload{};
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY;
    const std::uint32_t chunk_record_count = GetBindingChunkRecordCount(
        binding_record_count,
        chunk_index);
    std::uint32_t record_index = 0U;
    while (record_index < chunk_record_count) {
        const WorldComponentResourceBindingSnapshotRecord &record =
            records[first_record_index + record_index];
        const std::uint32_t payload_offset =
            record_index * WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_RECORD_BYTE_COUNT;
        EncodeBindingRecord(payload.data() + payload_offset, record);
        ++record_index;
    }

    SerializeStatus status = writer.BeginRecord(BindingChunkRecordId(chunk_index));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteFixedBytes(
        WORLD_SCENE_ASSEMBLY_MANIFEST_CHUNK_FIELD_RECORD_BYTES,
        payload.data(),
        RequiredBindingChunkPayloadByteCount(chunk_record_count));
}
}

WorldSceneAssemblyManifestStreamBridge::WorldSceneAssemblyManifestStreamBridge(
    WorldSceneAssemblyManifestStreamDesc desc)
    : attachment_capacity_(ClampCapacity(desc.attachment_capacity)),
      binding_capacity_(ClampCapacity(desc.binding_capacity)),
      snapshot_{
          ClampCapacity(desc.attachment_capacity),
          ClampCapacity(desc.binding_capacity),
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
          WorldSceneAssemblyManifestStreamStatus::Success} {
    if (desc.attachment_capacity == 0U) {
        snapshot_.last_status = WorldSceneAssemblyManifestStreamStatus::InvalidBridgeCapacity;
        return;
    }

    if (desc.binding_capacity == 0U) {
        snapshot_.last_status = WorldSceneAssemblyManifestStreamStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldSceneAssemblyManifestStreamResult WorldSceneAssemblyManifestStreamBridge::WriteManifest(
    SerializeWriter *writer,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count) {
    const WorldSceneAssemblyManifestStreamStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldSceneAssemblyManifestStreamStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (writer == nullptr) {
        return RecordFailure(WorldSceneAssemblyManifestStreamStatus::InvalidWriter);
    }

    WorldSceneAssemblyManifestStreamStatus status = ValidateWriteInputs(
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count);
    if (status != WorldSceneAssemblyManifestStreamStatus::Success) {
        return RecordFailure(status);
    }

    status = ValidateAttachmentRecords(input_attachments, input_attachment_count);
    if (status != WorldSceneAssemblyManifestStreamStatus::Success) {
        return RecordRejectedFailure(status);
    }

    status = ValidateBindingRecords(
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count);
    if (status != WorldSceneAssemblyManifestStreamStatus::Success) {
        return RecordRejectedFailure(status);
    }

    bool begin_stream = false;
    SerializeStatus serialize_status = ValidateWriteBudget(
        *writer,
        input_attachment_count,
        input_binding_count,
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

    serialize_status = WriteMetadataRecord(*writer, input_attachment_count, input_binding_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    const std::uint32_t attachment_chunk_count = CalculateAttachmentChunkCount(input_attachment_count);
    std::uint32_t attachment_chunk_index = 0U;
    while (attachment_chunk_index < attachment_chunk_count) {
        serialize_status = WriteAttachmentChunkRecord(
            *writer,
            input_attachments,
            input_attachment_count,
            attachment_chunk_index);
        if (serialize_status != SerializeStatus::Success) {
            return RecordSerializeFailure(serialize_status);
        }

        ++attachment_chunk_index;
    }

    const std::uint32_t binding_chunk_count = CalculateBindingChunkCount(input_binding_count);
    std::uint32_t binding_chunk_index = 0U;
    while (binding_chunk_index < binding_chunk_count) {
        serialize_status = WriteBindingChunkRecord(
            *writer,
            input_bindings,
            input_binding_count,
            binding_chunk_index);
        if (serialize_status != SerializeStatus::Success) {
            return RecordSerializeFailure(serialize_status);
        }

        ++binding_chunk_index;
    }

    WorldSceneAssemblyManifestStreamState state{};
    state.attachment_record_count = input_attachment_count;
    state.binding_record_count = input_binding_count;
    state.attachment_chunk_count = attachment_chunk_count;
    state.binding_chunk_count = binding_chunk_count;
    state.committed_byte_count = writer->Snapshot().committed_byte_count;
    return RecordWriteSuccess(state);
}

WorldSceneAssemblyManifestStreamResult WorldSceneAssemblyManifestStreamBridge::ReadManifest(
    SerializeReader *reader,
    WorldComponentAttachmentSnapshotRecord *output_attachments,
    std::uint32_t output_attachment_capacity,
    std::uint32_t *out_attachment_count,
    WorldComponentResourceBindingSnapshotRecord *output_bindings,
    std::uint32_t output_binding_capacity,
    std::uint32_t *out_binding_count) {
    const WorldSceneAssemblyManifestStreamStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldSceneAssemblyManifestStreamStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (reader == nullptr) {
        return RecordFailure(WorldSceneAssemblyManifestStreamStatus::InvalidReader);
    }

    WorldSceneAssemblyManifestStreamStatus output_status = ValidateReadOutputs(
        output_attachments,
        out_attachment_count,
        output_bindings,
        out_binding_count);
    if (output_status != WorldSceneAssemblyManifestStreamStatus::Success) {
        return RecordFailure(output_status);
    }

    SerializeStatus serialize_status = reader->OpenStream();
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    std::uint32_t schema_version = 0U;
    serialize_status = reader->ReadUInt32(
        WORLD_SCENE_ASSEMBLY_MANIFEST_METADATA_RECORD_ID,
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIELD_SCHEMA_VERSION,
        schema_version);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    if (schema_version != WORLD_SCENE_ASSEMBLY_MANIFEST_STREAM_SCHEMA_VERSION) {
        return RecordFailure(WorldSceneAssemblyManifestStreamStatus::UnsupportedVersion);
    }

    std::uint32_t attachment_record_count = 0U;
    serialize_status = reader->ReadUInt32(
        WORLD_SCENE_ASSEMBLY_MANIFEST_METADATA_RECORD_ID,
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIELD_ATTACHMENT_RECORD_COUNT,
        attachment_record_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    std::uint32_t binding_record_count = 0U;
    serialize_status = reader->ReadUInt32(
        WORLD_SCENE_ASSEMBLY_MANIFEST_METADATA_RECORD_ID,
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIELD_BINDING_RECORD_COUNT,
        binding_record_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    std::uint32_t attachment_chunk_count = 0U;
    serialize_status = reader->ReadUInt32(
        WORLD_SCENE_ASSEMBLY_MANIFEST_METADATA_RECORD_ID,
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIELD_ATTACHMENT_CHUNK_COUNT,
        attachment_chunk_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    std::uint32_t binding_chunk_count = 0U;
    serialize_status = reader->ReadUInt32(
        WORLD_SCENE_ASSEMBLY_MANIFEST_METADATA_RECORD_ID,
        WORLD_SCENE_ASSEMBLY_MANIFEST_FIELD_BINDING_CHUNK_COUNT,
        binding_chunk_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    if (attachment_record_count > attachment_capacity_) {
        return RecordFailure(WorldSceneAssemblyManifestStreamStatus::MalformedRecordCount);
    }

    if (binding_record_count > binding_capacity_) {
        return RecordFailure(WorldSceneAssemblyManifestStreamStatus::MalformedRecordCount);
    }

    if (attachment_record_count > output_attachment_capacity) {
        return RecordFailure(WorldSceneAssemblyManifestStreamStatus::OutputCapacityExceeded);
    }

    if (binding_record_count > output_binding_capacity) {
        return RecordFailure(WorldSceneAssemblyManifestStreamStatus::OutputCapacityExceeded);
    }

    if (attachment_chunk_count != CalculateAttachmentChunkCount(attachment_record_count)) {
        return RecordFailure(WorldSceneAssemblyManifestStreamStatus::MalformedRecordCount);
    }

    if (binding_chunk_count != CalculateBindingChunkCount(binding_record_count)) {
        return RecordFailure(WorldSceneAssemblyManifestStreamStatus::MalformedRecordCount);
    }

    std::array<WorldComponentAttachmentSnapshotRecord, MAX_WORLD_OBJECT_COUNT> decoded_attachments{};
    std::array<std::uint8_t, WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_PAYLOAD_BYTE_COUNT>
        attachment_payload{};
    std::uint32_t attachment_chunk_index = 0U;
    while (attachment_chunk_index < attachment_chunk_count) {
        const std::uint32_t chunk_record_count = GetAttachmentChunkRecordCount(
            attachment_record_count,
            attachment_chunk_index);
        const std::uint32_t expected_byte_count =
            RequiredAttachmentChunkPayloadByteCount(chunk_record_count);
        std::uint32_t payload_byte_count = 0U;
        serialize_status = reader->ReadFixedBytes(
            AttachmentChunkRecordId(attachment_chunk_index),
            WORLD_SCENE_ASSEMBLY_MANIFEST_CHUNK_FIELD_RECORD_BYTES,
            attachment_payload.data(),
            WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_PAYLOAD_BYTE_COUNT,
            payload_byte_count);
        if (serialize_status != SerializeStatus::Success) {
            return RecordSerializeFailure(serialize_status);
        }

        if (payload_byte_count != expected_byte_count) {
            return RecordFailure(WorldSceneAssemblyManifestStreamStatus::MalformedRecordCount);
        }

        std::uint32_t record_index = 0U;
        while (record_index < chunk_record_count) {
            const std::uint32_t payload_offset =
                record_index * WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_RECORD_BYTE_COUNT;
            const std::uint32_t output_record_index =
                (attachment_chunk_index *
                    WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY) +
                record_index;
            decoded_attachments[output_record_index] =
                DecodeAttachmentRecord(attachment_payload.data() + payload_offset);
            ++record_index;
        }

        ++attachment_chunk_index;
    }

    std::array<WorldComponentResourceBindingSnapshotRecord, MAX_WORLD_OBJECT_COUNT> decoded_bindings{};
    std::array<std::uint8_t, WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_PAYLOAD_BYTE_COUNT>
        binding_payload{};
    std::uint32_t binding_chunk_index = 0U;
    while (binding_chunk_index < binding_chunk_count) {
        const std::uint32_t chunk_record_count = GetBindingChunkRecordCount(
            binding_record_count,
            binding_chunk_index);
        const std::uint32_t expected_byte_count =
            RequiredBindingChunkPayloadByteCount(chunk_record_count);
        std::uint32_t payload_byte_count = 0U;
        serialize_status = reader->ReadFixedBytes(
            BindingChunkRecordId(binding_chunk_index),
            WORLD_SCENE_ASSEMBLY_MANIFEST_CHUNK_FIELD_RECORD_BYTES,
            binding_payload.data(),
            WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_PAYLOAD_BYTE_COUNT,
            payload_byte_count);
        if (serialize_status != SerializeStatus::Success) {
            return RecordSerializeFailure(serialize_status);
        }

        if (payload_byte_count != expected_byte_count) {
            return RecordFailure(WorldSceneAssemblyManifestStreamStatus::MalformedRecordCount);
        }

        std::uint32_t record_index = 0U;
        while (record_index < chunk_record_count) {
            const std::uint32_t payload_offset =
                record_index * WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_RECORD_BYTE_COUNT;
            const std::uint32_t output_record_index =
                (binding_chunk_index *
                    WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY) +
                record_index;
            decoded_bindings[output_record_index] =
                DecodeBindingRecord(binding_payload.data() + payload_offset);
            ++record_index;
        }

        ++binding_chunk_index;
    }

    WorldSceneAssemblyManifestStreamStatus record_status = ValidateAttachmentRecords(
        decoded_attachments.data(),
        attachment_record_count);
    if (record_status != WorldSceneAssemblyManifestStreamStatus::Success) {
        return RecordRejectedFailure(record_status);
    }

    record_status = ValidateBindingRecords(
        decoded_attachments.data(),
        attachment_record_count,
        decoded_bindings.data(),
        binding_record_count);
    if (record_status != WorldSceneAssemblyManifestStreamStatus::Success) {
        return RecordRejectedFailure(record_status);
    }

    CopyOutputs(
        decoded_attachments.data(),
        attachment_record_count,
        output_attachments,
        out_attachment_count,
        decoded_bindings.data(),
        binding_record_count,
        output_bindings,
        out_binding_count);

    WorldSceneAssemblyManifestStreamState state{};
    state.attachment_record_count = attachment_record_count;
    state.binding_record_count = binding_record_count;
    state.attachment_chunk_count = attachment_chunk_count;
    state.binding_chunk_count = binding_chunk_count;
    state.committed_byte_count = reader->Snapshot().committed_byte_count;
    return RecordReadSuccess(state);
}

WorldSceneAssemblyManifestStreamResult WorldSceneAssemblyManifestStreamBridge::WriteSnapshot(
    SerializeWriter *writer,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count) {
    return WriteManifest(
        writer,
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count);
}

WorldSceneAssemblyManifestStreamResult WorldSceneAssemblyManifestStreamBridge::ReadSnapshot(
    SerializeReader *reader,
    WorldComponentAttachmentSnapshotRecord *output_attachments,
    std::uint32_t output_attachment_capacity,
    std::uint32_t *out_attachment_count,
    WorldComponentResourceBindingSnapshotRecord *output_bindings,
    std::uint32_t output_binding_capacity,
    std::uint32_t *out_binding_count) {
    return ReadManifest(
        reader,
        output_attachments,
        output_attachment_capacity,
        out_attachment_count,
        output_bindings,
        output_binding_capacity,
        out_binding_count);
}

WorldSceneAssemblyManifestStreamSnapshot WorldSceneAssemblyManifestStreamBridge::Snapshot() const {
    return snapshot_;
}

WorldSceneAssemblyManifestStreamResult WorldSceneAssemblyManifestStreamBridge::RecordFailure(
    WorldSceneAssemblyManifestStreamStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = status;
    return WorldSceneAssemblyManifestStreamResult::Failure(status);
}

WorldSceneAssemblyManifestStreamResult WorldSceneAssemblyManifestStreamBridge::RecordRejectedFailure(
    WorldSceneAssemblyManifestStreamStatus status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status);
}

WorldSceneAssemblyManifestStreamResult WorldSceneAssemblyManifestStreamBridge::RecordSerializeFailure(
    SerializeStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_serialize_status = status;
    snapshot_.last_status = WorldSceneAssemblyManifestStreamStatus::SerializeFailure;
    return WorldSceneAssemblyManifestStreamResult::Failure(
        WorldSceneAssemblyManifestStreamStatus::SerializeFailure,
        status);
}

WorldSceneAssemblyManifestStreamResult WorldSceneAssemblyManifestStreamBridge::RecordWriteSuccess(
    const WorldSceneAssemblyManifestStreamState &state) {
    ++snapshot_.write_count;
    snapshot_.written_attachment_count += state.attachment_record_count;
    snapshot_.written_binding_count += state.binding_record_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = WorldSceneAssemblyManifestStreamStatus::Success;
    return WorldSceneAssemblyManifestStreamResult::Success(state);
}

WorldSceneAssemblyManifestStreamResult WorldSceneAssemblyManifestStreamBridge::RecordReadSuccess(
    const WorldSceneAssemblyManifestStreamState &state) {
    ++snapshot_.read_count;
    snapshot_.read_attachment_count += state.attachment_record_count;
    snapshot_.read_binding_count += state.binding_record_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = WorldSceneAssemblyManifestStreamStatus::Success;
    return WorldSceneAssemblyManifestStreamResult::Success(state);
}

WorldSceneAssemblyManifestStreamStatus WorldSceneAssemblyManifestStreamBridge::ValidateBridgeCapacity() const {
    if (attachment_capacity_ == 0U) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidBridgeCapacity;
    }

    if (binding_capacity_ == 0U) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidBridgeCapacity;
    }

    return WorldSceneAssemblyManifestStreamStatus::Success;
}

WorldSceneAssemblyManifestStreamStatus WorldSceneAssemblyManifestStreamBridge::ValidateWriteInputs(
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count) const {
    if (input_attachment_count > attachment_capacity_) {
        return WorldSceneAssemblyManifestStreamStatus::InputCountExceeded;
    }

    if (input_binding_count > binding_capacity_) {
        return WorldSceneAssemblyManifestStreamStatus::InputCountExceeded;
    }

    if (input_attachment_count > 0U && input_attachments == nullptr) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidAttachmentInput;
    }

    if (input_binding_count > 0U && input_bindings == nullptr) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidBindingInput;
    }

    return WorldSceneAssemblyManifestStreamStatus::Success;
}

WorldSceneAssemblyManifestStreamStatus WorldSceneAssemblyManifestStreamBridge::ValidateReadOutputs(
    WorldComponentAttachmentSnapshotRecord *output_attachments,
    std::uint32_t *out_attachment_count,
    WorldComponentResourceBindingSnapshotRecord *output_bindings,
    std::uint32_t *out_binding_count) const {
    if (output_attachments == nullptr) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidAttachmentOutput;
    }

    if (out_attachment_count == nullptr) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidAttachmentOutputCount;
    }

    if (output_bindings == nullptr) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidBindingOutput;
    }

    if (out_binding_count == nullptr) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidBindingOutputCount;
    }

    return WorldSceneAssemblyManifestStreamStatus::Success;
}

WorldSceneAssemblyManifestStreamStatus WorldSceneAssemblyManifestStreamBridge::ValidateAttachmentRecords(
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        const WorldSceneAssemblyManifestStreamStatus status = ValidateAttachmentRecord(
            records,
            record_index);
        if (status != WorldSceneAssemblyManifestStreamStatus::Success) {
            return status;
        }

        ++record_index;
    }

    return WorldSceneAssemblyManifestStreamStatus::Success;
}

WorldSceneAssemblyManifestStreamStatus WorldSceneAssemblyManifestStreamBridge::ValidateAttachmentRecord(
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t record_index) const {
    const WorldComponentAttachmentSnapshotRecord &record = records[record_index];
    if (!record.world_object_id.IsValid()) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidWorldObjectId;
    }

    if (!record.component_type_id.IsValid()) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidComponentTypeId;
    }

    if (!record.component_slot_id.IsValid()) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidComponentSlotId;
    }

    if (HasDuplicateAttachment(records, record_index)) {
        return WorldSceneAssemblyManifestStreamStatus::DuplicateAttachment;
    }

    return WorldSceneAssemblyManifestStreamStatus::Success;
}

WorldSceneAssemblyManifestStreamStatus WorldSceneAssemblyManifestStreamBridge::ValidateBindingRecords(
    const WorldComponentAttachmentSnapshotRecord *attachment_records,
    std::uint32_t attachment_record_count,
    const WorldComponentResourceBindingSnapshotRecord *binding_records,
    std::uint32_t binding_record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < binding_record_count) {
        const WorldSceneAssemblyManifestStreamStatus status = ValidateBindingRecord(
            attachment_records,
            attachment_record_count,
            binding_records,
            record_index);
        if (status != WorldSceneAssemblyManifestStreamStatus::Success) {
            return status;
        }

        ++record_index;
    }

    return WorldSceneAssemblyManifestStreamStatus::Success;
}

WorldSceneAssemblyManifestStreamStatus WorldSceneAssemblyManifestStreamBridge::ValidateBindingRecord(
    const WorldComponentAttachmentSnapshotRecord *attachment_records,
    std::uint32_t attachment_record_count,
    const WorldComponentResourceBindingSnapshotRecord *binding_records,
    std::uint32_t record_index) const {
    const WorldComponentResourceBindingSnapshotRecord &binding = binding_records[record_index];
    if (!binding.world_object_id.IsValid()) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidWorldObjectId;
    }

    if (!binding.component_type_id.IsValid()) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidComponentTypeId;
    }

    if (!binding.component_slot_id.IsValid()) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidComponentSlotId;
    }

    if (!binding.resource_handle.IsValid()) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidResourceHandle;
    }

    if (!binding.expected_resource_type.IsValid()) {
        return WorldSceneAssemblyManifestStreamStatus::InvalidResourceTypeId;
    }

    if (HasDuplicateBinding(binding_records, record_index)) {
        return WorldSceneAssemblyManifestStreamStatus::DuplicateBinding;
    }

    if (!HasAttachmentTuple(attachment_records, attachment_record_count, binding)) {
        return WorldSceneAssemblyManifestStreamStatus::MissingAttachment;
    }

    return WorldSceneAssemblyManifestStreamStatus::Success;
}

bool WorldSceneAssemblyManifestStreamBridge::HasDuplicateAttachment(
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t record_index) const {
    const WorldComponentAttachmentSnapshotRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldComponentAttachmentSnapshotRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value != record.world_object_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_record.component_type_id.value == record.component_type_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAssemblyManifestStreamBridge::HasDuplicateBinding(
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t record_index) const {
    const WorldComponentResourceBindingSnapshotRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldComponentResourceBindingSnapshotRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value != record.world_object_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_record.component_type_id.value != record.component_type_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_record.component_slot_id.value == record.component_slot_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneAssemblyManifestStreamBridge::HasAttachmentTuple(
    const WorldComponentAttachmentSnapshotRecord *attachment_records,
    std::uint32_t attachment_record_count,
    const WorldComponentResourceBindingSnapshotRecord &binding_record) const {
    std::uint32_t attachment_index = 0U;
    while (attachment_index < attachment_record_count) {
        const WorldComponentAttachmentSnapshotRecord &attachment = attachment_records[attachment_index];
        if (attachment.world_object_id.value != binding_record.world_object_id.value) {
            ++attachment_index;
            continue;
        }

        if (attachment.component_type_id.value != binding_record.component_type_id.value) {
            ++attachment_index;
            continue;
        }

        if (attachment.component_slot_id.value == binding_record.component_slot_id.value) {
            return true;
        }

        ++attachment_index;
    }

    return false;
}

void WorldSceneAssemblyManifestStreamBridge::CopyOutputs(
    const WorldComponentAttachmentSnapshotRecord *decoded_attachments,
    std::uint32_t decoded_attachment_count,
    WorldComponentAttachmentSnapshotRecord *output_attachments,
    std::uint32_t *out_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *decoded_bindings,
    std::uint32_t decoded_binding_count,
    WorldComponentResourceBindingSnapshotRecord *output_bindings,
    std::uint32_t *out_binding_count) const {
    std::uint32_t attachment_index = 0U;
    while (attachment_index < decoded_attachment_count) {
        output_attachments[attachment_index] = decoded_attachments[attachment_index];
        ++attachment_index;
    }

    std::uint32_t binding_index = 0U;
    while (binding_index < decoded_binding_count) {
        output_bindings[binding_index] = decoded_bindings[binding_index];
        ++binding_index;
    }

    *out_attachment_count = decoded_attachment_count;
    *out_binding_count = decoded_binding_count;
}
}
