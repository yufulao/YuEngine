// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceLoadCommitStatus.h

#pragma once

namespace yuengine::resource {
enum class ResourceLoadCommitStatus {
    Success,
    InvalidArgument,
    InvalidHandle,
    GenerationMismatch,
    TypeMismatch,
    DuplicateCommitId,
    InvalidTransition,
    CapacityExceeded
};
}
