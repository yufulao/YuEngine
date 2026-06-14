#include "yuengine/serialize/serialize_reader.h"

#include <array>

#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/serialize/serialize_constants.h"

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::serialize {
namespace {
constexpr std::uint32_t TYPE_TAG_TABLE_SIZE = 6U;
constexpr std::uint32_t TYPE_TAG_RESERVED_INDEX = 0U;
constexpr std::array<std::uint32_t, TYPE_TAG_TABLE_SIZE> TYPE_TAG_PAYLOAD_BYTE_COUNTS{
    0U,
    UINT32_PAYLOAD_BYTE_COUNT,
    INT32_PAYLOAD_BYTE_COUNT,
    UINT64_PAYLOAD_BYTE_COUNT,
    INT64_PAYLOAD_BYTE_COUNT,
    0U};
static_assert(static_cast<std::uint32_t>(SerializeTypeTag::FixedBytes) + 1U == TYPE_TAG_TABLE_SIZE);

std::uint32_t ClampByteCount(std::uint32_t byteCount) {
    if (byteCount > MAX_STREAM_BYTE_COUNT) {
        return MAX_STREAM_BYTE_COUNT;
    }

    return byteCount;
}

std::uint32_t ExpectedPayloadByteCount(SerializeTypeTag type) {
    const std::uint32_t typeValue = static_cast<std::uint32_t>(type);
    if (typeValue >= TYPE_TAG_TABLE_SIZE) {
        return 0U;
    }

    return TYPE_TAG_PAYLOAD_BYTE_COUNTS[typeValue];
}
}

SerializeReader::SerializeReader(const std::uint8_t* buffer, std::uint32_t byteCount)
    : _buffer(buffer),
      _byteCount(ClampByteCount(byteCount)),
      _snapshot{
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          SerializeStatus::Success},
      _isOpen(false) {
}

SerializeStatus SerializeReader::OpenStream() {
    _isOpen = false;

    if (!CanReadBytes(0U, STREAM_HEADER_BYTE_COUNT)) {
        return RecordFailure(SerializeStatus::TruncatedStream);
    }

    if (ReadUInt32At(STREAM_MAGIC_OFFSET) != STREAM_MAGIC) {
        return RecordFailure(SerializeStatus::InvalidHeader);
    }

    const std::uint16_t majorVersion = ReadUInt16At(STREAM_MAJOR_VERSION_OFFSET);
    if (majorVersion != STREAM_MAJOR_VERSION) {
        return RecordFailure(SerializeStatus::UnsupportedVersion);
    }

    if (ReadUInt32At(STREAM_FLAGS_OFFSET) != STREAM_FLAGS) {
        return RecordFailure(SerializeStatus::InvalidHeader);
    }

    std::uint32_t committedByteCount = 0U;
    std::uint32_t recordCount = 0U;
    std::uint32_t fieldCount = 0U;
    const SerializeStatus validationStatus = ValidateStream(committedByteCount, recordCount, fieldCount);
    if (validationStatus != SerializeStatus::Success) {
        return RecordFailure(validationStatus);
    }

    _snapshot.MajorVersion = majorVersion;
    _snapshot.MinorVersion = ReadUInt16At(STREAM_MINOR_VERSION_OFFSET);
    _snapshot.CommittedByteCount = committedByteCount;
    _snapshot.RecordCount = recordCount;
    _snapshot.FieldCount = fieldCount;
    _isOpen = true;
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::ReadUInt32(SerializeRecordId record, SerializeFieldId field, std::uint32_t& outValue) {
    FieldLocation location;
    const SerializeStatus findStatus = FindField(record, field, location);
    if (findStatus != SerializeStatus::Success) {
        return RecordFailure(findStatus);
    }

    if (location.Type != SerializeTypeTag::UInt32) {
        return RecordFailure(SerializeStatus::TypeMismatch);
    }

    if (location.PayloadByteCount != UINT32_PAYLOAD_BYTE_COUNT) {
        return RecordFailure(SerializeStatus::MalformedFieldLength);
    }

    outValue = ReadUInt32At(location.PayloadOffset);
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::ReadInt32(SerializeRecordId record, SerializeFieldId field, std::int32_t& outValue) {
    std::uint32_t rawValue = 0U;
    FieldLocation location;
    const SerializeStatus findStatus = FindField(record, field, location);
    if (findStatus != SerializeStatus::Success) {
        return RecordFailure(findStatus);
    }

    if (location.Type != SerializeTypeTag::Int32) {
        return RecordFailure(SerializeStatus::TypeMismatch);
    }

    if (location.PayloadByteCount != INT32_PAYLOAD_BYTE_COUNT) {
        return RecordFailure(SerializeStatus::MalformedFieldLength);
    }

    rawValue = ReadUInt32At(location.PayloadOffset);
    outValue = static_cast<std::int32_t>(rawValue);
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::ReadUInt64(SerializeRecordId record, SerializeFieldId field, std::uint64_t& outValue) {
    FieldLocation location;
    const SerializeStatus findStatus = FindField(record, field, location);
    if (findStatus != SerializeStatus::Success) {
        return RecordFailure(findStatus);
    }

    if (location.Type != SerializeTypeTag::UInt64) {
        return RecordFailure(SerializeStatus::TypeMismatch);
    }

    if (location.PayloadByteCount != UINT64_PAYLOAD_BYTE_COUNT) {
        return RecordFailure(SerializeStatus::MalformedFieldLength);
    }

    outValue = ReadUInt64At(location.PayloadOffset);
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::ReadInt64(SerializeRecordId record, SerializeFieldId field, std::int64_t& outValue) {
    std::uint64_t rawValue = 0U;
    FieldLocation location;
    const SerializeStatus findStatus = FindField(record, field, location);
    if (findStatus != SerializeStatus::Success) {
        return RecordFailure(findStatus);
    }

    if (location.Type != SerializeTypeTag::Int64) {
        return RecordFailure(SerializeStatus::TypeMismatch);
    }

    if (location.PayloadByteCount != INT64_PAYLOAD_BYTE_COUNT) {
        return RecordFailure(SerializeStatus::MalformedFieldLength);
    }

    rawValue = ReadUInt64At(location.PayloadOffset);
    outValue = static_cast<std::int64_t>(rawValue);
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::ReadFixedBytes(
    SerializeRecordId record,
    SerializeFieldId field,
    std::uint8_t* outBytes,
    std::uint32_t outCapacity,
    std::uint32_t& outByteCount) {
    FieldLocation location;
    const SerializeStatus findStatus = FindField(record, field, location);
    if (findStatus != SerializeStatus::Success) {
        return RecordFailure(findStatus);
    }

    if (location.Type != SerializeTypeTag::FixedBytes) {
        return RecordFailure(SerializeStatus::TypeMismatch);
    }

    if (location.PayloadByteCount > outCapacity) {
        return RecordFailure(SerializeStatus::BufferTooSmall);
    }

    if (location.PayloadByteCount > 0U && outBytes == nullptr) {
        return RecordFailure(SerializeStatus::BufferTooSmall);
    }

    std::uint32_t index = 0U;
    while (index < location.PayloadByteCount) {
        outBytes[index] = _buffer[location.PayloadOffset + index];
        ++index;
    }

    outByteCount = location.PayloadByteCount;
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeSnapshot SerializeReader::Snapshot() const {
    return _snapshot;
}

SerializeStatus SerializeReader::ValidateStream(
    std::uint32_t& outCommittedByteCount,
    std::uint32_t& outRecordCount,
    std::uint32_t& outFieldCount) const {
    const std::uint32_t recordCount = ReadUInt32At(STREAM_RECORD_COUNT_OFFSET);
    if (recordCount > MAX_RECORDS_PER_STREAM) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    std::uint32_t totalFieldCount = 0U;
    std::uint32_t recordIndex = 0U;
    while (recordIndex < recordCount) {
        if (!CanReadBytes(offset, RECORD_HEADER_BYTE_COUNT)) {
            return SerializeStatus::TruncatedStream;
        }

        const SerializeRecordId record{ReadUInt32At(offset)};
        if (!record.IsValid()) {
            return SerializeStatus::InvalidHeader;
        }

        const std::uint32_t fieldCount = ReadUInt32At(offset + sizeof(std::uint32_t));
        if (fieldCount > MAX_FIELDS_PER_RECORD) {
            return SerializeStatus::FieldCapacityExceeded;
        }

        if (totalFieldCount + fieldCount > MAX_FIELDS_PER_STREAM) {
            return SerializeStatus::FieldCapacityExceeded;
        }

        offset += RECORD_HEADER_BYTE_COUNT;
        std::array<SerializeFieldId, MAX_FIELDS_PER_RECORD> fields{};
        std::uint32_t recordFieldIndex = 0U;
        while (recordFieldIndex < fieldCount) {
            if (!CanReadBytes(offset, FIELD_HEADER_BYTE_COUNT)) {
                return SerializeStatus::TruncatedStream;
            }

            const SerializeFieldId field{ReadUInt32At(offset)};
            if (!field.IsValid()) {
                return SerializeStatus::InvalidHeader;
            }

            const std::uint32_t typeValue = ReadUInt32At(offset + sizeof(std::uint32_t));
            const std::uint32_t payloadByteCount = ReadUInt32At(offset + (sizeof(std::uint32_t) * 2U));
            if (!IsKnownTypeTag(typeValue)) {
                return SerializeStatus::UnknownTypeTag;
            }

            const SerializeTypeTag type = static_cast<SerializeTypeTag>(typeValue);
            if (payloadByteCount > MAX_FIELD_PAYLOAD_BYTE_COUNT) {
                return SerializeStatus::FieldPayloadTooLarge;
            }

            if (type != SerializeTypeTag::FixedBytes && payloadByteCount != ExpectedPayloadByteCount(type)) {
                return SerializeStatus::MalformedFieldLength;
            }

            const std::uint32_t payloadOffset = offset + FIELD_HEADER_BYTE_COUNT;
            if (!CanReadBytes(payloadOffset, payloadByteCount)) {
                return SerializeStatus::MalformedFieldLength;
            }

            if (IsDuplicateField(field, fields.data(), recordFieldIndex)) {
                return SerializeStatus::DuplicateField;
            }

            fields[recordFieldIndex] = field;
            offset = payloadOffset + payloadByteCount;
            ++recordFieldIndex;
        }

        totalFieldCount += fieldCount;
        ++recordIndex;
    }

    outCommittedByteCount = offset;
    outRecordCount = recordCount;
    outFieldCount = totalFieldCount;
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::FindField(SerializeRecordId record, SerializeFieldId field, FieldLocation& outLocation) const {
    if (!_isOpen) {
        return SerializeStatus::InvalidHeader;
    }

    if (!record.IsValid() || !field.IsValid()) {
        return SerializeStatus::InvalidHeader;
    }

    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    std::uint32_t recordIndex = 0U;
    while (recordIndex < _snapshot.RecordCount) {
        const SerializeRecordId currentRecord{ReadUInt32At(offset)};
        const std::uint32_t fieldCount = ReadUInt32At(offset + sizeof(std::uint32_t));
        offset += RECORD_HEADER_BYTE_COUNT;
        std::uint32_t fieldIndex = 0U;
        while (fieldIndex < fieldCount) {
            const SerializeFieldId currentField{ReadUInt32At(offset)};
            const SerializeTypeTag type = static_cast<SerializeTypeTag>(ReadUInt32At(offset + sizeof(std::uint32_t)));
            const std::uint32_t payloadByteCount = ReadUInt32At(offset + (sizeof(std::uint32_t) * 2U));
            const std::uint32_t payloadOffset = offset + FIELD_HEADER_BYTE_COUNT;
            if (currentRecord.Value == record.Value && currentField.Value == field.Value) {
                outLocation.Type = type;
                outLocation.PayloadOffset = payloadOffset;
                outLocation.PayloadByteCount = payloadByteCount;
                outLocation.Found = true;
                return SerializeStatus::Success;
            }

            offset = payloadOffset + payloadByteCount;
            ++fieldIndex;
        }

        ++recordIndex;
    }

    if (!outLocation.Found) {
        return SerializeStatus::FieldNotFound;
    }

    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::RecordFailure(SerializeStatus status) {
    ++_snapshot.FailedOperationCount;
    _snapshot.LastStatus = status;
    return status;
}

void SerializeReader::RecordSuccess() {
    ++_snapshot.AcceptedOperationCount;
    _snapshot.LastStatus = SerializeStatus::Success;
}

bool SerializeReader::CanReadBytes(std::uint32_t offset, std::uint32_t byteCount) const {
    if (_buffer == nullptr) {
        return false;
    }

    if (offset > _byteCount) {
        return false;
    }

    return byteCount <= (_byteCount - offset);
}

bool SerializeReader::IsKnownTypeTag(std::uint32_t value) const {
    if (value == TYPE_TAG_RESERVED_INDEX) {
        return false;
    }

    return value < TYPE_TAG_TABLE_SIZE;
}

bool SerializeReader::IsDuplicateField(SerializeFieldId field, const SerializeFieldId* fields, std::uint32_t fieldCount) const {
    std::uint32_t index = 0U;
    while (index < fieldCount) {
        if (fields[index].Value == field.Value) {
            return true;
        }

        ++index;
    }

    return false;
}

std::uint16_t SerializeReader::ReadUInt16At(std::uint32_t offset) const {
    const std::uint16_t byte0 = static_cast<std::uint16_t>(_buffer[offset]);
    const std::uint16_t byte1 = static_cast<std::uint16_t>(_buffer[offset + 1U]);
    return static_cast<std::uint16_t>(byte0 | static_cast<std::uint16_t>(byte1 << 8U));
}

std::uint32_t SerializeReader::ReadUInt32At(std::uint32_t offset) const {
    const std::uint32_t byte0 = static_cast<std::uint32_t>(_buffer[offset]);
    const std::uint32_t byte1 = static_cast<std::uint32_t>(_buffer[offset + 1U]);
    const std::uint32_t byte2 = static_cast<std::uint32_t>(_buffer[offset + 2U]);
    const std::uint32_t byte3 = static_cast<std::uint32_t>(_buffer[offset + 3U]);
    return byte0 | (byte1 << 8U) | (byte2 << 16U) | (byte3 << 24U);
}

std::uint64_t SerializeReader::ReadUInt64At(std::uint32_t offset) const {
    const std::uint64_t byte0 = static_cast<std::uint64_t>(_buffer[offset]);
    const std::uint64_t byte1 = static_cast<std::uint64_t>(_buffer[offset + 1U]);
    const std::uint64_t byte2 = static_cast<std::uint64_t>(_buffer[offset + 2U]);
    const std::uint64_t byte3 = static_cast<std::uint64_t>(_buffer[offset + 3U]);
    const std::uint64_t byte4 = static_cast<std::uint64_t>(_buffer[offset + 4U]);
    const std::uint64_t byte5 = static_cast<std::uint64_t>(_buffer[offset + 5U]);
    const std::uint64_t byte6 = static_cast<std::uint64_t>(_buffer[offset + 6U]);
    const std::uint64_t byte7 = static_cast<std::uint64_t>(_buffer[offset + 7U]);
    return byte0 | (byte1 << 8U) | (byte2 << 16U) | (byte3 << 24U) | (byte4 << 32U) | (byte5 << 40U) |
           (byte6 << 48U) | (byte7 << 56U);
}
}
