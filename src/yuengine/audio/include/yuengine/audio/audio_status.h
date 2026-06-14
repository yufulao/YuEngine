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
