// 模块: YuEngine AudioScene
// 文件: Src/YuEngine/AudioScene/Include/YuEngine/AudioScene/AudioSceneSourceRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetAudioReadyRecord.h"
#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Audio/AudioPcmSamplePacketHandle.h"
#include "YuEngine/Audio/AudioSourceId.h"
#include "YuEngine/AudioScene/AudioSceneConstants.h"
#include "YuEngine/AudioScene/AudioSceneSourceState.h"

namespace yuengine::audioscene {
struct AudioSceneSourceRecord final {
    yuengine::audio::AudioSourceId source_id{};
    yuengine::asset::AssetHandle sound_asset{};
    yuengine::asset::AssetAudioReadyRecord audio_ready{};
    yuengine::audio::AudioPcmSamplePacketHandle packet{};
    AudioSceneSourceState state = AudioSceneSourceState::Stopped;
    std::uint32_t bus_id = AUDIO_SCENE_MASTER_BUS_ID;
    float gain = 1.0F;
    bool loop = false;
    bool is_active = false;
};
}
