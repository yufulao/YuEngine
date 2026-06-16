// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceFactory.h

#pragma once

#include "YuEngine/Rhi/RhiDeviceCreateResult.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rhi {
class NullRhiDevice;

class RhiDeviceFactory final {
public:
    /**
     * @comment Creates a backend-neutral device view from caller-owned storage.
     * @param desc Input descriptor.
     * @param null_device Caller-owned null backend storage.
     * @return Explicit creation result.
     */
    static RhiDeviceCreateResult CreateDevice(const RhiDeviceDesc &desc, NullRhiDevice *null_device);
    /**
     * @comment Validates an RHI-owned native surface descriptor.
     * @param surface_desc Input descriptor.
     * @return Explicit operation status.
     */
    static RhiStatus ValidateNativeSurfaceDesc(const RhiNativeSurfaceDesc &surface_desc);

private:
    RhiDeviceFactory() = delete;

    static RhiStatus ValidateDeviceDesc(const RhiDeviceDesc &desc);
};
}
