// Module: YuEngine AudioResource
// File: Src/YuEngine/AudioResource/Include/YuEngine/AudioResource/AudioResourcePcmPacketImportRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Audio/AudioPcmSamplePacketRequest.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportHandle.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::audioresource {
struct AudioResourcePcmPacketImportRecord final {
    AudioResourcePcmPacketImportHandle handle{};
    std::uint64_t import_id = 0U;
    yuengine::resource::ResourceHandle resource;
    yuengine::resource::ResourceTypeId expected_type;
    std::uint64_t payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    std::uint64_t decode_result_id = 0U;
    yuengine::resource::ResourceDecodePlanAssetClass asset_class =
        yuengine::resource::ResourceDecodePlanAssetClass::Unknown;
    yuengine::resource::ResourceDecodeResultClass result_class =
        yuengine::resource::ResourceDecodeResultClass::Unknown;
    std::uint32_t decoded_byte_count = 0U;
    yuengine::audio::AudioPcmSamplePacketRequest packet_request;
    AudioResourcePcmPacketImportStatus status = AudioResourcePcmPacketImportStatus::Success;
    bool is_active = false;
};
}
