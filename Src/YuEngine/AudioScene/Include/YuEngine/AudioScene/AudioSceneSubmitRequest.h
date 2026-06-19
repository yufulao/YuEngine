// 模块: YuEngine AudioScene
// 文件: Src/YuEngine/AudioScene/Include/YuEngine/AudioScene/AudioSceneSubmitRequest.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/AudioScene/AudioSceneSourceRecord.h"

namespace yuengine::audioscene {
struct AudioSceneSubmitRequest final {
    std::uint32_t frame_id = 0U;
    bool backend_available = true;
    std::size_t chunk_frame_count = yuengine::audio::MAX_PCM_STREAM_CHUNK_FRAMES;
    std::span<const AudioSceneSourceRecord> sources{};
};
}
