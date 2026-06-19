// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetDependencyEdge.h

#pragma once

#include "YuEngine/Asset/AssetHandle.h"

namespace yuengine::asset {
struct AssetDependencyEdge final {
    AssetHandle dependent;
    AssetHandle dependency;
    bool is_active = false;
};
}
