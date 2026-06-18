// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldComponentResourceBindingSnapshotBridge.cpp

#include "YuEngine/World/WorldComponentResourceBindingSnapshotBridge.h"

#include <array>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/World/WorldComponentResourceBindingBridge.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotConstants.h"
#include "YuEngine/World/WorldConstants.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceTypeId;
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
    return (record_count + WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY;
}

std::uint32_t GetChunkRecordCount(std::uint32_t record_count, std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY) {
        return WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

SerializeRecordId BindingChunkRecordId(std::uint32_t chunk_index) {
    return SerializeRecordId{WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_ID_BASE + chunk_index};
}

void EncodeUInt32(std::uint8_t *bytes, std::uint32_t value) {
    bytes[0U] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

std::uint32_t ReadUInt32FromBytes(const std::uint8_t *bytes) {
    const std::uint32_t byte0 = static_cast<std::uint32_t>(bytes[0U]);
    const std::uint32_t byte1 = static_cast<std::uint32_t>(bytes[1U]);
    const std::uint32_t byte2 = static_cast<std::uint32_t>(bytes[2U]);
    const std::uint32_t byte3 = static_cast<std::uint32_t>(bytes[3U]);
    return byte0 | (byte1 << 8U) | (byte2 << 16U) | (byte3 << 24U);
}

void EncodeSnapshotRecord(
    std::uint8_t *bytes,
    const WorldComponentResourceBindingSnapshotRecord &record) {
    EncodeUInt32(bytes, record.world_object_id.value);
    EncodeUInt32(bytes + 4U, record.component_type_id.value);
    EncodeUInt32(bytes + 8U, record.component_slot_id.value);
    EncodeUInt32(bytes + 12U, record.resource_handle.slot);
    EncodeUInt32(bytes + 16U, record.resource_handle.generation);
    EncodeUInt32(bytes + 20U, record.expected_resource_type.value);
}

WorldComponentResourceBindingSnapshotRecord ReadSnapshotRecordFromBytes(const std::uint8_t *bytes) {
    WorldComponentResourceBindingSnapshotRecord record{};
    record.world_object_id.value = ReadUInt32FromBytes(bytes);
    record.component_type_id.value = ReadUInt32FromBytes(bytes + 4U);
    record.component_slot_id.value = ReadUInt32FromBytes(bytes + 8U);
    record.resource_handle.slot = ReadUInt32FromBytes(bytes + 12U);
    record.resource_handle.generation = ReadUInt32FromBytes(bytes + 16U);
    record.expected_resource_type.value = ReadUInt32FromBytes(bytes + 20U);
    return record;
}

std::uint32_t RequiredWriteRecordCount(std::uint32_t binding_record_count) {
    return 1U + CalculateChunkCount(binding_record_count);
}

std::uint32_t RequiredWriteFieldCount(std::uint32_t binding_record_count) {
    return WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_METADATA_FIELD_COUNT +
        CalculateChunkCount(binding_record_count);
}

std::uint32_t RequiredChunkPayloadByteCount(std::uint32_t chunk_record_count) {
    return chunk_record_count * WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_RECORD_BYTE_COUNT;
}

std::uint32_t RequiredWriteByteCount(std::uint32_t binding_record_count) {
    std::uint32_t result = WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_METADATA_RECORD_BYTE_COUNT;
    const std::uint32_t chunk_count = CalculateChunkCount(binding_record_count);
    std::uint32_t chunk_index = 0U;
    while (chunk_index < chunk_count) {
        const std::uint32_t chunk_record_count = GetChunkRecordCount(
            binding_record_count,
            chunk_index);
        result += yuengine::serialize::RECORD_HEADER_BYTE_COUNT;
        result += WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIXED_BYTES_FIELD_HEADER_BYTE_COUNT;
        result += RequiredChunkPayloadByteCount(chunk_record_count);
        ++chunk_index;
    }

    return result;
}

SerializeStatus ValidateWriteBudget(
    const SerializeWriter &writer,
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

    const std::uint32_t required_record_count = RequiredWriteRecordCount(binding_record_count);
    if (writer_snapshot.record_count > yuengine::serialize::MAX_RECORDS_PER_STREAM) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t remaining_record_count =
        yuengine::serialize::MAX_RECORDS_PER_STREAM - writer_snapshot.record_count;
    if (required_record_count > remaining_record_count) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t required_field_count = RequiredWriteFieldCount(binding_record_count);
    if (writer_snapshot.field_count > yuengine::serialize::MAX_FIELDS_PER_STREAM) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t remaining_field_count =
        yuengine::serialize::MAX_FIELDS_PER_STREAM - writer_snapshot.field_count;
    if (required_field_count > remaining_field_count) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t required_byte_count = RequiredWriteByteCount(binding_record_count);
    if (required_byte_count > writer.GetRemainingByteCapacity()) {
        return SerializeStatus::BufferTooSmall;
    }

    if (!writer.CanCommitByteCount(required_byte_count)) {
        return SerializeStatus::BufferTooSmall;
    }

    return SerializeStatus::Success;
}

SerializeStatus WriteMetadataRecord(SerializeWriter &writer, std::uint32_t binding_record_count) {
    SerializeStatus status = writer.BeginRecord(WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_METADATA_RECORD_ID);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_SCHEMA_VERSION,
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_SCHEMA_VERSION);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_RECORD_COUNT,
        binding_record_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteUInt32(
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_CHUNK_COUNT,
        CalculateChunkCount(binding_record_count));
}

SerializeStatus WriteChunkRecord(
    SerializeWriter &writer,
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t binding_record_count,
    std::uint32_t chunk_index) {
    std::array<std::uint8_t, WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_PAYLOAD_BYTE_COUNT> payload{};
    const std::uint32_t first_record_index =
        chunk_index * WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY;
    const std::uint32_t chunk_record_count = GetChunkRecordCount(binding_record_count, chunk_index);
    std::uint32_t record_index = 0U;
    while (record_index < chunk_record_count) {
        const WorldComponentResourceBindingSnapshotRecord &record =
            records[first_record_index + record_index];
        const std::uint32_t payload_offset =
            record_index * WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_RECORD_BYTE_COUNT;
        EncodeSnapshotRecord(payload.data() + payload_offset, record);
        ++record_index;
    }

    SerializeStatus status = writer.BeginRecord(BindingChunkRecordId(chunk_index));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteFixedBytes(
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_FIELD_RECORD_BYTES,
        payload.data(),
        RequiredChunkPayloadByteCount(chunk_record_count));
}

bool HasDuplicateRecord(
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t record_index) {
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
}

WorldComponentResourceBindingSnapshotBridge::WorldComponentResourceBindingSnapshotBridge(
    WorldComponentResourceBindingSnapshotBridgeDesc desc)
    : binding_capacity_(ClampCapacity(desc.binding_capacity)),
      snapshot_{
          ClampCapacity(desc.binding_capacity),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          SerializeStatus::Success,
          WorldComponentResourceBindingSnapshotStatus::Success} {
    if (desc.binding_capacity == 0U) {
        snapshot_.last_status = WorldComponentResourceBindingSnapshotStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldComponentResourceBindingSnapshotResult WorldComponentResourceBindingSnapshotBridge::WriteSnapshot(
    SerializeWriter *writer,
    const WorldComponentResourceBindingBridge *source_bridge) {
    const WorldComponentResourceBindingSnapshotStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentResourceBindingSnapshotStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (source_bridge == nullptr) {
        return RecordFailure(WorldComponentResourceBindingSnapshotStatus::InvalidSourceBridge);
    }

    if (writer == nullptr) {
        return RecordFailure(WorldComponentResourceBindingSnapshotStatus::InvalidWriter);
    }

    std::array<WorldComponentResourceBinding, MAX_WORLD_OBJECT_COUNT> bindings{};
    const std::uint32_t binding_count = source_bridge->ExportBindings(bindings.data(), binding_capacity_);
    if (binding_count > binding_capacity_) {
        return RecordFailure(WorldComponentResourceBindingSnapshotStatus::RecordCountExceeded);
    }

    std::array<WorldComponentResourceBindingSnapshotRecord, MAX_WORLD_OBJECT_COUNT> records{};
    std::uint32_t record_index = 0U;
    while (record_index < binding_count) {
        const WorldComponentResourceBinding &binding = bindings[record_index];
        records[record_index].world_object_id = binding.world_object_id;
        records[record_index].component_type_id = binding.component_type_id;
        records[record_index].component_slot_id = binding.component_slot_id;
        records[record_index].resource_handle = binding.resource_handle;
        records[record_index].expected_resource_type = binding.expected_resource_type;
        ++record_index;
    }

    const WorldComponentResourceBindingSnapshotStatus record_status = ValidateRecords(
        records.data(),
        binding_count);
    if (record_status != WorldComponentResourceBindingSnapshotStatus::Success) {
        return RecordRejectedFailure(record_status);
    }

    const SerializeStatus budget_status = ValidateWriteBudget(*writer, binding_count);
    if (budget_status != SerializeStatus::Success) {
        return RecordSerializeFailure(budget_status);
    }

    SerializeStatus status = WriteMetadataRecord(*writer, binding_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    const std::uint32_t chunk_count = CalculateChunkCount(binding_count);
    std::uint32_t chunk_index = 0U;
    while (chunk_index < chunk_count) {
        status = WriteChunkRecord(*writer, records.data(), binding_count, chunk_index);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        ++chunk_index;
    }

    WorldComponentResourceBindingSnapshotState state{};
    state.binding_record_count = binding_count;
    state.chunk_record_count = chunk_count;
    state.committed_byte_count = writer->Snapshot().committed_byte_count;
    return RecordWriteSuccess(state);
}

WorldComponentResourceBindingSnapshotResult WorldComponentResourceBindingSnapshotBridge::ReadSnapshot(
    SerializeReader *reader,
    WorldComponentResourceBinding *output_bindings,
    std::uint32_t output_capacity,
    std::uint32_t *out_binding_count) {
    const WorldComponentResourceBindingSnapshotStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldComponentResourceBindingSnapshotStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (reader == nullptr) {
        return RecordFailure(WorldComponentResourceBindingSnapshotStatus::InvalidReader);
    }

    if (output_bindings == nullptr) {
        return RecordFailure(WorldComponentResourceBindingSnapshotStatus::InvalidOutput);
    }

    if (out_binding_count == nullptr) {
        return RecordFailure(WorldComponentResourceBindingSnapshotStatus::InvalidOutputCount);
    }

    SerializeStatus status = reader->OpenStream();
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    std::uint32_t schema_version = 0U;
    status = reader->ReadUInt32(
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_METADATA_RECORD_ID,
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_SCHEMA_VERSION,
        schema_version);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    if (schema_version != WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_SCHEMA_VERSION) {
        return RecordFailure(WorldComponentResourceBindingSnapshotStatus::UnsupportedVersion);
    }

    std::uint32_t binding_record_count = 0U;
    status = reader->ReadUInt32(
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_METADATA_RECORD_ID,
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_RECORD_COUNT,
        binding_record_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    std::uint32_t chunk_count = 0U;
    status = reader->ReadUInt32(
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_METADATA_RECORD_ID,
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_CHUNK_COUNT,
        chunk_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    if (binding_record_count > binding_capacity_) {
        return RecordFailure(WorldComponentResourceBindingSnapshotStatus::MalformedRecordCount);
    }

    if (binding_record_count > output_capacity) {
        return RecordFailure(WorldComponentResourceBindingSnapshotStatus::OutputCapacityExceeded);
    }

    if (chunk_count != CalculateChunkCount(binding_record_count)) {
        return RecordFailure(WorldComponentResourceBindingSnapshotStatus::MalformedRecordCount);
    }

    std::array<WorldComponentResourceBindingSnapshotRecord, MAX_WORLD_OBJECT_COUNT> records{};
    std::array<std::uint8_t, WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_PAYLOAD_BYTE_COUNT> payload{};
    std::uint32_t chunk_index = 0U;
    while (chunk_index < chunk_count) {
        const std::uint32_t chunk_record_count = GetChunkRecordCount(
            binding_record_count,
            chunk_index);
        const std::uint32_t expected_byte_count = RequiredChunkPayloadByteCount(chunk_record_count);
        std::uint32_t payload_byte_count = 0U;
        status = reader->ReadFixedBytes(
            BindingChunkRecordId(chunk_index),
            WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_FIELD_RECORD_BYTES,
            payload.data(),
            WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_PAYLOAD_BYTE_COUNT,
            payload_byte_count);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        if (payload_byte_count != expected_byte_count) {
            return RecordFailure(WorldComponentResourceBindingSnapshotStatus::MalformedRecordCount);
        }

        std::uint32_t record_index = 0U;
        while (record_index < chunk_record_count) {
            const std::uint32_t payload_offset =
                record_index * WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_RECORD_BYTE_COUNT;
            const std::uint32_t output_record_index =
                (chunk_index * WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY) + record_index;
            records[output_record_index] = ReadSnapshotRecordFromBytes(payload.data() + payload_offset);
            ++record_index;
        }

        ++chunk_index;
    }

    const WorldComponentResourceBindingSnapshotStatus record_status = ValidateRecords(
        records.data(),
        binding_record_count);
    if (record_status != WorldComponentResourceBindingSnapshotStatus::Success) {
        return RecordRejectedFailure(record_status);
    }

    WriteOutputBindings(records.data(), binding_record_count, output_bindings);
    *out_binding_count = binding_record_count;

    WorldComponentResourceBindingSnapshotState state{};
    state.binding_record_count = binding_record_count;
    state.chunk_record_count = chunk_count;
    state.committed_byte_count = reader->Snapshot().committed_byte_count;
    return RecordReadSuccess(state);
}

WorldComponentResourceBindingSnapshotBridgeSnapshot WorldComponentResourceBindingSnapshotBridge::Snapshot() const {
    return snapshot_;
}

WorldComponentResourceBindingSnapshotResult WorldComponentResourceBindingSnapshotBridge::RecordFailure(
    WorldComponentResourceBindingSnapshotStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = status;
    return WorldComponentResourceBindingSnapshotResult::Failure(status);
}

WorldComponentResourceBindingSnapshotResult WorldComponentResourceBindingSnapshotBridge::RecordRejectedFailure(
    WorldComponentResourceBindingSnapshotStatus status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status);
}

WorldComponentResourceBindingSnapshotResult WorldComponentResourceBindingSnapshotBridge::RecordSerializeFailure(
    SerializeStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_serialize_status = status;
    snapshot_.last_status = WorldComponentResourceBindingSnapshotStatus::SerializeFailure;
    return WorldComponentResourceBindingSnapshotResult::Failure(
        WorldComponentResourceBindingSnapshotStatus::SerializeFailure,
        status);
}

WorldComponentResourceBindingSnapshotResult WorldComponentResourceBindingSnapshotBridge::RecordWriteSuccess(
    const WorldComponentResourceBindingSnapshotState &state) {
    ++snapshot_.write_count;
    snapshot_.written_record_count += state.binding_record_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = WorldComponentResourceBindingSnapshotStatus::Success;
    return WorldComponentResourceBindingSnapshotResult::Success(state);
}

WorldComponentResourceBindingSnapshotResult WorldComponentResourceBindingSnapshotBridge::RecordReadSuccess(
    const WorldComponentResourceBindingSnapshotState &state) {
    ++snapshot_.read_count;
    snapshot_.read_record_count += state.binding_record_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = WorldComponentResourceBindingSnapshotStatus::Success;
    return WorldComponentResourceBindingSnapshotResult::Success(state);
}

WorldComponentResourceBindingSnapshotStatus WorldComponentResourceBindingSnapshotBridge::ValidateBridgeCapacity() const {
    if (binding_capacity_ == 0U) {
        return WorldComponentResourceBindingSnapshotStatus::InvalidBridgeCapacity;
    }

    return WorldComponentResourceBindingSnapshotStatus::Success;
}

WorldComponentResourceBindingSnapshotStatus WorldComponentResourceBindingSnapshotBridge::ValidateRecords(
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t record_count) const {
    if (record_count > binding_capacity_) {
        return WorldComponentResourceBindingSnapshotStatus::RecordCountExceeded;
    }

    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        const WorldComponentResourceBindingSnapshotRecord &record = records[record_index];
        if (!record.world_object_id.IsValid()) {
            return WorldComponentResourceBindingSnapshotStatus::InvalidWorldObjectId;
        }

        if (!record.component_type_id.IsValid()) {
            return WorldComponentResourceBindingSnapshotStatus::InvalidComponentTypeId;
        }

        if (!record.component_slot_id.IsValid()) {
            return WorldComponentResourceBindingSnapshotStatus::InvalidComponentSlotId;
        }

        if (!record.resource_handle.IsValid()) {
            return WorldComponentResourceBindingSnapshotStatus::InvalidResourceHandle;
        }

        if (!record.expected_resource_type.IsValid()) {
            return WorldComponentResourceBindingSnapshotStatus::InvalidResourceTypeId;
        }

        if (HasDuplicateRecord(records, record_index)) {
            return WorldComponentResourceBindingSnapshotStatus::DuplicateBinding;
        }

        ++record_index;
    }

    return WorldComponentResourceBindingSnapshotStatus::Success;
}

void WorldComponentResourceBindingSnapshotBridge::WriteOutputBindings(
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t record_count,
    WorldComponentResourceBinding *output_bindings) const {
    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        const WorldComponentResourceBindingSnapshotRecord &record = records[record_index];
        WorldComponentResourceBinding &output_binding = output_bindings[record_index];
        output_binding.world_object_id = record.world_object_id;
        output_binding.component_type_id = record.component_type_id;
        output_binding.component_slot_id = record.component_slot_id;
        output_binding.resource_handle = record.resource_handle;
        output_binding.expected_resource_type = record.expected_resource_type;
        output_binding.is_bound = true;
        output_binding.is_acquired = false;
        ++record_index;
    }
}
}
