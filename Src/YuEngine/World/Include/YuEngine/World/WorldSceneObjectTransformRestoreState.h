// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformRestoreState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldSceneObjectTransformRestoreState final {
    std::uint32_t input_identity_count = 0U;
    std::uint32_t input_transform_count = 0U;
    std::uint32_t restored_identity_count = 0U;
    std::uint32_t restored_transform_count = 0U;
    std::uint32_t rejected_identity_count = 0U;
    std::uint32_t rejected_transform_count = 0U;
};
}
