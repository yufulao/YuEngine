#pragma once

namespace yuengine::serialize {
enum class SERIALIZE_STATUS {
    Success,
    BufferTooSmall,
    RecordCapacityExceeded,
    FieldCapacityExceeded,
    FieldPayloadTooLarge,
    InvalidHeader,
    UnsupportedVersion,
    TruncatedStream,
    UnknownTypeTag,
    TypeMismatch,
    DuplicateField,
    MalformedFieldLength,
    FieldNotFound
};
}
