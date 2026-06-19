// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetTextureReadyRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiSampledTextureBinding.h"

namespace yuengine::asset {
struct AssetTextureReadyRecord final {
    yuengine::rhi::RhiSampledTextureBinding sampled_texture;
    std::uint32_t decoded_byte_count = 0U;
    std::uint32_t uploaded_byte_count = 0U;
    bool is_ready = false;
};
}
