// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffState.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldSceneAssemblyStatus.h"

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectRestoreHandoffState final {
    std::uint32_t input_mapping_count = 0U;
    std::uint32_t output_identity_count = 0U;
    std::uint32_t output_transform_count = 0U;
    std::uint32_t proof_record_count = 0U;
    std::uint32_t slice_record_count = 0U;
    std::uint32_t gate_record_count = 0U;
    yuengine::world::WorldSceneAssemblyStatus assembly_status =
        yuengine::world::WorldSceneAssemblyStatus::Success;
    std::uint32_t restored_attachment_count = 0U;
    std::uint32_t restored_binding_count = 0U;
    std::uint32_t restored_identity_count = 0U;
    std::uint32_t restored_transform_count = 0U;
    std::uint32_t applied_transform_value_count = 0U;
    std::uint32_t updated_world_object_count = 0U;
};
}
