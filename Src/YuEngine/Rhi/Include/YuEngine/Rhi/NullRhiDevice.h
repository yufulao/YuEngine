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
    NullRhiDevice();

    RhiStatus Initialize(const RhiDeviceDesc& desc);
    RhiStatus CreateColorTarget(const RhiColorTargetDesc& desc, RhiTextureHandle& out_handle);
    RhiStatus DestroyTarget(RhiTextureHandle handle);
    RhiStatus RecordClear(RhiCommandList& command_list, RhiTextureHandle handle, RhiColor color);
    RhiStatus Submit(const RhiCommandList& command_list);
    RhiStatus Present();
    RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination);
    RhiCapabilities Capabilities() const;
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
