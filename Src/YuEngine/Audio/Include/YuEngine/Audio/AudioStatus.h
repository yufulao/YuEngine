// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioStatus.h

#pragma once

namespace yuengine::audio {
enum class AudioStatus {
    Success,
    UnsupportedBackend,
    UnsupportedFormat,
    InvalidDescriptor,
    CapacityExceeded,
    SourceNotFound,
    InvalidHandle,
    InvalidGain
};
}
