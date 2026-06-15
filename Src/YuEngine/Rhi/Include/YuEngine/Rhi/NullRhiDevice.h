// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/NullRhiDevice.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTargetSlot.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
class NullRhiDevice final {
public:
    /**
     * @comment Constructs a NullRhiDevice instance.
     */
    NullRhiDevice();

    /**
     * @comment Initializes the instance.
     * @param desc Input descriptor.
     * @return Explicit operation status.
     */
    RhiStatus Initialize(const RhiDeviceDesc& desc);
    /**
     * @comment Creates color target.
     * @param desc Input descriptor.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    RhiStatus CreateColorTarget(const RhiColorTargetDesc& desc, RhiTextureHandle& out_handle);
    /**
     * @comment Destroys target.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    RhiStatus DestroyTarget(RhiTextureHandle handle);
    /**
     * @comment Records clear.
     * @param command_list Command list updated by the function.
     * @param handle Input handle.
     * @param color Input color.
     * @return Explicit operation status.
     */
    RhiStatus RecordClear(RhiCommandList& command_list, RhiTextureHandle handle, RhiColor color);
    /**
     * @comment Submits requested work.
     * @param command_list Input command list.
     * @return Explicit operation status.
     */
    RhiStatus Submit(const RhiCommandList& command_list);
    /**
     * @comment Presents the submitted target.
     * @return Explicit operation status.
     */
    RhiStatus Present();
    /**
     * @comment Captures the presented target.
     * @param destination Input destination.
     * @return Explicit operation result.
     */
    RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination);
    /**
     * @comment Returns the supported capabilities.
     * @return Capability data.
     */
    RhiCapabilities Capabilities() const;
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    RhiDeviceSnapshot Snapshot() const;

private:
    RhiStatus RecordFailure(RhiStatus status);
    bool IsTargetHandleValid(RhiTextureHandle handle) const;
    bool IsCommandTargetValidForFrame(const RhiCommandRecord& command, RhiTextureHandle frame_target) const;
    bool IsColorTargetDescValid(const RhiColorTargetDesc& desc) const;
    std::size_t PixelByteCount(const RhiColorTargetDesc& desc) const;
    void ExecuteClear(RhiTextureHandle handle, RhiColor color);

    std::vector<RhiTargetSlot> targets_;
    RhiCapabilities capabilities_;
    RhiDeviceSnapshot snapshot_;
    RhiTextureHandle submitted_handle_;
    RhiTextureHandle presented_handle_;
    std::uint32_t generation_seed_;
    bool is_initialized_;
    bool has_submitted_frame_;
    bool has_presented_frame_;
};
}
