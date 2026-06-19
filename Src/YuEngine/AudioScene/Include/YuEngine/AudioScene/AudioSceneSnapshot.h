// 模块: YuEngine AudioScene
// 文件: Src/YuEngine/AudioScene/Include/YuEngine/AudioScene/AudioSceneSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/AudioScene/AudioSceneStatus.h"

namespace yuengine::audioscene {
struct AudioSceneSnapshot final {
    std::uint64_t submit_count = 0U;
    std::uint64_t failed_submit_count = 0U;
    std::uint32_t last_frame_id = 0U;
    std::size_t last_active_source_count = 0U;
    std::size_t last_playing_source_count = 0U;
    std::size_t last_queue_request_count = 0U;
    std::size_t last_skipped_source_count = 0U;
    std::uint32_t last_bus_id = 0U;
    AudioSceneStatus last_status = AudioSceneStatus::Success;
};
}
