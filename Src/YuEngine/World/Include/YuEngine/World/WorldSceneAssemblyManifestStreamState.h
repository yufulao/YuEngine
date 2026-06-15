// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyManifestStreamState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldSceneAssemblyManifestStreamState final {
    std::uint32_t attachment_record_count = 0U;
    std::uint32_t binding_record_count = 0U;
    std::uint32_t attachment_chunk_count = 0U;
    std::uint32_t binding_chunk_count = 0U;
    std::uint32_t committed_byte_count = 0U;
};
}
