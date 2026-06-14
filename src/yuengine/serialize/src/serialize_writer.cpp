#include "yuengine/serialize/serialize_writer.h"

#include "yuengine/memory/memory_accounting_status.h"

using yuengine::memory::MEMORY_ACCOUNTING_STATUS;

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
}

static_assert(sizeof(SerializeSnapshot) == 32U);

SerializeWriter::SerializeWriter(std::uint8_t* buffer, std::uint32_t capacity)
    : _buffer(buffer),
      _capacity(ClampCapacity(capacity)),
      _activeRecordOffset(0U),
      _currentRecordFieldCount(0U),
      _currentRecordFields{},
      _snapshot{
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MEMORY_ACCOUNTING_STATUS::ExplicitlyTrackedOnly,
          SERIALIZE_STATUS::Success},
      _hasStream(false),
      _hasActiveRecord(false) {
}

SERIALIZE_STATUS SerializeWriter::BeginStream() {
    if (!CanCommitBytes(STREAM_HEADER_BYTE_COUNT)) {
        return RecordFailure(SERIALIZE_STATUS::BufferTooSmall);
    }

    _snapshot = SerializeSnapshot{
        STREAM_MAJOR_VERSION,
        STREAM_MINOR_VERSION,
        STREAM_HEADER_BYTE_COUNT,
        0U,
        0U,
        0U,
        0U,
        MEMORY_ACCOUNTING_STATUS::ExplicitlyTrackedOnly,
        SERIALIZE_STATUS::Success};
    _activeRecordOffset = 0U;
    _currentRecordFieldCount = 0U;
    _hasStream = true;
    _hasActiveRecord = false;

    WriteUInt32At(STREAM_MAGIC_OFFSET, STREAM_MAGIC);
    WriteUInt16At(STREAM_MAJOR_VERSION_OFFSET, STREAM_MAJOR_VERSION);
    WriteUInt16At(STREAM_MINOR_VERSION_OFFSET, STREAM_MINOR_VERSION);
    WriteUInt32At(STREAM_FLAGS_OFFSET, STREAM_FLAGS);
    WriteUInt32At(STREAM_RECORD_COUNT_OFFSET, 0U);
    RecordSuccess();
    return SERIALIZE_STATUS::Success;
}

SERIALIZE_STATUS SerializeWriter::BeginRecord(SerializeRecordId record) {
    if (!_hasStream) {
        return RecordFailure(SERIALIZE_STATUS::InvalidHeader);
    }

    if (!record.IsValid()) {
        return RecordFailure(SERIALIZE_STATUS::InvalidHeader);
    }

    if (_snapshot.RecordCount >= MAX_RECORDS_PER_STREAM) {
        return RecordFailure(SERIALIZE_STATUS::RecordCapacityExceeded);
    }

    if (!CanCommitBytes(RECORD_HEADER_BYTE_COUNT)) {
        return RecordFailure(SERIALIZE_STATUS::BufferTooSmall);
    }

    _activeRecordOffset = _snapshot.CommittedByteCount;
    WriteUInt32At(_activeRecordOffset, record.Value);
    WriteUInt32At(_activeRecordOffset + sizeof(std::uint32_t), 0U);
    _snapshot.CommittedByteCount += RECORD_HEADER_BYTE_COUNT;
    ++_snapshot.RecordCount;
    WriteUInt32At(STREAM_RECORD_COUNT_OFFSET, _snapshot.RecordCount);
    _currentRecordFieldCount = 0U;
    _hasActiveRecord = true;
    RecordSuccess();
    return SERIALIZE_STATUS::Success;
}

SERIALIZE_STATUS SerializeWriter::WriteUInt32(SerializeFieldId field, std::uint32_t value) {
    std::array<std::uint8_t, UINT32_PAYLOAD_BYTE_COUNT> payload{};
    EncodeUInt32(payload.data(), value);
    return CommitField(field, SERIALIZE_TYPE_TAG::UInt32, payload.data(), UINT32_PAYLOAD_BYTE_COUNT);
}

SERIALIZE_STATUS SerializeWriter::WriteInt32(SerializeFieldId field, std::int32_t value) {
    std::array<std::uint8_t, INT32_PAYLOAD_BYTE_COUNT> payload{};
    EncodeUInt32(payload.data(), static_cast<std::uint32_t>(value));
    return CommitField(field, SERIALIZE_TYPE_TAG::Int32, payload.data(), INT32_PAYLOAD_BYTE_COUNT);
}

SERIALIZE_STATUS SerializeWriter::WriteUInt64(SerializeFieldId field, std::uint64_t value) {
    std::array<std::uint8_t, UINT64_PAYLOAD_BYTE_COUNT> payload{};
    EncodeUInt64(payload.data(), value);
    return CommitField(field, SERIALIZE_TYPE_TAG::UInt64, payload.data(), UINT64_PAYLOAD_BYTE_COUNT);
}

SERIALIZE_STATUS SerializeWriter::WriteInt64(SerializeFieldId field, std::int64_t value) {
    std::array<std::uint8_t, INT64_PAYLOAD_BYTE_COUNT> payload{};
    EncodeUInt64(payload.data(), static_cast<std::uint64_t>(value));
    return CommitField(field, SERIALIZE_TYPE_TAG::Int64, payload.data(), INT64_PAYLOAD_BYTE_COUNT);
}

SERIALIZE_STATUS SerializeWriter::WriteFixedBytes(SerializeFieldId field, const std::uint8_t* bytes, std::uint32_t byteCount) {
    return CommitField(field, SERIALIZE_TYPE_TAG::FixedBytes, bytes, byteCount);
}

SerializeSnapshot SerializeWriter::Snapshot() const {
    return _snapshot;
}

SERIALIZE_STATUS SerializeWriter::CommitField(
    SerializeFieldId field,
    SERIALIZE_TYPE_TAG type,
    const std::uint8_t* payload,
    std::uint32_t byteCount) {
    if (!_hasStream) {
        return RecordFailure(SERIALIZE_STATUS::InvalidHeader);
    }

    if (!_hasActiveRecord) {
        return RecordFailure(SERIALIZE_STATUS::InvalidHeader);
    }

    if (!field.IsValid()) {
        return RecordFailure(SERIALIZE_STATUS::InvalidHeader);
    }

    if (byteCount > MAX_FIELD_PAYLOAD_BYTE_COUNT) {
        return RecordFailure(SERIALIZE_STATUS::FieldPayloadTooLarge);
    }

    if (byteCount > 0U && payload == nullptr) {
        return RecordFailure(SERIALIZE_STATUS::BufferTooSmall);
    }

    if (_snapshot.FieldCount >= MAX_FIELDS_PER_STREAM) {
        return RecordFailure(SERIALIZE_STATUS::FieldCapacityExceeded);
    }

    if (_currentRecordFieldCount >= MAX_FIELDS_PER_RECORD) {
        return RecordFailure(SERIALIZE_STATUS::FieldCapacityExceeded);
    }

    if (HasFieldInCurrentRecord(field)) {
        return RecordFailure(SERIALIZE_STATUS::DuplicateField);
    }

    if (!CanCommitBytes(FIELD_HEADER_BYTE_COUNT + byteCount)) {
        return RecordFailure(SERIALIZE_STATUS::BufferTooSmall);
    }

    const std::uint32_t fieldOffset = _snapshot.CommittedByteCount;
    WriteUInt32At(fieldOffset, field.Value);
    WriteUInt32At(fieldOffset + sizeof(std::uint32_t), static_cast<std::uint32_t>(type));
    WriteUInt32At(fieldOffset + (sizeof(std::uint32_t) * 2U), byteCount);
    CopyPayload(fieldOffset + FIELD_HEADER_BYTE_COUNT, payload, byteCount);
    _snapshot.CommittedByteCount += FIELD_HEADER_BYTE_COUNT + byteCount;
    _currentRecordFields[_currentRecordFieldCount] = field;
    ++_currentRecordFieldCount;
    ++_snapshot.FieldCount;
    WriteUInt32At(_activeRecordOffset + sizeof(std::uint32_t), _currentRecordFieldCount);
    RecordSuccess();
    return SERIALIZE_STATUS::Success;
}

SERIALIZE_STATUS SerializeWriter::RecordFailure(SERIALIZE_STATUS status) {
    ++_snapshot.FailedOperationCount;
    _snapshot.LastStatus = status;
    return status;
}

void SerializeWriter::RecordSuccess() {
    ++_snapshot.AcceptedOperationCount;
    _snapshot.LastStatus = SERIALIZE_STATUS::Success;
}

bool SerializeWriter::CanCommitBytes(std::uint32_t byteCount) const {
    if (_buffer == nullptr) {
        return false;
    }

    if (_snapshot.CommittedByteCount > _capacity) {
        return false;
    }

    return byteCount <= (_capacity - _snapshot.CommittedByteCount);
}

bool SerializeWriter::HasFieldInCurrentRecord(SerializeFieldId field) const {
    std::uint32_t index = 0U;
    for (const SerializeFieldId& currentField : _currentRecordFields) {
        if (index >= _currentRecordFieldCount) {
            return false;
        }

        if (currentField.Value == field.Value) {
            return true;
        }

        ++index;
    }

    return false;
}

void SerializeWriter::WriteUInt16At(std::uint32_t offset, std::uint16_t value) {
    _buffer[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    _buffer[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void SerializeWriter::WriteUInt32At(std::uint32_t offset, std::uint32_t value) {
    EncodeUInt32(_buffer + offset, value);
}

void SerializeWriter::CopyPayload(std::uint32_t offset, const std::uint8_t* payload, std::uint32_t byteCount) {
    std::uint32_t index = 0U;
    while (index < byteCount) {
        _buffer[offset + index] = payload[index];
        ++index;
    }
}
}
