// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderScene/RenderSceneStatus.h"

namespace yuengine::renderscene {
struct RenderSceneSnapshot final {
    std::uint64_t submit_count = 0U;
    std::uint64_t failed_submit_count = 0U;
    std::uint32_t last_frame_id = 0U;
    std::uint32_t last_camera_id = 0U;
    std::size_t last_output_packet_count = 0U;
    std::size_t last_visible_entity_count = 0U;
    RenderSceneStatus last_status = RenderSceneStatus::Success;
};
}
