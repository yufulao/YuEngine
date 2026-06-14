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
