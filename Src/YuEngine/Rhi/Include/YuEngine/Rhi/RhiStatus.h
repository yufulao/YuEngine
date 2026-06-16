// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiStatus.h

#pragma once

namespace yuengine::rhi {
enum class RhiStatus {
    Success,
    UnsupportedBackend,
    UnsupportedFormat,
    InvalidDescriptor,
    CapacityExceeded,
    InvalidHandle,
    InvalidLifecycle,
    MissingHardware,
    DeviceLost
};
}
