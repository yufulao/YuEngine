#pragma once

namespace yuengine::rhi
{
enum class RhiStatus
{
    Success,
    UnsupportedBackend,
    UnsupportedFormat,
    InvalidDescriptor,
    CapacityExceeded,
    InvalidHandle,
    InvalidLifecycle
};
}
