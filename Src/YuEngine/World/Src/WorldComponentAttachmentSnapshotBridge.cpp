// Module: YuEngine World
// File: Src/YuEngine/World/Src/WorldComponentAttachmentSnapshotBridge.cpp

#include "YuEngine/World/WorldComponentAttachmentSnapshotBridge.h"

#include <array>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotConstants.h"
#include "YuEngine/World/WorldConstants.h"

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

std::uint32_t CalculateChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY;
}

std::uint32_t GetChunkRecordCount(std::uint32_t record_count, std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY) {
        return WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

SerializeRecordId AttachmentChunkRecordId(std::uint32_t chunk_index) {
    return SerializeRecordId{WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_ID_BASE + chunk_index};
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

void EncodeSnapshotRecord(
    std::uint8_t *bytes,
    const WorldComponentAttachmentSnapshotRecord &record) {
    EncodeUInt32(bytes, record.world_object_id.value);
    EncodeUInt32(bytes + 4U, record.component_type_id.value);
    EncodeUInt32(bytes + 8U, record.component_slot_id.value);
}

WorldComponentAttachmentSnapshotRecord DecodeSnapshotRecord(const std::uint8_t *bytes) {
    WorldComponentAttachmentSnapshotRecord record{};
    record.world_object_id.value = DecodeUInt32(bytes);
    record.component_type_id.value = DecodeUInt32(bytes + 4U);
    record.component_slot_id.value = DecodeUInt32(bytes + 8U);
    return record;
}

std::uint32_t RequiredWriteRecordCount(std::uint32_t attachment_record_count) {
    return 1U + CalculateChunkCount(attachment_record_count);
}

std::uint32_t RequiredWriteFieldCount(std::uint32_t attachment_record_count) {
    return WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_METADATA_FIELD_COUNT +
        CalculateChunkCount(attachment_record_count);
}

std::uint32_t RequiredChunkPayloadByteCount(std::uint32_t chunk_record_count) {
    return chunk_record_count * WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_RECORD_BYTE_COUNT;
}

std::uint32_t RequiredWriteByteCount(std::uint32_t attachment_record_count) {
    std::uint32_t result = WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_METADATA_RECORD_BYTE_COUNT;
    const std::uint32_t chunk_count = CalculateChunkCount(attachment_record_count);
    std::uint32_t chunk_index = 0U;
    while (chunk_index < chunk_count) {
        const std::uint32_t chunk_record_count = GetChunkRecordCount(
            attachment_record_count,
            chunk_index);
        result += yuengine::serialize::RECORD_HEADER_BYTE_COUNT;
        result += WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIXED_BYTES_FIELD_HEADER_BYTE_COUNT;
        result += RequiredChunkPayloadByteCount(chunk_record_count);
        ++chunk_index;
    }

    return result;
}

SerializeStatus ValidateWriteBudget(
    const SerializeWriter &writer,
    std::uint32_t attachment_record_count) {
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

    const std::uint32_t required_record_count = RequiredWriteRecordCount(attachment_record_count);
    if (writer_snapshot.record_count > yuengine::serialize::MAX_RECORDS_PER_STREAM) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t remaining_record_count =
        yuengine::serialize::MAX_RECORDS_PER_STREAM - writer_snapshot.record_count;
    if (required_record_count > remaining_record_count) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t required_field_count = RequiredWriteFieldCount(attachment_record_count);
    if (writer_snapshot.field_count > yuengine::serialize::MAX_FIELDS_PER_STREAM) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t remaining_field_count =
        yuengine::serialize::MAX_FIELDS_PER_STREAM - writer_snapshot.field_count;
    if (required_field_count > remaining_field_count) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t required_byte_count = RequiredWriteByteCount(attachment_record_count);
    if (required_byte_count > writer.GetRemainingByteCapacity()) {
        return SerializeStatus::BufferTooSmall;
    }

    if (!writer.CanCommitByteCount(required_byte_count)) {
        return SerializeStatus::BufferTooSmall;
    }

    return SerializeStatus::Success;
}

SerializeStatus WriteMetadataRecord(SerializeWriter &writer, std::uint32_t attachment_record_count) {
    SerializeStatus status = writer.BeginRecord(WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_METADATA_RECORD_ID);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_SCHEMA_VERSION,
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_SCHEMA_VERSION);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_RECORD_COUNT,
        attachment_record_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteUInt32(
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_CHUNK_COUNT,
        CalculateChunkCount(attachment_record_count));
}

SerializeStatus WriteChunkRecord(
    SerializeWriter &writer,
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t attachment_record_count,
    std::uint32_t chunk_index) {
    std::array<std::uint8_t, WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_PAYLOAD_BYTE_COUNT> payload{};
    const std::uint32_t first_record_index =
        chunk_index * WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY;
    const std::uint32_t chunk_record_count = GetChunkRecordCount(attachment_record_count, chunk_index);
    std::uint32_t record_index = 0U;
    while (record_index < chunk_record_count) {
        const WorldComponentAttachmentSnapshotRecord &record = records[first_record_index + record_index];
        const std::uint32_t payload_offset =
            record_index * WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_RECORD_BYTE_COUNT;
        EncodeSnapshotRecord(payload.data() + payload_offset, record);
        ++record_index;
    }

    SerializeStatus status = writer.BeginRecord(AttachmentChunkRecordId(chunk_index));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteFixedBytes(
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_FIELD_RECORD_BYTES,
        payload.data(),
        RequiredChunkPayloadByteCount(chunk_record_count));
}

bool HasDuplicateRecord(
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t record_index) {
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
}

WorldComponentAttachmentSnapshotBridge::WorldComponentAttachmentSnapshotBridge(
    WorldComponentAttachmentSnapshotBridgeDesc desc)
    : attachment_capacity_(ClampCapacity(desc.attachment_capacity)),
      snapshot_{
          ClampCapacity(desc.attachment_capacity),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          SerializeStatus::Success,
          WorldComponentAttachmentSnapshotStatus::Success} {
    if (desc.attachment_capacity == 0U) {
        snapshot_.last_status = WorldComponentAttachmentSnapshotStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldComponentAttachmentSnapshotResult WorldComponentAttachmentSnapshotBridge::WriteSnapshot(
    SerializeWriter *writer,
    const WorldComponentAttachmentBridge *source_bridge) {
    const WorldComponentAttachmentSnapshotStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentSnapshotStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (source_bridge == nullptr) {
        return RecordFailure(WorldComponentAttachmentSnapshotStatus::InvalidSourceBridge);
    }

    if (writer == nullptr) {
        return RecordFailure(WorldComponentAttachmentSnapshotStatus::InvalidWriter);
    }

    std::array<WorldComponentAttachment, MAX_WORLD_OBJECT_COUNT> attachments{};
    const std::uint32_t attachment_count =
        source_bridge->ExportAttachments(attachments.data(), attachment_capacity_);
    if (attachment_count > attachment_capacity_) {
        return RecordFailure(WorldComponentAttachmentSnapshotStatus::RecordCountExceeded);
    }

    std::array<WorldComponentAttachmentSnapshotRecord, MAX_WORLD_OBJECT_COUNT> records{};
    std::uint32_t record_index = 0U;
    while (record_index < attachment_count) {
        const WorldComponentAttachment &attachment = attachments[record_index];
        records[record_index].world_object_id = attachment.world_object_id;
        records[record_index].component_type_id = attachment.component_type_id;
        records[record_index].component_slot_id = attachment.component_slot_id;
        ++record_index;
    }

    const SerializeStatus budget_status = ValidateWriteBudget(*writer, attachment_count);
    if (budget_status != SerializeStatus::Success) {
        return RecordSerializeFailure(budget_status);
    }

    SerializeStatus status = WriteMetadataRecord(*writer, attachment_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    const std::uint32_t chunk_count = CalculateChunkCount(attachment_count);
    std::uint32_t chunk_index = 0U;
    while (chunk_index < chunk_count) {
        status = WriteChunkRecord(*writer, records.data(), attachment_count, chunk_index);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        ++chunk_index;
    }

    WorldComponentAttachmentSnapshotState state{};
    state.attachment_record_count = attachment_count;
    state.chunk_record_count = chunk_count;
    state.committed_byte_count = writer->Snapshot().committed_byte_count;
    return RecordWriteSuccess(state);
}

WorldComponentAttachmentSnapshotResult WorldComponentAttachmentSnapshotBridge::ReadSnapshot(
    SerializeReader *reader,
    WorldComponentAttachmentBridge *destination_bridge) {
    const WorldComponentAttachmentSnapshotStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentAttachmentSnapshotStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (reader == nullptr) {
        return RecordFailure(WorldComponentAttachmentSnapshotStatus::InvalidReader);
    }

    if (destination_bridge == nullptr) {
        return RecordFailure(WorldComponentAttachmentSnapshotStatus::InvalidDestinationBridge);
    }

    SerializeStatus status = reader->OpenStream();
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    std::uint32_t schema_version = 0U;
    status = reader->ReadUInt32(
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_METADATA_RECORD_ID,
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_SCHEMA_VERSION,
        schema_version);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    if (schema_version != WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_SCHEMA_VERSION) {
        return RecordFailure(WorldComponentAttachmentSnapshotStatus::UnsupportedVersion);
    }

    std::uint32_t attachment_record_count = 0U;
    status = reader->ReadUInt32(
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_METADATA_RECORD_ID,
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_RECORD_COUNT,
        attachment_record_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    std::uint32_t chunk_count = 0U;
    status = reader->ReadUInt32(
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_METADATA_RECORD_ID,
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_CHUNK_COUNT,
        chunk_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    if (attachment_record_count > attachment_capacity_) {
        return RecordFailure(WorldComponentAttachmentSnapshotStatus::MalformedRecordCount);
    }

    if (chunk_count != CalculateChunkCount(attachment_record_count)) {
        return RecordFailure(WorldComponentAttachmentSnapshotStatus::MalformedRecordCount);
    }

    std::array<WorldComponentAttachmentSnapshotRecord, MAX_WORLD_OBJECT_COUNT> records{};
    std::array<std::uint8_t, WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_PAYLOAD_BYTE_COUNT> payload{};
    std::uint32_t chunk_index = 0U;
    while (chunk_index < chunk_count) {
        const std::uint32_t chunk_record_count = GetChunkRecordCount(
            attachment_record_count,
            chunk_index);
        const std::uint32_t expected_byte_count = RequiredChunkPayloadByteCount(chunk_record_count);
        std::uint32_t payload_byte_count = 0U;
        status = reader->ReadFixedBytes(
            AttachmentChunkRecordId(chunk_index),
            WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_FIELD_RECORD_BYTES,
            payload.data(),
            WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_PAYLOAD_BYTE_COUNT,
            payload_byte_count);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        if (payload_byte_count != expected_byte_count) {
            return RecordFailure(WorldComponentAttachmentSnapshotStatus::MalformedRecordCount);
        }

        std::uint32_t record_index = 0U;
        while (record_index < chunk_record_count) {
            const std::uint32_t payload_offset =
                record_index * WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_RECORD_BYTE_COUNT;
            const std::uint32_t output_record_index =
                (chunk_index * WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY) + record_index;
            records[output_record_index] = DecodeSnapshotRecord(payload.data() + payload_offset);
            ++record_index;
        }

        ++chunk_index;
    }

    const WorldComponentAttachmentSnapshotStatus record_status = ValidateRecords(
        records.data(),
        attachment_record_count);
    if (record_status != WorldComponentAttachmentSnapshotStatus::Success) {
        return RecordRejectedFailure(record_status);
    }

    const WorldComponentAttachmentSnapshotStatus restore_status = RestoreRecords(
        destination_bridge,
        records.data(),
        attachment_record_count);
    if (restore_status != WorldComponentAttachmentSnapshotStatus::Success) {
        return RecordFailure(restore_status);
    }

    WorldComponentAttachmentSnapshotState state{};
    state.attachment_record_count = attachment_record_count;
    state.chunk_record_count = chunk_count;
    state.committed_byte_count = reader->Snapshot().committed_byte_count;
    return RecordReadSuccess(state);
}

WorldComponentAttachmentSnapshotBridgeSnapshot WorldComponentAttachmentSnapshotBridge::Snapshot() const {
    return snapshot_;
}

WorldComponentAttachmentSnapshotResult WorldComponentAttachmentSnapshotBridge::RecordFailure(
    WorldComponentAttachmentSnapshotStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = status;
    return WorldComponentAttachmentSnapshotResult::Failure(status);
}

WorldComponentAttachmentSnapshotResult WorldComponentAttachmentSnapshotBridge::RecordRejectedFailure(
    WorldComponentAttachmentSnapshotStatus status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status);
}

WorldComponentAttachmentSnapshotResult WorldComponentAttachmentSnapshotBridge::RecordSerializeFailure(
    SerializeStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_serialize_status = status;
    snapshot_.last_status = WorldComponentAttachmentSnapshotStatus::SerializeFailure;
    return WorldComponentAttachmentSnapshotResult::Failure(
        WorldComponentAttachmentSnapshotStatus::SerializeFailure,
        status);
}

WorldComponentAttachmentSnapshotResult WorldComponentAttachmentSnapshotBridge::RecordWriteSuccess(
    const WorldComponentAttachmentSnapshotState &state) {
    ++snapshot_.write_count;
    snapshot_.written_record_count += state.attachment_record_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = WorldComponentAttachmentSnapshotStatus::Success;
    return WorldComponentAttachmentSnapshotResult::Success(state);
}

WorldComponentAttachmentSnapshotResult WorldComponentAttachmentSnapshotBridge::RecordReadSuccess(
    const WorldComponentAttachmentSnapshotState &state) {
    ++snapshot_.read_count;
    snapshot_.read_record_count += state.attachment_record_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = WorldComponentAttachmentSnapshotStatus::Success;
    return WorldComponentAttachmentSnapshotResult::Success(state);
}

WorldComponentAttachmentSnapshotStatus WorldComponentAttachmentSnapshotBridge::ValidateBridgeCapacity() const {
    if (attachment_capacity_ == 0U) {
        return WorldComponentAttachmentSnapshotStatus::InvalidBridgeCapacity;
    }

    return WorldComponentAttachmentSnapshotStatus::Success;
}

WorldComponentAttachmentSnapshotStatus WorldComponentAttachmentSnapshotBridge::ValidateRecords(
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t record_count) const {
    if (record_count > attachment_capacity_) {
        return WorldComponentAttachmentSnapshotStatus::RecordCountExceeded;
    }

    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        const WorldComponentAttachmentSnapshotRecord &record = records[record_index];
        if (!record.world_object_id.IsValid()) {
            return WorldComponentAttachmentSnapshotStatus::InvalidWorldObjectId;
        }

        if (!record.component_type_id.IsValid()) {
            return WorldComponentAttachmentSnapshotStatus::InvalidComponentTypeId;
        }

        if (!record.component_slot_id.IsValid()) {
            return WorldComponentAttachmentSnapshotStatus::InvalidComponentSlotId;
        }

        if (HasDuplicateRecord(records, record_index)) {
            return WorldComponentAttachmentSnapshotStatus::DuplicateAttachment;
        }

        ++record_index;
    }

    return WorldComponentAttachmentSnapshotStatus::Success;
}

WorldComponentAttachmentSnapshotStatus WorldComponentAttachmentSnapshotBridge::RestoreRecords(
    WorldComponentAttachmentBridge *destination_bridge,
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t record_count) const {
    const WorldComponentAttachmentSnapshot destination_snapshot = destination_bridge->Snapshot();
    if (record_count > destination_snapshot.attachment_capacity) {
        return WorldComponentAttachmentSnapshotStatus::DestinationCapacityExceeded;
    }

    const WorldComponentAttachmentStatus clear_status = destination_bridge->Clear();
    if (clear_status != WorldComponentAttachmentStatus::Success) {
        return WorldComponentAttachmentSnapshotStatus::DestinationClearFailed;
    }

    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        const WorldComponentAttachmentSnapshotRecord &record = records[record_index];
        const WorldComponentAttachmentResult add_result = destination_bridge->Add(
            record.world_object_id,
            record.component_type_id,
            record.component_slot_id);
        if (!add_result.Succeeded()) {
            return WorldComponentAttachmentSnapshotStatus::DestinationRestoreFailed;
        }

        ++record_index;
    }

    return WorldComponentAttachmentSnapshotStatus::Success;
}
}
