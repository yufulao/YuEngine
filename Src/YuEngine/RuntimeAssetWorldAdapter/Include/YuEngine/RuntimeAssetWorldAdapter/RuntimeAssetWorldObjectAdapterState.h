// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterState.h

#pragma once

#include <cstdint>

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectAdapterState final {
    std::uint32_t input_mapping_count = 0U;
    std::uint32_t output_identity_count = 0U;
    std::uint32_t output_transform_count = 0U;
    std::uint32_t sampled_transform_value_count = 0U;
    std::uint32_t required_sampled_transform_value_count = 0U;
    std::uint32_t applied_transform_value_count = 0U;
    std::uint32_t updated_world_object_count = 0U;
};
}
