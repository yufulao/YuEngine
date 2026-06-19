// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetAudioReadyRecord.h

#pragma once

#include "YuEngine/Audio/AudioPcmSamplePacketRequest.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportHandle.h"

namespace yuengine::asset {
struct AssetAudioReadyRecord final {
    yuengine::audioresource::AudioResourcePcmPacketImportHandle import_handle;
    yuengine::audio::AudioPcmSamplePacketRequest packet_request;
    bool is_ready = false;
};
}
