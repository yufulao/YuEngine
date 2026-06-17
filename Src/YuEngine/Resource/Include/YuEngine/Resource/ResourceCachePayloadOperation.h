// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceCachePayloadOperation.h

#pragma once

namespace yuengine::resource {
enum class ResourceCachePayloadOperation {
    None,
    ConfigureBudget,
    Store,
    Read,
    Release
};
}
