#pragma once

namespace yuengine::rhi {
enum class RHI_STATUS {
    Success,
    UnsupportedBackend,
    UnsupportedFormat,
    InvalidDescriptor,
    CapacityExceeded,
    InvalidHandle,
    InvalidLifecycle
};
}
