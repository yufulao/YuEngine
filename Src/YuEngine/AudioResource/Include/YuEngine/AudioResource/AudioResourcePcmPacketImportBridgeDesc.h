// Module: YuEngine AudioResource
// File: Src/YuEngine/AudioResource/Include/YuEngine/AudioResource/AudioResourcePcmPacketImportBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/AudioResource/AudioResourcePcmPacketImportConstants.h"

namespace yuengine::audioresource {
struct AudioResourcePcmPacketImportBridgeDesc final {
    std::uint32_t import_capacity = MAX_AUDIO_RESOURCE_PCM_PACKET_IMPORT_RECORDS;
};
}
