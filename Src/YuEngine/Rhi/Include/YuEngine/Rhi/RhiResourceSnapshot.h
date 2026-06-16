// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiResourceSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::rhi {
struct RhiResourceSnapshot final {
    std::size_t buffer_capacity = 0U;
    std::size_t buffer_count = 0U;
    std::size_t texture_capacity = 0U;
    std::size_t texture_count = 0U;
    std::size_t sampler_capacity = 0U;
    std::size_t sampler_count = 0U;
    std::size_t shader_module_capacity = 0U;
    std::size_t shader_module_count = 0U;
    std::size_t pipeline_capacity = 0U;
    std::size_t pipeline_count = 0U;
    std::uint64_t created_primitive_count = 0U;
    std::uint64_t destroyed_primitive_count = 0U;
    std::uint64_t updated_primitive_count = 0U;
    std::uint64_t signaled_fence_count = 0U;
    std::size_t last_update_bytes = 0U;
};
}
