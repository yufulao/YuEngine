// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Src/SerializeReader.cpp

#include "YuEngine/Serialize/SerializeReader.h"

#include <array>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeConstants.h"

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

std::uint32_t ClampByteCount(std::uint32_t byte_count) {
    if (byte_count > MAX_STREAM_BYTE_COUNT) {
        return MAX_STREAM_BYTE_COUNT;
    }

    return byte_count;
}

std::uint32_t ExpectedPayloadByteCount(SerializeTypeTag type) {
    const std::uint32_t type_value = static_cast<std::uint32_t>(type);
    if (type_value >= TYPE_TAG_TABLE_SIZE) {
        return 0U;
    }

    return TYPE_TAG_PAYLOAD_BYTE_COUNTS[type_value];
}
}

SerializeReader::SerializeReader(const std::uint8_t* buffer, std::uint32_t byte_count)
    : buffer_(buffer),
      byte_count_(ClampByteCount(byte_count)),
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
      is_open_(false) {
}

SerializeStatus SerializeReader::OpenStream() {
    is_open_ = false;

    if (!CanReadBytes(0U, STREAM_HEADER_BYTE_COUNT)) {
        return RecordFailure(SerializeStatus::TruncatedStream);
    }

    if (ReadUInt32At(STREAM_MAGIC_OFFSET) != STREAM_MAGIC) {
        return RecordFailure(SerializeStatus::InvalidHeader);
    }

    const std::uint16_t major_version = ReadUInt16At(STREAM_MAJOR_VERSION_OFFSET);
    if (major_version != STREAM_MAJOR_VERSION) {
        return RecordFailure(SerializeStatus::UnsupportedVersion);
    }

    if (ReadUInt32At(STREAM_FLAGS_OFFSET) != STREAM_FLAGS) {
        return RecordFailure(SerializeStatus::InvalidHeader);
    }

    std::uint32_t committed_byte_count = 0U;
    std::uint32_t record_count = 0U;
    std::uint32_t field_count = 0U;
    const SerializeStatus validation_status = ValidateStream(committed_byte_count, record_count, field_count);
    if (validation_status != SerializeStatus::Success) {
        return RecordFailure(validation_status);
    }

    snapshot_.major_version = major_version;
    snapshot_.minor_version = ReadUInt16At(STREAM_MINOR_VERSION_OFFSET);
    snapshot_.committed_byte_count = committed_byte_count;
    snapshot_.record_count = record_count;
    snapshot_.field_count = field_count;
    is_open_ = true;
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::ReadUInt32(SerializeRecordId record, SerializeFieldId field, std::uint32_t& out_value) {
    FieldLocation location;
    const SerializeStatus find_status = FindField(record, field, location);
    if (find_status != SerializeStatus::Success) {
        return RecordFailure(find_status);
    }

    if (location.type != SerializeTypeTag::UInt32) {
        return RecordFailure(SerializeStatus::TypeMismatch);
    }

    if (location.payload_byte_count != UINT32_PAYLOAD_BYTE_COUNT) {
        return RecordFailure(SerializeStatus::MalformedFieldLength);
    }

    out_value = ReadUInt32At(location.payload_offset);
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::ReadInt32(SerializeRecordId record, SerializeFieldId field, std::int32_t& out_value) {
    std::uint32_t raw_value = 0U;
    FieldLocation location;
    const SerializeStatus find_status = FindField(record, field, location);
    if (find_status != SerializeStatus::Success) {
        return RecordFailure(find_status);
    }

    if (location.type != SerializeTypeTag::Int32) {
        return RecordFailure(SerializeStatus::TypeMismatch);
    }

    if (location.payload_byte_count != INT32_PAYLOAD_BYTE_COUNT) {
        return RecordFailure(SerializeStatus::MalformedFieldLength);
    }

    raw_value = ReadUInt32At(location.payload_offset);
    out_value = static_cast<std::int32_t>(raw_value);
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::ReadUInt64(SerializeRecordId record, SerializeFieldId field, std::uint64_t& out_value) {
    FieldLocation location;
    const SerializeStatus find_status = FindField(record, field, location);
    if (find_status != SerializeStatus::Success) {
        return RecordFailure(find_status);
    }

    if (location.type != SerializeTypeTag::UInt64) {
        return RecordFailure(SerializeStatus::TypeMismatch);
    }

    if (location.payload_byte_count != UINT64_PAYLOAD_BYTE_COUNT) {
        return RecordFailure(SerializeStatus::MalformedFieldLength);
    }

    out_value = ReadUInt64At(location.payload_offset);
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::ReadInt64(SerializeRecordId record, SerializeFieldId field, std::int64_t& out_value) {
    std::uint64_t raw_value = 0U;
    FieldLocation location;
    const SerializeStatus find_status = FindField(record, field, location);
    if (find_status != SerializeStatus::Success) {
        return RecordFailure(find_status);
    }

    if (location.type != SerializeTypeTag::Int64) {
        return RecordFailure(SerializeStatus::TypeMismatch);
    }

    if (location.payload_byte_count != INT64_PAYLOAD_BYTE_COUNT) {
        return RecordFailure(SerializeStatus::MalformedFieldLength);
    }

    raw_value = ReadUInt64At(location.payload_offset);
    out_value = static_cast<std::int64_t>(raw_value);
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::ReadFixedBytes(
    SerializeRecordId record,
    SerializeFieldId field,
    std::uint8_t* out_bytes,
    std::uint32_t out_capacity,
    std::uint32_t& out_byte_count) {
    FieldLocation location;
    const SerializeStatus find_status = FindField(record, field, location);
    if (find_status != SerializeStatus::Success) {
        return RecordFailure(find_status);
    }

    if (location.type != SerializeTypeTag::FixedBytes) {
        return RecordFailure(SerializeStatus::TypeMismatch);
    }

    if (location.payload_byte_count > out_capacity) {
        return RecordFailure(SerializeStatus::BufferTooSmall);
    }

    if (location.payload_byte_count > 0U && out_bytes == nullptr) {
        return RecordFailure(SerializeStatus::BufferTooSmall);
    }

    std::uint32_t index = 0U;
    while (index < location.payload_byte_count) {
        out_bytes[index] = buffer_[location.payload_offset + index];
        ++index;
    }

    out_byte_count = location.payload_byte_count;
    RecordSuccess();
    return SerializeStatus::Success;
}

SerializeSnapshot SerializeReader::Snapshot() const {
    return snapshot_;
}

SerializeStatus SerializeReader::ValidateStream(
    std::uint32_t& out_committed_byte_count,
    std::uint32_t& out_record_count,
    std::uint32_t& out_field_count) const {
    const std::uint32_t record_count = ReadUInt32At(STREAM_RECORD_COUNT_OFFSET);
    if (record_count > MAX_RECORDS_PER_STREAM) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    std::uint32_t total_field_count = 0U;
    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        if (!CanReadBytes(offset, RECORD_HEADER_BYTE_COUNT)) {
            return SerializeStatus::TruncatedStream;
        }

        const SerializeRecordId record{ReadUInt32At(offset)};
        if (!record.IsValid()) {
            return SerializeStatus::InvalidHeader;
        }

        const std::uint32_t field_count = ReadUInt32At(offset + sizeof(std::uint32_t));
        if (field_count > MAX_FIELDS_PER_RECORD) {
            return SerializeStatus::FieldCapacityExceeded;
        }

        if (total_field_count + field_count > MAX_FIELDS_PER_STREAM) {
            return SerializeStatus::FieldCapacityExceeded;
        }

        offset += RECORD_HEADER_BYTE_COUNT;
        std::array<SerializeFieldId, MAX_FIELDS_PER_RECORD> fields{};
        std::uint32_t record_field_index = 0U;
        while (record_field_index < field_count) {
            if (!CanReadBytes(offset, FIELD_HEADER_BYTE_COUNT)) {
                return SerializeStatus::TruncatedStream;
            }

            const SerializeFieldId field{ReadUInt32At(offset)};
            if (!field.IsValid()) {
                return SerializeStatus::InvalidHeader;
            }

            const std::uint32_t type_value = ReadUInt32At(offset + sizeof(std::uint32_t));
            const std::uint32_t payload_byte_count = ReadUInt32At(offset + (sizeof(std::uint32_t) * 2U));
            if (!IsKnownTypeTag(type_value)) {
                return SerializeStatus::UnknownTypeTag;
            }

            const SerializeTypeTag type = static_cast<SerializeTypeTag>(type_value);
            if (payload_byte_count > MAX_FIELD_PAYLOAD_BYTE_COUNT) {
                return SerializeStatus::FieldPayloadTooLarge;
            }

            if (type != SerializeTypeTag::FixedBytes && payload_byte_count != ExpectedPayloadByteCount(type)) {
                return SerializeStatus::MalformedFieldLength;
            }

            const std::uint32_t payload_offset = offset + FIELD_HEADER_BYTE_COUNT;
            if (!CanReadBytes(payload_offset, payload_byte_count)) {
                return SerializeStatus::MalformedFieldLength;
            }

            if (IsDuplicateField(field, fields.data(), record_field_index)) {
                return SerializeStatus::DuplicateField;
            }

            fields[record_field_index] = field;
            offset = payload_offset + payload_byte_count;
            ++record_field_index;
        }

        total_field_count += field_count;
        ++record_index;
    }

    out_committed_byte_count = offset;
    out_record_count = record_count;
    out_field_count = total_field_count;
    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::FindField(SerializeRecordId record, SerializeFieldId field, FieldLocation& out_location) const {
    if (!is_open_) {
        return SerializeStatus::InvalidHeader;
    }

    if (!record.IsValid() || !field.IsValid()) {
        return SerializeStatus::InvalidHeader;
    }

    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    std::uint32_t record_index = 0U;
    while (record_index < snapshot_.record_count) {
        const SerializeRecordId current_record{ReadUInt32At(offset)};
        const std::uint32_t field_count = ReadUInt32At(offset + sizeof(std::uint32_t));
        offset += RECORD_HEADER_BYTE_COUNT;
        std::uint32_t field_index = 0U;
        while (field_index < field_count) {
            const SerializeFieldId current_field{ReadUInt32At(offset)};
            const SerializeTypeTag type = static_cast<SerializeTypeTag>(ReadUInt32At(offset + sizeof(std::uint32_t)));
            const std::uint32_t payload_byte_count = ReadUInt32At(offset + (sizeof(std::uint32_t) * 2U));
            const std::uint32_t payload_offset = offset + FIELD_HEADER_BYTE_COUNT;
            if (current_record.value == record.value && current_field.value == field.value) {
                out_location.type = type;
                out_location.payload_offset = payload_offset;
                out_location.payload_byte_count = payload_byte_count;
                out_location.found = true;
                return SerializeStatus::Success;
            }

            offset = payload_offset + payload_byte_count;
            ++field_index;
        }

        ++record_index;
    }

    if (!out_location.found) {
        return SerializeStatus::FieldNotFound;
    }

    return SerializeStatus::Success;
}

SerializeStatus SerializeReader::RecordFailure(SerializeStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

void SerializeReader::RecordSuccess() {
    ++snapshot_.accepted_operation_count;
    snapshot_.last_status = SerializeStatus::Success;
}

bool SerializeReader::CanReadBytes(std::uint32_t offset, std::uint32_t byte_count) const {
    if (buffer_ == nullptr) {
        return false;
    }

    if (offset > byte_count_) {
        return false;
    }

    return byte_count <= (byte_count_ - offset);
}

bool SerializeReader::IsKnownTypeTag(std::uint32_t value) const {
    if (value == TYPE_TAG_RESERVED_INDEX) {
        return false;
    }

    return value < TYPE_TAG_TABLE_SIZE;
}

bool SerializeReader::IsDuplicateField(SerializeFieldId field, const SerializeFieldId* fields, std::uint32_t field_count) const {
    std::uint32_t index = 0U;
    while (index < field_count) {
        if (fields[index].value == field.value) {
            return true;
        }

        ++index;
    }

    return false;
}

std::uint16_t SerializeReader::ReadUInt16At(std::uint32_t offset) const {
    const std::uint16_t byte0 = static_cast<std::uint16_t>(buffer_[offset]);
    const std::uint16_t byte1 = static_cast<std::uint16_t>(buffer_[offset + 1U]);
    return static_cast<std::uint16_t>(byte0 | static_cast<std::uint16_t>(byte1 << 8U));
}

std::uint32_t SerializeReader::ReadUInt32At(std::uint32_t offset) const {
    const std::uint32_t byte0 = static_cast<std::uint32_t>(buffer_[offset]);
    const std::uint32_t byte1 = static_cast<std::uint32_t>(buffer_[offset + 1U]);
    const std::uint32_t byte2 = static_cast<std::uint32_t>(buffer_[offset + 2U]);
    const std::uint32_t byte3 = static_cast<std::uint32_t>(buffer_[offset + 3U]);
    return byte0 | (byte1 << 8U) | (byte2 << 16U) | (byte3 << 24U);
}

std::uint64_t SerializeReader::ReadUInt64At(std::uint32_t offset) const {
    const std::uint64_t byte0 = static_cast<std::uint64_t>(buffer_[offset]);
    const std::uint64_t byte1 = static_cast<std::uint64_t>(buffer_[offset + 1U]);
    const std::uint64_t byte2 = static_cast<std::uint64_t>(buffer_[offset + 2U]);
    const std::uint64_t byte3 = static_cast<std::uint64_t>(buffer_[offset + 3U]);
    const std::uint64_t byte4 = static_cast<std::uint64_t>(buffer_[offset + 4U]);
    const std::uint64_t byte5 = static_cast<std::uint64_t>(buffer_[offset + 5U]);
    const std::uint64_t byte6 = static_cast<std::uint64_t>(buffer_[offset + 6U]);
    const std::uint64_t byte7 = static_cast<std::uint64_t>(buffer_[offset + 7U]);
    return byte0 | (byte1 << 8U) | (byte2 << 16U) | (byte3 << 24U) | (byte4 << 32U) | (byte5 << 40U) |
           (byte6 << 48U) | (byte7 << 56U);
}
}
