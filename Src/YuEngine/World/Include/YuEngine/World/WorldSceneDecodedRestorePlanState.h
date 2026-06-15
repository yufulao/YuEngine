// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneDecodedRestorePlanState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldSceneDecodedRestorePlanState final {
    std::uint32_t input_identity_count = 0U;
    std::uint32_t input_transform_count = 0U;
    std::uint32_t input_attachment_count = 0U;
    std::uint32_t input_binding_count = 0U;
    std::uint32_t output_plan_count = 0U;
    std::uint32_t planned_identity_count = 0U;
    std::uint32_t planned_transform_count = 0U;
    std::uint32_t planned_attachment_count = 0U;
    std::uint32_t planned_binding_count = 0U;
    std::uint32_t projected_object_acquire_count = 0U;
    std::uint32_t projected_resource_acquire_count = 0U;
};
}
