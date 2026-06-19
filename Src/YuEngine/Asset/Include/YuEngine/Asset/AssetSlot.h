// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetSlot.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetConstants.h"
#include "YuEngine/Asset/AssetRecord.h"

namespace yuengine::asset {
struct AssetSlot final {
    AssetRecord record;
    std::uint32_t generation = INVALID_ASSET_GENERATION;
    bool is_active = false;
};
}
