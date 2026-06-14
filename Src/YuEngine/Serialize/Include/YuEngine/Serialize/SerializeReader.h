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
    SerializeReader(const std::uint8_t* buffer, std::uint32_t byteCount);

    SerializeStatus OpenStream();
    SerializeStatus ReadUInt32(SerializeRecordId record, SerializeFieldId field, std::uint32_t& outValue);
    SerializeStatus ReadInt32(SerializeRecordId record, SerializeFieldId field, std::int32_t& outValue);
    SerializeStatus ReadUInt64(SerializeRecordId record, SerializeFieldId field, std::uint64_t& outValue);
    SerializeStatus ReadInt64(SerializeRecordId record, SerializeFieldId field, std::int64_t& outValue);
    SerializeStatus ReadFixedBytes(
        SerializeRecordId record,
        SerializeFieldId field,
        std::uint8_t* outBytes,
        std::uint32_t outCapacity,
        std::uint32_t& outByteCount);
    SerializeSnapshot Snapshot() const;

private:
    SerializeStatus ValidateStream(std::uint32_t& outCommittedByteCount, std::uint32_t& outRecordCount, std::uint32_t& outFieldCount) const;
    SerializeStatus FindField(SerializeRecordId record, SerializeFieldId field, FieldLocation& outLocation) const;
    SerializeStatus RecordFailure(SerializeStatus status);
    void RecordSuccess();
    bool CanReadBytes(std::uint32_t offset, std::uint32_t byteCount) const;
    bool IsKnownTypeTag(std::uint32_t value) const;
    bool IsDuplicateField(SerializeFieldId field, const SerializeFieldId* fields, std::uint32_t fieldCount) const;
    std::uint16_t ReadUInt16At(std::uint32_t offset) const;
    std::uint32_t ReadUInt32At(std::uint32_t offset) const;
    std::uint64_t ReadUInt64At(std::uint32_t offset) const;

    const std::uint8_t* _buffer;
    std::uint32_t _byteCount;
    SerializeSnapshot _snapshot;
    bool _isOpen;
};
}
