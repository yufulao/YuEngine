// Module: YuEngine AudioResource
// File: Src/YuEngine/AudioResource/Include/YuEngine/AudioResource/AudioResourcePcmPacketImportRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/Audio/AudioPcmSamplePacketRequest.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::audioresource {
struct AudioResourcePcmPacketImportRequest final {
    std::uint64_t import_id = 0U;
    yuengine::resource::ResourceHandle resource;
    yuengine::resource::ResourceTypeId expected_type;
    std::uint64_t payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    std::uint64_t decode_result_id = 0U;
    yuengine::audio::AudioPcmSamplePacketRequest packet_request;
};
}
