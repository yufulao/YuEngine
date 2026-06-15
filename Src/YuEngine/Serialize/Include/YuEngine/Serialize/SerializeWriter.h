// Module: YuEngine Serialize
// File: Src/YuEngine/Serialize/Include/YuEngine/Serialize/SerializeWriter.h

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
    /**
     * @comment Constructs a SerializeWriter instance.
     * @param buffer Input buffer.
     * @param capacity Input capacity.
     */
    SerializeWriter(std::uint8_t* buffer, std::uint32_t capacity);

    /**
     * @comment Begins a serialized stream.
     * @return Explicit operation status.
     */
    SerializeStatus BeginStream();
    /**
     * @comment Begins a serialized record.
     * @param record Input record.
     * @return Explicit operation status.
     */
    SerializeStatus BeginRecord(SerializeRecordId record);
    /**
     * @comment Writes uint32.
     * @param field Input field.
     * @param value Input value.
     * @return Explicit operation status.
     */
    SerializeStatus WriteUInt32(SerializeFieldId field, std::uint32_t value);
    /**
     * @comment Writes int32.
     * @param field Input field.
     * @param value Input value.
     * @return Explicit operation status.
     */
    SerializeStatus WriteInt32(SerializeFieldId field, std::int32_t value);
    /**
     * @comment Writes uint64.
     * @param field Input field.
     * @param value Input value.
     * @return Explicit operation status.
     */
    SerializeStatus WriteUInt64(SerializeFieldId field, std::uint64_t value);
    /**
     * @comment Writes int64.
     * @param field Input field.
     * @param value Input value.
     * @return Explicit operation status.
     */
    SerializeStatus WriteInt64(SerializeFieldId field, std::int64_t value);
    /**
     * @comment Writes fixed bytes.
     * @param field Input field.
     * @param bytes Input byte count or byte payload.
     * @param byte_count Input byte count.
     * @return Explicit operation status.
     */
    SerializeStatus WriteFixedBytes(SerializeFieldId field, const std::uint8_t* bytes, std::uint32_t byte_count);
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    SerializeSnapshot Snapshot() const;

private:
    SerializeStatus CommitField(
        SerializeFieldId field,
        SerializeTypeTag type,
        const std::uint8_t* payload,
        std::uint32_t byte_count);
    SerializeStatus RecordFailure(SerializeStatus status);
    void RecordSuccess();
    bool CanCommitBytes(std::uint32_t byte_count) const;
    bool HasFieldInCurrentRecord(SerializeFieldId field) const;
    void WriteUInt16At(std::uint32_t offset, std::uint16_t value);
    void WriteUInt32At(std::uint32_t offset, std::uint32_t value);
    void CopyPayload(std::uint32_t offset, const std::uint8_t* payload, std::uint32_t byte_count);

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
