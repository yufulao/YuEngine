// Module: YuEngine Serialize
// File: Src/YuEngine/Serialize/Include/YuEngine/Serialize/SerializeReader.h

#pragma once

#include <cstdint>

#include "YuEngine/Serialize/SerializeFieldId.h"
#include "YuEngine/Serialize/SerializeRecordId.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/Serialize/FieldLocation.h"

namespace yuengine::serialize {
class SerializeReader final {
public:
    /**
     * @comment Constructs a SerializeReader instance.
     * @param buffer Input buffer.
     * @param byte_count Input byte count.
     */
    SerializeReader(const std::uint8_t* buffer, std::uint32_t byte_count);

    /**
     * @comment Opens a serialized stream.
     * @return Explicit operation status.
     */
    SerializeStatus OpenStream();
    /**
     * @comment Reads uint32.
     * @param record Input record.
     * @param field Input field.
     * @param out_value Output value written on success.
     * @return Explicit operation status.
     */
    SerializeStatus ReadUInt32(SerializeRecordId record, SerializeFieldId field, std::uint32_t& out_value);
    /**
     * @comment Reads int32.
     * @param record Input record.
     * @param field Input field.
     * @param out_value Output value written on success.
     * @return Explicit operation status.
     */
    SerializeStatus ReadInt32(SerializeRecordId record, SerializeFieldId field, std::int32_t& out_value);
    /**
     * @comment Reads uint64.
     * @param record Input record.
     * @param field Input field.
     * @param out_value Output value written on success.
     * @return Explicit operation status.
     */
    SerializeStatus ReadUInt64(SerializeRecordId record, SerializeFieldId field, std::uint64_t& out_value);
    /**
     * @comment Reads int64.
     * @param record Input record.
     * @param field Input field.
     * @param out_value Output value written on success.
     * @return Explicit operation status.
     */
    SerializeStatus ReadInt64(SerializeRecordId record, SerializeFieldId field, std::int64_t& out_value);
    /**
     * @comment Reads fixed bytes.
     * @param record Input record.
     * @param field Input field.
     * @param out_bytes Output bytes written on success.
     * @param out_capacity Output capacity written on success.
     * @param out_byte_count Output byte count written on success.
     * @return Explicit operation status.
     */
    SerializeStatus ReadFixedBytes(
        SerializeRecordId record,
        SerializeFieldId field,
        std::uint8_t* out_bytes,
        std::uint32_t out_capacity,
        std::uint32_t& out_byte_count);
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    SerializeSnapshot Snapshot() const;

private:
    SerializeStatus ValidateStream(std::uint32_t& out_committed_byte_count, std::uint32_t& out_record_count, std::uint32_t& out_field_count) const;
    SerializeStatus FindField(SerializeRecordId record, SerializeFieldId field, FieldLocation& out_location) const;
    SerializeStatus RecordFailure(SerializeStatus status);
    void RecordSuccess();
    bool CanReadBytes(std::uint32_t offset, std::uint32_t byte_count) const;
    bool IsKnownTypeTag(std::uint32_t value) const;
    bool IsDuplicateField(SerializeFieldId field, const SerializeFieldId* fields, std::uint32_t field_count) const;
    std::uint16_t ReadUInt16At(std::uint32_t offset) const;
    std::uint32_t ReadUInt32At(std::uint32_t offset) const;
    std::uint64_t ReadUInt64At(std::uint32_t offset) const;

    const std::uint8_t* buffer_;
    std::uint32_t byte_count_;
    SerializeSnapshot snapshot_;
    bool is_open_;
};
}
