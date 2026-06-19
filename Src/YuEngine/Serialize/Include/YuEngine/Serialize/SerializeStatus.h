// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Include/YuEngine/Serialize/SerializeStatus.h

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
    FieldNotFound,
    InvalidArgument
};
}
