#pragma once

#include <array>
#include <cstdint>

#include "yuengine/serialize/SerializeConstants.h"
#include "yuengine/serialize/SerializeFieldId.h"
#include "yuengine/serialize/SerializeRecordId.h"
#include "yuengine/serialize/SerializeSnapshot.h"
#include "yuengine/serialize/SerializeStatus.h"
#include "yuengine/serialize/SerializeTypeTag.h"

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

    std::uint8_t* _buffer;
    std::uint32_t _capacity;
    std::uint32_t _activeRecordOffset;
    std::uint32_t _currentRecordFieldCount;
    std::array<SerializeFieldId, MAX_FIELDS_PER_RECORD> _currentRecordFields;
    SerializeSnapshot _snapshot;
    bool _hasStream;
    bool _hasActiveRecord;
};
}
