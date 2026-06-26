// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeFrameResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderScene/RenderSceneRuntimeFrameStatus.h"

namespace yuengine::renderscene {
struct RenderSceneRuntimeFrameResult final {
    RenderSceneRuntimeFrameStatus status = RenderSceneRuntimeFrameStatus::Success;
    std::uint32_t frame_id = 0U;
    std::uint32_t camera_id = 0U;
    std::uint32_t material_id = 0U;
    std::size_t material_texture_slot_count = 0U;
    std::size_t material_count = 0U;
    std::size_t material_variant_count = 0U;
    std::size_t submitted_entity_count = 0U;
    std::size_t output_draw_count = 0U;
    std::size_t skipped_entity_count = 0U;
    std::uint32_t first_failed_entity_index = 0xFFFFFFFFU;
    std::uint32_t first_failed_material_index = 0xFFFFFFFFU;
};
}
