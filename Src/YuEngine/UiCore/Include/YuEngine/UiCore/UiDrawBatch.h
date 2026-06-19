// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDrawBatch.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDrawBatchKey.h"

namespace yuengine::uicore {
struct UiDrawBatch final {
    UiDrawBatchKey key;
    std::uint32_t first_element_index = 0U;
    std::uint32_t element_count = 0U;
};
}
