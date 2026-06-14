#include "yuengine/rhi/null_rhi_device.h"

#include <algorithm>

#include "yuengine/rhi/rhi_constants.h"

namespace yuengine::rhi {
namespace {
constexpr std::uint32_t INVALID_GENERATION = 0U;
}

NullRhiDevice::NullRhiDevice()
    : _targets(),
      _capabilities{},
      _snapshot{},
      _submittedHandle{},
      _presentedHandle{},
      _generationSeed(INVALID_GENERATION),
      _isInitialized(false),
      _hasSubmittedFrame(false),
      _hasPresentedFrame(false) {
}

RHI_STATUS NullRhiDevice::Initialize(const RhiDeviceDesc& desc) {
    if (desc.BackendKind != RHI_BACKEND_KIND::Null) {
        return RHI_STATUS::UnsupportedBackend;
    }

    if (desc.ColorTargetCapacity == 0U) {
        return RHI_STATUS::InvalidDescriptor;
    }

    if (desc.ColorTargetCapacity > MAX_COLOR_TARGETS) {
        return RHI_STATUS::CapacityExceeded;
    }

    if (desc.CommandListCapacity == 0U) {
        return RHI_STATUS::InvalidDescriptor;
    }

    if (desc.CommandListCapacity > MAX_COMMANDS) {
        return RHI_STATUS::CapacityExceeded;
    }

    ++_generationSeed;
    if (_generationSeed == INVALID_GENERATION) {
        ++_generationSeed;
    }

    _targets.assign(desc.ColorTargetCapacity, RhiTargetSlot{});
    for (RhiTargetSlot& target : _targets) {
        target.Generation = _generationSeed;
    }

    _capabilities = RhiCapabilities{
        RHI_BACKEND_KIND::Null,
        RHI_FORMAT::Rgba8Unorm,
        desc.ColorTargetCapacity,
        desc.CommandListCapacity,
        MAX_COLOR_TARGET_EXTENT,
        MAX_CAPTURE_FIXTURE_EXTENT,
        true};
    _snapshot = RhiDeviceSnapshot{};
    _snapshot.ColorTargetCapacity = desc.ColorTargetCapacity;
    _isInitialized = true;
    _hasSubmittedFrame = false;
    _hasPresentedFrame = false;
    return RHI_STATUS::Success;
}

RHI_STATUS NullRhiDevice::CreateColorTarget(const RhiColorTargetDesc& desc, RhiTextureHandle& outHandle) {
    if (!_isInitialized) {
        return RecordFailure(RHI_STATUS::InvalidLifecycle);
    }

    if (desc.Format != RHI_FORMAT::Rgba8Unorm) {
        return RecordFailure(RHI_STATUS::UnsupportedFormat);
    }

    if (!IsColorTargetDescValid(desc)) {
        return RecordFailure(RHI_STATUS::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < _targets.size(); ++index) {
        RhiTargetSlot& slot = _targets[index];
        if (slot.IsActive) {
            continue;
        }

        if (slot.Generation == INVALID_GENERATION) {
            slot.Generation = 1U;
        }

        slot.IsActive = true;
        slot.Desc = desc;
        slot.Bytes.assign(PixelByteCount(desc), 0U);
        outHandle = RhiTextureHandle{static_cast<std::uint32_t>(index), slot.Generation};
        ++_snapshot.ColorTargetCount;
        ++_snapshot.CreatedTargetCount;
        return RHI_STATUS::Success;
    }

    return RecordFailure(RHI_STATUS::CapacityExceeded);
}

RHI_STATUS NullRhiDevice::DestroyTarget(RhiTextureHandle handle) {
    if (!IsTargetHandleValid(handle)) {
        return RecordFailure(RHI_STATUS::InvalidHandle);
    }

    RhiTargetSlot& slot = _targets[handle.Slot];
    slot.IsActive = false;
    slot.Bytes.clear();
    ++slot.Generation;
    --_snapshot.ColorTargetCount;
    ++_snapshot.DestroyedTargetCount;
    return RHI_STATUS::Success;
}

RHI_STATUS NullRhiDevice::RecordClear(RhiCommandList& commandList, RhiTextureHandle handle, RhiColor color) {
    if (!IsTargetHandleValid(handle)) {
        return RecordFailure(RHI_STATUS::InvalidHandle);
    }

    const RHI_STATUS status = commandList.RecordClear(handle, color);
    if (status != RHI_STATUS::Success) {
        return RecordFailure(status);
    }

    ++_snapshot.RecordedCommandCount;
    return RHI_STATUS::Success;
}

RHI_STATUS NullRhiDevice::Submit(const RhiCommandList& commandList) {
    if (!commandList.IsComplete()) {
        return RecordFailure(RHI_STATUS::InvalidLifecycle);
    }

    if (commandList.Capacity() > _capabilities.CommandListCapacity) {
        return RecordFailure(RHI_STATUS::CapacityExceeded);
    }

    const RhiTextureHandle target = commandList.TargetHandle();
    if (!IsTargetHandleValid(target)) {
        return RecordFailure(RHI_STATUS::InvalidHandle);
    }

    for (std::size_t index = 0U; index < commandList.CommandCount(); ++index) {
        const RhiCommandRecord& command = commandList.CommandAt(index);
        if (!IsCommandTargetValidForFrame(command, target)) {
            return RecordFailure(RHI_STATUS::InvalidHandle);
        }
    }

    _snapshot.CommandStorageCapacityBeforeFrame = commandList.Capacity();
    for (std::size_t index = 0U; index < commandList.CommandCount(); ++index) {
        const RhiCommandRecord& command = commandList.CommandAt(index);
        if (command.Type == RHI_COMMAND_TYPE::ClearColor) {
            ExecuteClear(command.Target, command.Color);
            continue;
        }
    }

    _submittedHandle = target;
    _hasSubmittedFrame = true;
    _hasPresentedFrame = false;
    ++_snapshot.SubmitCount;
    _snapshot.CommandStorageCapacityAfterLastFrame = commandList.Capacity();
    return RHI_STATUS::Success;
}

RHI_STATUS NullRhiDevice::Present() {
    if (!_hasSubmittedFrame) {
        return RecordFailure(RHI_STATUS::InvalidLifecycle);
    }

    if (!IsTargetHandleValid(_submittedHandle)) {
        return RecordFailure(RHI_STATUS::InvalidHandle);
    }

    _presentedHandle = _submittedHandle;
    _hasPresentedFrame = true;
    ++_snapshot.PresentCount;
    return RHI_STATUS::Success;
}

RhiCaptureResult NullRhiDevice::CapturePresentedTarget(std::span<std::uint8_t> destination) {
    if (!_hasPresentedFrame) {
        RecordFailure(RHI_STATUS::InvalidLifecycle);
        return RhiCaptureResult{RHI_STATUS::InvalidLifecycle, 0U};
    }

    if (!IsTargetHandleValid(_presentedHandle)) {
        RecordFailure(RHI_STATUS::InvalidHandle);
        return RhiCaptureResult{RHI_STATUS::InvalidHandle, 0U};
    }

    const RhiTargetSlot& slot = _targets[_presentedHandle.Slot];
    if (slot.Desc.Extent.Width > MAX_CAPTURE_FIXTURE_EXTENT || slot.Desc.Extent.Height > MAX_CAPTURE_FIXTURE_EXTENT) {
        RecordFailure(RHI_STATUS::CapacityExceeded);
        _snapshot.LastCaptureBytesWritten = 0U;
        return RhiCaptureResult{RHI_STATUS::CapacityExceeded, 0U};
    }

    const std::size_t byteCount = slot.Bytes.size();
    if (destination.size() < byteCount) {
        RecordFailure(RHI_STATUS::CapacityExceeded);
        _snapshot.LastCaptureBytesWritten = 0U;
        return RhiCaptureResult{RHI_STATUS::CapacityExceeded, 0U};
    }

    std::copy(slot.Bytes.begin(), slot.Bytes.end(), destination.begin());
    ++_snapshot.CaptureCount;
    _snapshot.LastCaptureBytesWritten = byteCount;
    return RhiCaptureResult{RHI_STATUS::Success, byteCount};
}

RhiCapabilities NullRhiDevice::Capabilities() const {
    return _capabilities;
}

RhiDeviceSnapshot NullRhiDevice::Snapshot() const {
    return _snapshot;
}

RHI_STATUS NullRhiDevice::RecordFailure(RHI_STATUS status) {
    ++_snapshot.FailedOperationCount;
    return status;
}

bool NullRhiDevice::IsTargetHandleValid(RhiTextureHandle handle) const {
    if (!_isInitialized) {
        return false;
    }

    if (handle.Generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.Slot >= _targets.size()) {
        return false;
    }

    const RhiTargetSlot& slot = _targets[handle.Slot];
    if (!slot.IsActive) {
        return false;
    }

    return slot.Generation == handle.Generation;
}

bool NullRhiDevice::IsCommandTargetValidForFrame(const RhiCommandRecord& command, RhiTextureHandle frameTarget) const {
    if (command.Type != RHI_COMMAND_TYPE::ClearColor) {
        return true;
    }

    if (command.Target.Slot != frameTarget.Slot) {
        return false;
    }

    if (command.Target.Generation != frameTarget.Generation) {
        return false;
    }

    return IsTargetHandleValid(command.Target);
}

bool NullRhiDevice::IsColorTargetDescValid(const RhiColorTargetDesc& desc) const {
    if (desc.Extent.Width == 0U) {
        return false;
    }

    if (desc.Extent.Height == 0U) {
        return false;
    }

    if (desc.Extent.Width > MAX_COLOR_TARGET_EXTENT) {
        return false;
    }

    if (desc.Extent.Height > MAX_COLOR_TARGET_EXTENT) {
        return false;
    }

    return true;
}

std::size_t NullRhiDevice::PixelByteCount(const RhiColorTargetDesc& desc) const {
    return static_cast<std::size_t>(desc.Extent.Width) * static_cast<std::size_t>(desc.Extent.Height) * RGBA8_BYTES_PER_PIXEL;
}

void NullRhiDevice::ExecuteClear(RhiTextureHandle handle, RhiColor color) {
    if (!IsTargetHandleValid(handle)) {
        return;
    }

    RhiTargetSlot& slot = _targets[handle.Slot];
    for (std::size_t index = 0U; index < slot.Bytes.size(); index += RGBA8_BYTES_PER_PIXEL) {
        slot.Bytes[index] = color.R;
        slot.Bytes[index + 1U] = color.G;
        slot.Bytes[index + 2U] = color.B;
        slot.Bytes[index + 3U] = color.A;
    }
}
}
