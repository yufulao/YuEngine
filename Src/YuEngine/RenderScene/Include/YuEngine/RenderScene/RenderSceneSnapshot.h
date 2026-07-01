// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderScene/RenderSceneStatus.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::renderscene {
struct RenderSceneSnapshot final {
    std::uint64_t submit_count = 0U;
    std::uint64_t failed_submit_count = 0U;
    std::uint32_t last_frame_id = 0U;
    std::uint32_t last_camera_id = 0U;
    std::size_t last_output_packet_count = 0U;
    std::size_t last_visible_entity_count = 0U;
    std::size_t last_required_output_packet_count = 0U;
    std::size_t last_failed_entry_index = 0U;
    yuengine::world::WorldObjectId last_failed_entity_id{};
    std::uint32_t last_failed_camera_id = 0U;
    std::uint32_t last_failed_draw_id = 0U;
    std::size_t last_skipped_entity_count = 0U;
    RenderSceneStatus last_status = RenderSceneStatus::Success;
};
}
