// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceLoadCommitStatus.h

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
