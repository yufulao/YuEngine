// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetHandle.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetConstants.h"

namespace yuengine::asset {
struct AssetHandle final {
    std::uint32_t slot = INVALID_ASSET_SLOT;
    std::uint32_t generation = INVALID_ASSET_GENERATION;

    /**
     * @comment 检查值是否合法。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const {
        if (slot == INVALID_ASSET_SLOT) {
            return false;
        }

        return generation != INVALID_ASSET_GENERATION;
    }
};
}
