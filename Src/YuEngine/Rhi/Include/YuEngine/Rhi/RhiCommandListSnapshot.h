// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandListSnapshot.h

#pragma once

#include <cstddef>

namespace yuengine::rhi {
struct RhiCommandListSnapshot final {
    std::size_t capacity = 0U;
    std::size_t command_count = 0U;
    std::size_t draw_command_count = 0U;
    std::size_t indexed_draw_command_count = 0U;
    std::size_t sampled_texture_bind_command_count = 0U;
    std::size_t sampler_bind_command_count = 0U;
    std::size_t blend_state_bind_command_count = 0U;
    bool is_recording = false;
    bool is_complete = false;
};
}
