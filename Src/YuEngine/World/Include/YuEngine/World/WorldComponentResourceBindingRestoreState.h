// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingRestoreState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldComponentResourceBindingRestoreState final {
    std::uint32_t input_binding_count = 0U;
    std::uint32_t restored_binding_count = 0U;
    std::uint32_t rejected_binding_count = 0U;
};
}
