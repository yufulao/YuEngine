#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "yuengine/rhi/rhi_capabilities.h"
#include "yuengine/rhi/rhi_capture_result.h"
#include "yuengine/rhi/rhi_color_target_desc.h"
#include "yuengine/rhi/rhi_command_list.h"
#include "yuengine/rhi/rhi_device_desc.h"
#include "yuengine/rhi/rhi_device_snapshot.h"
#include "yuengine/rhi/rhi_status.h"
#include "yuengine/rhi/rhi_target_slot.h"
#include "yuengine/rhi/rhi_texture_handle.h"

namespace yuengine::rhi {
class NullRhiDevice final {
public:
    NullRhiDevice();

    RhiStatus Initialize(const RhiDeviceDesc& desc);
    RhiStatus CreateColorTarget(const RhiColorTargetDesc& desc, RhiTextureHandle& outHandle);
    RhiStatus DestroyTarget(RhiTextureHandle handle);
    RhiStatus RecordClear(RhiCommandList& commandList, RhiTextureHandle handle, RhiColor color);
    RhiStatus Submit(const RhiCommandList& commandList);
    RhiStatus Present();
    RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination);
    RhiCapabilities Capabilities() const;
    RhiDeviceSnapshot Snapshot() const;

private:
    RhiStatus RecordFailure(RhiStatus status);
    bool IsTargetHandleValid(RhiTextureHandle handle) const;
    bool IsCommandTargetValidForFrame(const RhiCommandRecord& command, RhiTextureHandle frameTarget) const;
    bool IsColorTargetDescValid(const RhiColorTargetDesc& desc) const;
    std::size_t PixelByteCount(const RhiColorTargetDesc& desc) const;
    void ExecuteClear(RhiTextureHandle handle, RhiColor color);

    std::vector<RhiTargetSlot> _targets;
    RhiCapabilities _capabilities;
    RhiDeviceSnapshot _snapshot;
    RhiTextureHandle _submittedHandle;
    RhiTextureHandle _presentedHandle;
    std::uint32_t _generationSeed;
    bool _isInitialized;
    bool _hasSubmittedFrame;
    bool _hasPresentedFrame;
};
}
