// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceResidencyState.h

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
