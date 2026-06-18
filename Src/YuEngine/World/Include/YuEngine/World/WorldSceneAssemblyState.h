// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldSceneAssemblyState final {
    std::uint32_t input_attachment_count = 0U;
    std::uint32_t input_binding_count = 0U;
    std::uint32_t restored_attachment_count = 0U;
    std::uint32_t restored_binding_count = 0U;
    std::uint32_t rejected_attachment_count = 0U;
    std::uint32_t rejected_binding_count = 0U;
};
}
