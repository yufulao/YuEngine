#include "yuengine/rhi/NullRhiDevice.h"

#include <algorithm>

#include "yuengine/rhi/RhiConstants.h"

namespace yuengine::rhi
{
namespace
{
constexpr std::uint32_t INVALID_GENERATION = 0U;
}

NullRhiDevice::NullRhiDevice()
    : _targets(),
      _capabilities{},
      _snapshot{},
      _submittedHandle{},
      _presentedHandle{},
      _isInitialized(false),
      _hasSubmittedFrame(false),
      _hasPresentedFrame(false)
{
}

RhiStatus NullRhiDevice::Initialize(const RhiDeviceDesc& desc)
{
    if (desc.BackendKind != RhiBackendKind::Null)
    {
        return RhiStatus::UnsupportedBackend;
    }

    if (desc.ColorTargetCapacity == 0U)
    {
        return RhiStatus::InvalidDescriptor;
    }

    if (desc.ColorTargetCapacity > MAX_COLOR_TARGETS)
    {
        return RhiStatus::CapacityExceeded;
    }

    if (desc.CommandListCapacity == 0U)
    {
        return RhiStatus::InvalidDescriptor;
    }

    if (desc.CommandListCapacity > MAX_COMMANDS)
    {
        return RhiStatus::CapacityExceeded;
    }

    _targets.assign(desc.ColorTargetCapacity, RhiTargetSlot{});
    _capabilities = RhiCapabilities{
        RhiBackendKind::Null,
        RhiFormat::Rgba8Unorm,
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
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::CreateColorTarget(const RhiColorTargetDesc& desc, RhiTextureHandle& outHandle)
{
    if (!_isInitialized)
    {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (desc.Format != RhiFormat::Rgba8Unorm)
    {
        return RecordFailure(RhiStatus::UnsupportedFormat);
    }

    if (!IsColorTargetDescValid(desc))
    {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < _targets.size(); ++index)
    {
        RhiTargetSlot& slot = _targets[index];
        if (slot.IsActive)
        {
            continue;
        }

        if (slot.Generation == INVALID_GENERATION)
        {
            slot.Generation = 1U;
        }

        slot.IsActive = true;
        slot.Desc = desc;
        slot.Bytes.assign(PixelByteCount(desc), 0U);
        outHandle = RhiTextureHandle{static_cast<std::uint32_t>(index), slot.Generation};
        ++_snapshot.ColorTargetCount;
        ++_snapshot.CreatedTargetCount;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus NullRhiDevice::DestroyTarget(RhiTextureHandle handle)
{
    if (!IsTargetHandleValid(handle))
    {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RhiTargetSlot& slot = _targets[handle.Slot];
    slot.IsActive = false;
    slot.Bytes.clear();
    ++slot.Generation;
    --_snapshot.ColorTargetCount;
    ++_snapshot.DestroyedTargetCount;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RecordClear(RhiCommandList& commandList, RhiTextureHandle handle, RhiColor color)
{
    if (!IsTargetHandleValid(handle))
    {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    const RhiStatus status = commandList.RecordClear(handle, color);
    if (status != RhiStatus::Success)
    {
        return RecordFailure(status);
    }

    ++_snapshot.RecordedCommandCount;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::Submit(const RhiCommandList& commandList)
{
    if (!commandList.IsComplete())
    {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    const RhiTextureHandle target = commandList.TargetHandle();
    if (!IsTargetHandleValid(target))
    {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    _snapshot.CommandStorageCapacityBeforeFrame = commandList.Capacity();
    for (std::size_t index = 0U; index < commandList.CommandCount(); ++index)
    {
        const RhiCommandRecord& command = commandList.CommandAt(index);
        if (command.Type == RhiCommandType::ClearColor)
        {
            ExecuteClear(command.Target, command.Color);
            continue;
        }
    }

    _submittedHandle = target;
    _hasSubmittedFrame = true;
    _hasPresentedFrame = false;
    ++_snapshot.SubmitCount;
    _snapshot.CommandStorageCapacityAfterLastFrame = commandList.Capacity();
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::Present()
{
    if (!_hasSubmittedFrame)
    {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    _presentedHandle = _submittedHandle;
    _hasPresentedFrame = true;
    ++_snapshot.PresentCount;
    return RhiStatus::Success;
}

RhiCaptureResult NullRhiDevice::CapturePresentedTarget(std::span<std::uint8_t> destination)
{
    if (!_hasPresentedFrame)
    {
        RecordFailure(RhiStatus::InvalidLifecycle);
        return RhiCaptureResult{RhiStatus::InvalidLifecycle, 0U};
    }

    if (!IsTargetHandleValid(_presentedHandle))
    {
        RecordFailure(RhiStatus::InvalidHandle);
        return RhiCaptureResult{RhiStatus::InvalidHandle, 0U};
    }

    const RhiTargetSlot& slot = _targets[_presentedHandle.Slot];
    const std::size_t byteCount = slot.Bytes.size();
    if (destination.size() < byteCount)
    {
        RecordFailure(RhiStatus::CapacityExceeded);
        _snapshot.LastCaptureBytesWritten = 0U;
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    std::copy(slot.Bytes.begin(), slot.Bytes.end(), destination.begin());
    ++_snapshot.CaptureCount;
    _snapshot.LastCaptureBytesWritten = byteCount;
    return RhiCaptureResult{RhiStatus::Success, byteCount};
}

RhiCapabilities NullRhiDevice::Capabilities() const
{
    return _capabilities;
}

RhiDeviceSnapshot NullRhiDevice::Snapshot() const
{
    return _snapshot;
}

RhiStatus NullRhiDevice::RecordFailure(RhiStatus status)
{
    ++_snapshot.FailedOperationCount;
    return status;
}

bool NullRhiDevice::IsTargetHandleValid(RhiTextureHandle handle) const
{
    if (!_isInitialized)
    {
        return false;
    }

    if (handle.Generation == INVALID_GENERATION)
    {
        return false;
    }

    if (handle.Slot >= _targets.size())
    {
        return false;
    }

    const RhiTargetSlot& slot = _targets[handle.Slot];
    if (!slot.IsActive)
    {
        return false;
    }

    return slot.Generation == handle.Generation;
}

bool NullRhiDevice::IsColorTargetDescValid(const RhiColorTargetDesc& desc) const
{
    if (desc.Extent.Width == 0U)
    {
        return false;
    }

    if (desc.Extent.Height == 0U)
    {
        return false;
    }

    if (desc.Extent.Width > MAX_COLOR_TARGET_EXTENT)
    {
        return false;
    }

    if (desc.Extent.Height > MAX_COLOR_TARGET_EXTENT)
    {
        return false;
    }

    return true;
}

std::size_t NullRhiDevice::PixelByteCount(const RhiColorTargetDesc& desc) const
{
    return static_cast<std::size_t>(desc.Extent.Width) * static_cast<std::size_t>(desc.Extent.Height) * RGBA8_BYTES_PER_PIXEL;
}

void NullRhiDevice::ExecuteClear(RhiTextureHandle handle, RhiColor color)
{
    if (!IsTargetHandleValid(handle))
    {
        return;
    }

    RhiTargetSlot& slot = _targets[handle.Slot];
    for (std::size_t index = 0U; index < slot.Bytes.size(); index += RGBA8_BYTES_PER_PIXEL)
    {
        slot.Bytes[index] = color.R;
        slot.Bytes[index + 1U] = color.G;
        slot.Bytes[index + 2U] = color.B;
        slot.Bytes[index + 3U] = color.A;
    }
}
}
