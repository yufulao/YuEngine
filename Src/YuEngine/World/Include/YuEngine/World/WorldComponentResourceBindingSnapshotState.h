// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingSnapshotState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldComponentResourceBindingSnapshotState final {
    std::uint32_t binding_record_count = 0U;
    std::uint32_t chunk_record_count = 0U;
    std::uint32_t committed_byte_count = 0U;
};
}
