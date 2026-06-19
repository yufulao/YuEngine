// 模块: YuEngine AudioScene
// 文件: Src/YuEngine/AudioScene/Include/YuEngine/AudioScene/AudioSceneConstants.h

#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::audioscene {
constexpr std::size_t MAX_AUDIO_SCENE_QUEUE_REQUESTS = 64U;
constexpr std::uint32_t AUDIO_SCENE_MASTER_BUS_ID = 1U;
constexpr std::uint32_t AUDIO_SCENE_EFFECTS_BUS_ID = 2U;
constexpr std::uint32_t AUDIO_SCENE_MAX_BUS_ID = AUDIO_SCENE_EFFECTS_BUS_ID;
constexpr std::uint32_t AUDIO_SCENE_BUS_QUEUE_ID_STRIDE = 1000U;
constexpr float AUDIO_SCENE_MAX_GAIN = 4.0F;
}
