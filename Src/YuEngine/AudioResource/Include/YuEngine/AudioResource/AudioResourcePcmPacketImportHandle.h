// Module: YuEngine AudioResource
// File: Src/YuEngine/AudioResource/Include/YuEngine/AudioResource/AudioResourcePcmPacketImportHandle.h

#pragma once

#include <cstdint>

#include "YuEngine/AudioResource/AudioResourcePcmPacketImportConstants.h"

namespace yuengine::audioresource {
struct AudioResourcePcmPacketImportHandle final {
    std::uint32_t slot = INVALID_AUDIO_RESOURCE_PCM_PACKET_IMPORT_SLOT;
    std::uint32_t generation = INVALID_AUDIO_RESOURCE_PCM_PACKET_IMPORT_GENERATION;

    /**
     * @comment Checks whether the value is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const {
        if (slot == INVALID_AUDIO_RESOURCE_PCM_PACKET_IMPORT_SLOT) {
            return false;
        }

        return generation != INVALID_AUDIO_RESOURCE_PCM_PACKET_IMPORT_GENERATION;
    }
};
}
