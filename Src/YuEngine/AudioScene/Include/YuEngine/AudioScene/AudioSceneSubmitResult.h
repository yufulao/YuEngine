// 模块: YuEngine AudioScene
// 文件: Src/YuEngine/AudioScene/Include/YuEngine/AudioScene/AudioSceneSubmitResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/AudioScene/AudioSceneStatus.h"

namespace yuengine::audioscene {
struct AudioSceneSubmitResult final {
    AudioSceneStatus status = AudioSceneStatus::Success;
    std::uint32_t frame_id = 0U;
    std::size_t active_source_count = 0U;
    std::size_t playing_source_count = 0U;
    std::size_t queue_request_count = 0U;
    std::size_t skipped_source_count = 0U;
    std::size_t required_output_contract_count = 0U;
    std::uint32_t last_bus_id = 0U;
};
}
