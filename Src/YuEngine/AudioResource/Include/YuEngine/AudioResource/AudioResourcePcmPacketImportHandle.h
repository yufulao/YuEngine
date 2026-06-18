// 模块: YuEngine AudioResource
// 文件: Src/YuEngine/AudioResource/Include/YuEngine/AudioResource/AudioResourcePcmPacketImportHandle.h

#pragma once

#include <cstdint>

#include "YuEngine/AudioResource/AudioResourcePcmPacketImportConstants.h"

namespace yuengine::audioresource {
struct AudioResourcePcmPacketImportHandle final {
    std::uint32_t slot = INVALID_AUDIO_RESOURCE_PCM_PACKET_IMPORT_SLOT;
    std::uint32_t generation = INVALID_AUDIO_RESOURCE_PCM_PACKET_IMPORT_GENERATION;

    /**
     * @comment 检查 value 是否有效。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsValid() const {
        if (slot == INVALID_AUDIO_RESOURCE_PCM_PACKET_IMPORT_SLOT) {
            return false;
        }

        return generation != INVALID_AUDIO_RESOURCE_PCM_PACKET_IMPORT_GENERATION;
    }
};
}
