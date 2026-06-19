// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneActiveRestoreGateState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldSceneActiveRestoreGateState final {
    std::uint32_t input_identity_count = 0U;
    std::uint32_t input_transform_count = 0U;
    std::uint32_t input_attachment_count = 0U;
    std::uint32_t input_binding_count = 0U;
    std::uint32_t proof_record_count = 0U;
    std::uint32_t slice_record_count = 0U;
    std::uint32_t gate_record_count = 0U;
    std::uint32_t policy_record_count = 0U;
    std::uint32_t identity_gate_count = 0U;
    std::uint32_t transform_gate_count = 0U;
    std::uint32_t attachment_gate_count = 0U;
    std::uint32_t binding_gate_count = 0U;
    std::uint32_t projected_object_acquire_count = 0U;
    std::uint32_t projected_resource_acquire_count = 0U;
};
}
