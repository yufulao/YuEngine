// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmStreamQueueOperation.h

#pragma once

namespace yuengine::audio {
enum class AudioPcmStreamQueueOperation {
    None,
    Create,
    Query,
    Drain,
    Release
};
}
