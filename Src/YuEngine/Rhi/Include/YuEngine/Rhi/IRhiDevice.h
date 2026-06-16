// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/IRhiDevice.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
class IRhiDevice {
public:
    /**
     * @comment Destroys the interface.
     */
    virtual ~IRhiDevice() = default;

    /**
     * @comment Initializes the device.
     * @param desc Input descriptor.
     * @return Explicit operation status.
     */
    virtual RhiStatus Initialize(const RhiDeviceDesc &desc) = 0;
    /**
     * @comment Creates a color target.
     * @param desc Input descriptor.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    virtual RhiStatus CreateColorTarget(const RhiColorTargetDesc &desc, RhiTextureHandle &out_handle) = 0;
    /**
     * @comment Destroys a target.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    virtual RhiStatus DestroyTarget(RhiTextureHandle handle) = 0;
    /**
     * @comment Records a clear command.
     * @param command_list Command list updated by the function.
     * @param handle Input handle.
     * @param color Input color.
     * @return Explicit operation status.
     */
    virtual RhiStatus RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) = 0;
    /**
     * @comment Submits recorded work.
     * @param command_list Input command list.
     * @return Explicit operation status.
     */
    virtual RhiStatus Submit(const RhiCommandList &command_list) = 0;
    /**
     * @comment Presents submitted work.
     * @return Explicit operation status.
     */
    virtual RhiStatus Present() = 0;
    /**
     * @comment Captures the presented target into caller-owned storage.
     * @param destination Input destination.
     * @return Explicit operation result.
     */
    virtual RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination) = 0;
    /**
     * @comment Returns supported capabilities.
     * @return Capability data.
     */
    virtual RhiCapabilities Capabilities() const = 0;
    /**
     * @comment Returns current device state.
     * @return Snapshot value.
     */
    virtual RhiDeviceSnapshot Snapshot() const = 0;
};
}
