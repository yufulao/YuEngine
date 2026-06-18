// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformManifestStreamState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldSceneObjectTransformManifestStreamState final {
    std::uint32_t identity_record_count = 0U;
    std::uint32_t transform_record_count = 0U;
    std::uint32_t identity_chunk_count = 0U;
    std::uint32_t transform_chunk_count = 0U;
    std::uint32_t committed_byte_count = 0U;
};
}
