// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetDescriptor.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::asset {
struct AssetDescriptor final {
    std::uint64_t stable_id = 0U;
    AssetTypeId asset_type;
    yuengine::resource::ResourceHandle resource;
    yuengine::resource::ResourceTypeId resource_type;
    std::uint32_t initial_reference_count = 0U;
};
}
