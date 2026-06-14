#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "yuengine/rhi/RhiCapabilities.h"
#include "yuengine/rhi/RhiCaptureResult.h"
#include "yuengine/rhi/RhiColorTargetDesc.h"
#include "yuengine/rhi/RhiCommandList.h"
#include "yuengine/rhi/RhiDeviceDesc.h"
#include "yuengine/rhi/RhiDeviceSnapshot.h"
#include "yuengine/rhi/RhiStatus.h"
#include "yuengine/rhi/RhiTargetSlot.h"
#include "yuengine/rhi/RhiTextureHandle.h"

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
