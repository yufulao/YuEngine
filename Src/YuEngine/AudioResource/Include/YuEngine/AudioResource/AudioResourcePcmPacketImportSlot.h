// Module: YuEngine AudioResource
// File: Src/YuEngine/AudioResource/Include/YuEngine/AudioResource/AudioResourcePcmPacketImportSlot.h

#pragma once

#include <cstdint>

#include "YuEngine/AudioResource/AudioResourcePcmPacketImportRecord.h"

namespace yuengine::audioresource {
struct AudioResourcePcmPacketImportSlot final {
    AudioResourcePcmPacketImportRecord record{};
    std::uint32_t generation = 1U;
    bool is_active = false;
};
}
