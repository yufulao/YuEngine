// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetTypeId.h

#pragma once

#include <cstdint>

namespace yuengine::asset {
struct AssetTypeId final {
    std::uint32_t value = 0U;

    /**
     * @comment 检查值是否合法。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const {
        return value != 0U;
    }
};
}
