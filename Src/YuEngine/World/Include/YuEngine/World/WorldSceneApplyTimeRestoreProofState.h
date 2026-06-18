// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneApplyTimeRestoreProofState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldSceneApplyTimeRestoreProofState final {
    std::uint32_t input_identity_count = 0U;
    std::uint32_t input_transform_count = 0U;
    std::uint32_t input_attachment_count = 0U;
    std::uint32_t input_binding_count = 0U;
    std::uint32_t plan_record_count = 0U;
    std::uint32_t proof_record_count = 0U;
    std::uint32_t slice_record_count = 0U;
    std::uint32_t proven_identity_count = 0U;
    std::uint32_t proven_transform_count = 0U;
    std::uint32_t proven_attachment_count = 0U;
    std::uint32_t proven_binding_count = 0U;
    std::uint32_t emitted_slice_count = 0U;
    std::uint32_t projected_object_acquire_count = 0U;
    std::uint32_t projected_resource_acquire_count = 0U;
};
}
