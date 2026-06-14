#pragma once

namespace yuengine::serialize {
enum class SerializeStatus {
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
