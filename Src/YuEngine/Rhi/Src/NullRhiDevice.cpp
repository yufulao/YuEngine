#include "YuEngine/Rhi/NullRhiDevice.h"

#include <algorithm>

#include "YuEngine/Rhi/RhiConstants.h"

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

RhiStatus NullRhiDevice::Initialize(const RhiDeviceDesc& desc) {
    if (desc.backend_kind != RhiBackendKind::Null) {
        return RhiStatus::UnsupportedBackend;
    }

    if (desc.color_target_capacity == 0U) {
        return RhiStatus::InvalidDescriptor;
    }

    if (desc.color_target_capacity > MAX_COLOR_TARGETS) {
        return RhiStatus::CapacityExceeded;
    }

    if (desc.command_list_capacity == 0U) {
        return RhiStatus::InvalidDescriptor;
    }

    if (desc.command_list_capacity > MAX_COMMANDS) {
        return RhiStatus::CapacityExceeded;
    }

    ++_generationSeed;
    if (_generationSeed == INVALID_GENERATION) {
        ++_generationSeed;
    }

    _targets.assign(desc.color_target_capacity, RhiTargetSlot{});
    for (RhiTargetSlot& target : _targets) {
        target.generation = _generationSeed;
    }

    _capabilities = RhiCapabilities{
        RhiBackendKind::Null,
        RhiFormat::Rgba8Unorm,
        desc.color_target_capacity,
        desc.command_list_capacity,
        MAX_COLOR_TARGET_EXTENT,
        MAX_CAPTURE_FIXTURE_EXTENT,
        true};
    _snapshot = RhiDeviceSnapshot{};
    _snapshot.color_target_capacity = desc.color_target_capacity;
    _isInitialized = true;
    _hasSubmittedFrame = false;
    _hasPresentedFrame = false;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::CreateColorTarget(const RhiColorTargetDesc& desc, RhiTextureHandle& outHandle) {
    if (!_isInitialized) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (desc.format != RhiFormat::Rgba8Unorm) {
        return RecordFailure(RhiStatus::UnsupportedFormat);
    }

    if (!IsColorTargetDescValid(desc)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < _targets.size(); ++index) {
        RhiTargetSlot& slot = _targets[index];
        if (slot.is_active) {
            continue;
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.is_active = true;
        slot.desc = desc;
        slot.bytes.assign(PixelByteCount(desc), 0U);
        outHandle = RhiTextureHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++_snapshot.color_target_count;
        ++_snapshot.created_target_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus NullRhiDevice::DestroyTarget(RhiTextureHandle handle) {
    if (!IsTargetHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RhiTargetSlot& slot = _targets[handle.slot];
    slot.is_active = false;
    slot.bytes.clear();
    ++slot.generation;
    --_snapshot.color_target_count;
    ++_snapshot.destroyed_target_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RecordClear(RhiCommandList& commandList, RhiTextureHandle handle, RhiColor color) {
    if (!IsTargetHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    const RhiStatus status = commandList.RecordClear(handle, color);
    if (status != RhiStatus::Success) {
        return RecordFailure(status);
    }

    ++_snapshot.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::Submit(const RhiCommandList& commandList) {
    if (!commandList.IsComplete()) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (commandList.Capacity() > _capabilities.command_list_capacity) {
        return RecordFailure(RhiStatus::CapacityExceeded);
    }

    const RhiTextureHandle target = commandList.TargetHandle();
    if (!IsTargetHandleValid(target)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    for (std::size_t index = 0U; index < commandList.CommandCount(); ++index) {
        const RhiCommandRecord& command = commandList.CommandAt(index);
        if (!IsCommandTargetValidForFrame(command, target)) {
            return RecordFailure(RhiStatus::InvalidHandle);
        }
    }

    _snapshot.command_storage_capacity_before_frame = commandList.Capacity();
    for (std::size_t index = 0U; index < commandList.CommandCount(); ++index) {
        const RhiCommandRecord& command = commandList.CommandAt(index);
        if (command.type == RhiCommandType::ClearColor) {
            ExecuteClear(command.target, command.color);
            continue;
        }
    }

    _submittedHandle = target;
    _hasSubmittedFrame = true;
    _hasPresentedFrame = false;
    ++_snapshot.submit_count;
    _snapshot.command_storage_capacity_after_last_frame = commandList.Capacity();
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::Present() {
    if (!_hasSubmittedFrame) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsTargetHandleValid(_submittedHandle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    _presentedHandle = _submittedHandle;
    _hasPresentedFrame = true;
    ++_snapshot.present_count;
    return RhiStatus::Success;
}

RhiCaptureResult NullRhiDevice::CapturePresentedTarget(std::span<std::uint8_t> destination) {
    if (!_hasPresentedFrame) {
        RecordFailure(RhiStatus::InvalidLifecycle);
        return RhiCaptureResult{RhiStatus::InvalidLifecycle, 0U};
    }

    if (!IsTargetHandleValid(_presentedHandle)) {
        RecordFailure(RhiStatus::InvalidHandle);
        return RhiCaptureResult{RhiStatus::InvalidHandle, 0U};
    }

    const RhiTargetSlot& slot = _targets[_presentedHandle.slot];
    if (slot.desc.extent.width > MAX_CAPTURE_FIXTURE_EXTENT || slot.desc.extent.height > MAX_CAPTURE_FIXTURE_EXTENT) {
        RecordFailure(RhiStatus::CapacityExceeded);
        _snapshot.last_capture_bytes_written = 0U;
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    const std::size_t byteCount = slot.bytes.size();
    if (destination.size() < byteCount) {
        RecordFailure(RhiStatus::CapacityExceeded);
        _snapshot.last_capture_bytes_written = 0U;
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    std::copy(slot.bytes.begin(), slot.bytes.end(), destination.begin());
    ++_snapshot.capture_count;
    _snapshot.last_capture_bytes_written = byteCount;
    return RhiCaptureResult{RhiStatus::Success, byteCount};
}

RhiCapabilities NullRhiDevice::Capabilities() const {
    return _capabilities;
}

RhiDeviceSnapshot NullRhiDevice::Snapshot() const {
    return _snapshot;
}

RhiStatus NullRhiDevice::RecordFailure(RhiStatus status) {
    ++_snapshot.failed_operation_count;
    return status;
}

bool NullRhiDevice::IsTargetHandleValid(RhiTextureHandle handle) const {
    if (!_isInitialized) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= _targets.size()) {
        return false;
    }

    const RhiTargetSlot& slot = _targets[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool NullRhiDevice::IsCommandTargetValidForFrame(const RhiCommandRecord& command, RhiTextureHandle frameTarget) const {
    if (command.type != RhiCommandType::ClearColor) {
        return true;
    }

    if (command.target.slot != frameTarget.slot) {
        return false;
    }

    if (command.target.generation != frameTarget.generation) {
        return false;
    }

    return IsTargetHandleValid(command.target);
}

bool NullRhiDevice::IsColorTargetDescValid(const RhiColorTargetDesc& desc) const {
    if (desc.extent.width == 0U) {
        return false;
    }

    if (desc.extent.height == 0U) {
        return false;
    }

    if (desc.extent.width > MAX_COLOR_TARGET_EXTENT) {
        return false;
    }

    if (desc.extent.height > MAX_COLOR_TARGET_EXTENT) {
        return false;
    }

    return true;
}

std::size_t NullRhiDevice::PixelByteCount(const RhiColorTargetDesc& desc) const {
    return static_cast<std::size_t>(desc.extent.width) * static_cast<std::size_t>(desc.extent.height) * RGBA8_BYTES_PER_PIXEL;
}

void NullRhiDevice::ExecuteClear(RhiTextureHandle handle, RhiColor color) {
    if (!IsTargetHandleValid(handle)) {
        return;
    }

    RhiTargetSlot& slot = _targets[handle.slot];
    for (std::size_t index = 0U; index < slot.bytes.size(); index += RGBA8_BYTES_PER_PIXEL) {
        slot.bytes[index] = color.r;
        slot.bytes[index + 1U] = color.g;
        slot.bytes[index + 2U] = color.b;
        slot.bytes[index + 3U] = color.a;
    }
}
}
