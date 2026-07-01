// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneSubmitResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderScene/RenderSceneStatus.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::renderscene {
struct RenderSceneSubmitResult final {
    RenderSceneStatus status = RenderSceneStatus::Success;
    std::uint32_t frame_id = 0U;
    std::uint32_t camera_id = 0U;
    std::size_t visible_entity_count = 0U;
    std::size_t output_packet_count = 0U;
    std::size_t required_output_packet_count = 0U;
    std::size_t failed_entry_index = 0U;
    yuengine::world::WorldObjectId failed_entity_id{};
    std::uint32_t failed_camera_id = 0U;
    std::uint32_t failed_draw_id = 0U;
    std::size_t skipped_entity_count = 0U;
};
}
