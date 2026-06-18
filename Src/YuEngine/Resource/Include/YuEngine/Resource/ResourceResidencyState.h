// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceResidencyState.h

#pragma once

namespace yuengine::resource {
enum class ResourceResidencyState {
    Unloaded,
    Uploaded,
    Resident,
    Pinned,
    Evictable,
    Evicted,
    Failed
};
}
