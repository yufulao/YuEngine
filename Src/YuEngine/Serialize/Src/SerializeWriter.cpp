// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Src/SerializeWriter.cpp

#include "YuEngine/Serialize/SerializeWriter.h"

#include "YuEngine/Memory/MemoryAccountingStatus.h"

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::serialize {
namespace {
std::uint32_t ClampCapacity(std::uint32_t capacity) {
    if (capacity > MAX_STREAM_BYTE_COUNT) {
        return MAX_STREAM_BYTE_COUNT;
    }

    return capacity;
}

void EncodeUInt32(std::uint8_t* bytes, std::uint32_t value) {
    bytes[0U] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void EncodeUInt64(std::uint8_t* bytes, std::uint64_t value) {
    bytes[0U] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    bytes[4U] = static_cast<std::uint8_t>((value >> 32U) & 0xFFU);
    bytes[5U] = static_cast<std::uint8_t>((value >> 40U) & 0xFFU);
    bytes[6U] = static_cast<std::uint8_t>((value >> 48U) & 0xFFU);
    bytes[7U] = static_cast<std::uint8_t>((value >> 56U) & 0xFFU);
}

void ClearCapacityEntry(SerializeSnapshot &snapshot) {
    snapshot.last_failed_record_id = SerializeRecordId{};
    snapshot.last_failed_field_id = SerializeFieldId{};
    snapshot.last_failed_entry_index = 0U;
}

void SetCapacityEntry(
    SerializeSnapshot &snapshot,
    SerializeRecordId record,
    SerializeFieldId field,
    std::uint32_t entry_index) {
    snapshot.last_failed_record_id = record;
    snapshot.last_failed_field_id = field;
    snapshot.last_failed_entry_index = entry_index;
}

static_assert(sizeof(SerializeSnapshot) == 68U);
}

SerializeWriter::SerializeWriter(std::uint8_t* buffer, std::uint32_t capacity)
    : buffer_(buffer),
      capacity_(ClampCapacity(capacity)),
      active_record_offset_(0U),
      current_record_id_(),
      current_record_field_count_(0U),
      current_record_fields_{},
      snapshot_{
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          SerializeStatus::Success},
      has_stream_(false),
      has_active_record_(false) {
}

SerializeStatus SerializeWriter::BeginStream() {
    if (!CanCommitBytes(STREAM_HEADER_BYTE_COUNT)) {
        return RecordFailure(SerializeStatus::BufferTooSmall);
    }

    const std::uint32_t previous_failed_operation_count = snapshot_.failed_operation_count;
    snapshot_ = SerializeSnapshot{
        STREAM_MAJOR_VERSION,
        STREAM_MINOR_VERSION,
        STREAM_HEADER_BYTE_COUNT,
        0U,
        0U,
        0U,
        previous_failed_operation_count,
        MemoryAccountingStatus::ExplicitlyTrackedOnly,
        SerializeStatus::Success};
    active_record_offset_ = 0U;
    current_record_id_ = SerializeRecordId{};
    current_record_field_count_ = 0U;
    has_stream_ = true;
    has_active_record_ = false;

    WriteUInt32At(STREAM_MAGIC_OFFSET, STREAM_MAGIC);
    WriteUInt16At(STREAM_MAJOR_VERSION_OFFSET, STREAM_MAJOR_VERSION);
    WriteUInt16At(STREAM_MINOR_VERSION_OFFSET, STREAM_MINOR_VERSION);
    WriteUInt32At(STREAM_FLAGS_OFFSET, STREAM_FLAGS);
    WriteUInt32At(STREAM_RECORD_COUNT_OFFSET, 0U);
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeWriter::BeginRecord(SerializeRecordId record) {
    if (!has_stream_) {
        return RecordFailure(SerializeStatus::InvalidHeader);
    }

    if (!record.IsValid()) {
        return RecordFailure(SerializeStatus::InvalidHeader);
    }

    if (snapshot_.record_count >= MAX_RECORDS_PER_STREAM) {
        snapshot_.last_required_record_count = snapshot_.record_count + 1U;
        snapshot_.last_required_field_count = 0U;
        SetCapacityEntry(snapshot_, record, SerializeFieldId{0U}, snapshot_.record_count);
        return RecordFailure(SerializeStatus::RecordCapacityExceeded);
    }

    if (!CanCommitBytes(RECORD_HEADER_BYTE_COUNT)) {
        return RecordFailure(SerializeStatus::BufferTooSmall);
    }

    active_record_offset_ = snapshot_.committed_byte_count;
    current_record_id_ = record;
    WriteUInt32At(active_record_offset_, record.value);
    WriteUInt32At(active_record_offset_ + sizeof(std::uint32_t), 0U);
    snapshot_.committed_byte_count += RECORD_HEADER_BYTE_COUNT;
    ++snapshot_.record_count;
    WriteUInt32At(STREAM_RECORD_COUNT_OFFSET, snapshot_.record_count);
    current_record_field_count_ = 0U;
    has_active_record_ = true;
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeWriter::WriteUInt32(SerializeFieldId field, std::uint32_t value) {
    std::array<std::uint8_t, UINT32_PAYLOAD_BYTE_COUNT> payload{};
    EncodeUInt32(payload.data(), value);
    return CommitField(field, SerializeTypeTag::UInt32, payload.data(), UINT32_PAYLOAD_BYTE_COUNT);
}

SerializeStatus SerializeWriter::WriteInt32(SerializeFieldId field, std::int32_t value) {
    std::array<std::uint8_t, INT32_PAYLOAD_BYTE_COUNT> payload{};
    EncodeUInt32(payload.data(), static_cast<std::uint32_t>(value));
    return CommitField(field, SerializeTypeTag::Int32, payload.data(), INT32_PAYLOAD_BYTE_COUNT);
}

SerializeStatus SerializeWriter::WriteUInt64(SerializeFieldId field, std::uint64_t value) {
    std::array<std::uint8_t, UINT64_PAYLOAD_BYTE_COUNT> payload{};
    EncodeUInt64(payload.data(), value);
    return CommitField(field, SerializeTypeTag::UInt64, payload.data(), UINT64_PAYLOAD_BYTE_COUNT);
}

SerializeStatus SerializeWriter::WriteInt64(SerializeFieldId field, std::int64_t value) {
    std::array<std::uint8_t, INT64_PAYLOAD_BYTE_COUNT> payload{};
    EncodeUInt64(payload.data(), static_cast<std::uint64_t>(value));
    return CommitField(field, SerializeTypeTag::Int64, payload.data(), INT64_PAYLOAD_BYTE_COUNT);
}

SerializeStatus SerializeWriter::WriteFixedBytes(SerializeFieldId field, const std::uint8_t* bytes, std::uint32_t byte_count) {
    return CommitField(field, SerializeTypeTag::FixedBytes, bytes, byte_count);
}

std::uint32_t SerializeWriter::GetByteCapacity() const {
    return capacity_;
}

std::uint32_t SerializeWriter::GetRemainingByteCapacity() const {
    if (snapshot_.committed_byte_count > capacity_) {
        return 0U;
    }

    return capacity_ - snapshot_.committed_byte_count;
}

bool SerializeWriter::CanCommitByteCount(std::uint32_t byte_count) const {
    return CanCommitBytes(byte_count);
}

SerializeSnapshot SerializeWriter::Snapshot() const {
    return snapshot_;
}

SerializeStatus SerializeWriter::CommitField(
    SerializeFieldId field,
    SerializeTypeTag type,
    const std::uint8_t* payload,
    std::uint32_t byte_count) {
    if (!has_stream_) {
        return RecordFailure(SerializeStatus::InvalidHeader);
    }

    if (!has_active_record_) {
        return RecordFailure(SerializeStatus::InvalidHeader);
    }

    if (!field.IsValid()) {
        return RecordFailure(SerializeStatus::InvalidHeader);
    }

    if (byte_count > MAX_FIELD_PAYLOAD_BYTE_COUNT) {
        return RecordFailure(SerializeStatus::FieldPayloadTooLarge);
    }

    if (byte_count > 0U && payload == nullptr) {
        return RecordFailure(SerializeStatus::BufferTooSmall);
    }

    if (snapshot_.field_count >= MAX_FIELDS_PER_STREAM) {
        snapshot_.last_required_record_count = 0U;
        snapshot_.last_required_field_count = snapshot_.field_count + 1U;
        SetCapacityEntry(snapshot_, current_record_id_, field, snapshot_.field_count);
        RecordFieldCapacityFailure(
            field,
            type,
            MAX_FIELDS_PER_STREAM,
            snapshot_.field_count,
            snapshot_.last_required_field_count);
        return RecordFailure(SerializeStatus::FieldCapacityExceeded);
    }

    if (current_record_field_count_ >= MAX_FIELDS_PER_RECORD) {
        snapshot_.last_required_record_count = 0U;
        snapshot_.last_required_field_count = current_record_field_count_ + 1U;
        SetCapacityEntry(snapshot_, current_record_id_, field, current_record_field_count_);
        RecordFieldCapacityFailure(
            field,
            type,
            MAX_FIELDS_PER_RECORD,
            current_record_field_count_,
            snapshot_.last_required_field_count);
        return RecordFailure(SerializeStatus::FieldCapacityExceeded);
    }

    if (HasFieldInCurrentRecord(field)) {
        return RecordFailure(SerializeStatus::DuplicateField);
    }

    if (!CanCommitBytes(FIELD_HEADER_BYTE_COUNT + byte_count)) {
        return RecordFailure(SerializeStatus::BufferTooSmall);
    }

    const std::uint32_t field_offset = snapshot_.committed_byte_count;
    WriteUInt32At(field_offset, field.value);
    WriteUInt32At(field_offset + sizeof(std::uint32_t), static_cast<std::uint32_t>(type));
    WriteUInt32At(field_offset + (sizeof(std::uint32_t) * 2U), byte_count);
    CopyPayload(field_offset + FIELD_HEADER_BYTE_COUNT, payload, byte_count);
    snapshot_.committed_byte_count += FIELD_HEADER_BYTE_COUNT + byte_count;
    current_record_fields_[current_record_field_count_] = field;
    ++current_record_field_count_;
    ++snapshot_.field_count;
    WriteUInt32At(active_record_offset_ + sizeof(std::uint32_t), current_record_field_count_);
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeWriter::RecordFailure(SerializeStatus status) {
    if (status == SerializeStatus::FieldCapacityExceeded) {
        ++snapshot_.failed_operation_count;
        snapshot_.last_status = status;
        return status;
    }

    if (status == SerializeStatus::RecordCapacityExceeded) {
        ClearFieldCapacityFailure();
        ++snapshot_.failed_operation_count;
        snapshot_.last_status = status;
        return status;
    }

    snapshot_.last_required_record_count = 0U;
    snapshot_.last_required_field_count = 0U;
    ClearCapacityEntry(snapshot_);
    ClearFieldCapacityFailure();
    ++snapshot_.failed_operation_count;

    snapshot_.last_status = status;
    return status;
}

void SerializeWriter::RecordSuccess() {
    ++snapshot_.accepted_operation_count;
    snapshot_.last_required_record_count = 0U;
    snapshot_.last_required_field_count = 0U;
    ClearCapacityEntry(snapshot_);
    ClearFieldCapacityFailure();
    snapshot_.last_status = SerializeStatus::Success;
}

void SerializeWriter::ClearFieldCapacityFailure() {
    snapshot_.last_failed_field_record_id = SerializeRecordId{};
    snapshot_.last_failed_field_id = SerializeFieldId{};
    snapshot_.last_failed_field_type = SerializeTypeTag::UInt32;
    snapshot_.last_failed_field_capacity = 0U;
    snapshot_.last_failed_field_count = 0U;
}

void SerializeWriter::RecordFieldCapacityFailure(
    SerializeFieldId field,
    SerializeTypeTag type,
    std::uint32_t field_capacity,
    std::uint32_t current_field_count,
    std::uint32_t required_field_count) {
    snapshot_.last_failed_field_record_id = current_record_id_;
    snapshot_.last_failed_field_id = field;
    snapshot_.last_failed_field_type = type;
    snapshot_.last_failed_field_capacity = field_capacity;
    snapshot_.last_failed_field_count = current_field_count;
    snapshot_.last_required_field_count = required_field_count;
}

bool SerializeWriter::CanCommitBytes(std::uint32_t byte_count) const {
    if (buffer_ == nullptr) {
        return false;
    }

    if (snapshot_.committed_byte_count > capacity_) {
        return false;
    }

    return byte_count <= (capacity_ - snapshot_.committed_byte_count);
}

bool SerializeWriter::HasFieldInCurrentRecord(SerializeFieldId field) const {
    std::uint32_t index = 0U;
    for (const SerializeFieldId& current_field : current_record_fields_) {
        if (index >= current_record_field_count_) {
            return false;
        }

        if (current_field.value == field.value) {
            return true;
        }

        ++index;
    }

    return false;
}

void SerializeWriter::WriteUInt16At(std::uint32_t offset, std::uint16_t value) {
    buffer_[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    buffer_[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void SerializeWriter::WriteUInt32At(std::uint32_t offset, std::uint32_t value) {
    EncodeUInt32(buffer_ + offset, value);
}

void SerializeWriter::CopyPayload(std::uint32_t offset, const std::uint8_t* payload, std::uint32_t byte_count) {
    std::uint32_t index = 0U;
    while (index < byte_count) {
        buffer_[offset + index] = payload[index];
        ++index;
    }
}
}
