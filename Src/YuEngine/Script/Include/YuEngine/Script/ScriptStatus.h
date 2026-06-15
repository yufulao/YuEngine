// Module: YuEngine Script
// File: Src/YuEngine/Script/Include/YuEngine/Script/ScriptStatus.h

#pragma once

namespace yuengine::script {
enum class ScriptStatus {
    Success,
    InvalidCallId,
    DuplicateCallId,
    CapacityExceeded,
    NullNativeFunction,
    ArgumentCountMismatch,
    ArgumentTypeMismatch,
    ResultCountMismatch,
    ResultTypeMismatch,
    InvalidArgumentBuffer,
    InvalidResultBuffer,
    NativeCallFailed
};
}
