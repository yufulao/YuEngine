// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceFactory.h

#pragma once

#include <cstddef>
#include <span>

#include "YuEngine/Rhi/RhiBackendKind.h"
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
     * @comment Creates a backend-neutral device view in caller-owned storage.
     * @param desc Input descriptor.
     * @param device_storage Caller-owned storage for the selected backend.
     * @return Explicit creation result.
     */
    static RhiDeviceCreateResult CreateDevice(const RhiDeviceDesc &desc, std::span<std::byte> device_storage);
    /**
     * @comment Destroys a device created in caller-owned storage.
     * @param device Device pointer returned by CreateDevice.
     * @return Explicit operation status.
     */
    static RhiStatus DestroyDevice(IRhiDevice *device);
    /**
     * @comment Returns required storage bytes for a backend.
     * @param backend_kind Input backend kind.
     * @return Required byte count.
     */
    static std::size_t RequiredDeviceStorageSize(RhiBackendKind backend_kind);
    /**
     * @comment Returns required storage alignment for a backend.
     * @param backend_kind Input backend kind.
     * @return Required byte alignment.
     */
    static std::size_t RequiredDeviceStorageAlignment(RhiBackendKind backend_kind);
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
