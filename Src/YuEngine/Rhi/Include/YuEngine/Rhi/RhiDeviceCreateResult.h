// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceCreateResult.h

#pragma once

#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rhi {
class IRhiDevice;

struct RhiDeviceCreateResult final {
    RhiStatus status = RhiStatus::InvalidDescriptor;
    IRhiDevice *device = nullptr;
    RhiCapabilities capabilities{};
};
}
