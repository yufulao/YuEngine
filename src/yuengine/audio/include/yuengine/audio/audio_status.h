#pragma once

namespace yuengine::audio {
enum class AUDIO_STATUS {
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
