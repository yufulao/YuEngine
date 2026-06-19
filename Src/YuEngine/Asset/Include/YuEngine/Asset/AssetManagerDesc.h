// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetManagerDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetConstants.h"

namespace yuengine::asset {
struct AssetManagerDesc final {
    std::uint32_t asset_capacity = MAX_ASSET_COUNT;
    std::uint32_t type_capacity = MAX_ASSET_TYPE_COUNT;
    std::uint32_t dependency_edge_capacity = MAX_ASSET_DEPENDENCY_EDGE_COUNT;
};
}
