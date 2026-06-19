// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiCacheCounters.h

#pragma once

#include <cstdint>

namespace yuengine::uicore {
struct UiCacheCounters final {
    std::uint32_t layout_rebuild_count = 0U;
    std::uint32_t paint_rebuild_count = 0U;
};
}
