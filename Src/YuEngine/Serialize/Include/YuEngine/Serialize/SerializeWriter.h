#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeFieldId.h"
#include "YuEngine/Serialize/SerializeRecordId.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/Serialize/SerializeTypeTag.h"

namespace yuengine::serialize {
class SerializeWriter final {
public:
    SerializeWriter(std::uint8_t* buffer, std::uint32_t capacity);

    SerializeStatus BeginStream();
    SerializeStatus BeginRecord(SerializeRecordId record);
    SerializeStatus WriteUInt32(SerializeFieldId field, std::uint32_t value);
    SerializeStatus WriteInt32(SerializeFieldId field, std::int32_t value);
    SerializeStatus WriteUInt64(SerializeFieldId field, std::uint64_t value);
    SerializeStatus WriteInt64(SerializeFieldId field, std::int64_t value);
    SerializeStatus WriteFixedBytes(SerializeFieldId field, const std::uint8_t* bytes, std::uint32_t byteCount);
    SerializeSnapshot Snapshot() const;

private:
    SerializeStatus CommitField(
        SerializeFieldId field,
        SerializeTypeTag type,
        const std::uint8_t* payload,
        std::uint32_t byteCount);
    SerializeStatus RecordFailure(SerializeStatus status);
    void RecordSuccess();
    bool CanCommitBytes(std::uint32_t byteCount) const;
    bool HasFieldInCurrentRecord(SerializeFieldId field) const;
    void WriteUInt16At(std::uint32_t offset, std::uint16_t value);
    void WriteUInt32At(std::uint32_t offset, std::uint32_t value);
    void CopyPayload(std::uint32_t offset, const std::uint8_t* payload, std::uint32_t byteCount);

    std::uint8_t* buffer_;
    std::uint32_t capacity_;
    std::uint32_t active_record_offset_;
    std::uint32_t current_record_field_count_;
    std::array<SerializeFieldId, MAX_FIELDS_PER_RECORD> current_record_fields_;
    SerializeSnapshot snapshot_;
    bool has_stream_;
    bool has_active_record_;
};
}
