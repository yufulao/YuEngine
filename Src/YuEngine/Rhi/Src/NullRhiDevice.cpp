#include "YuEngine/Rhi/NullRhiDevice.h"

#include <algorithm>

#include "YuEngine/Rhi/RhiConstants.h"

namespace yuengine::rhi {
namespace {
constexpr std::uint32_t INVALID_GENERATION = 0U;
}

NullRhiDevice::NullRhiDevice()
    : targets_(),
      capabilities_{},
      snapshot_{},
      submitted_handle_{},
      presented_handle_{},
      generation_seed_(INVALID_GENERATION),
      is_initialized_(false),
      has_submitted_frame_(false),
      has_presented_frame_(false) {
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

    ++generation_seed_;
    if (generation_seed_ == INVALID_GENERATION) {
        ++generation_seed_;
    }

    targets_.assign(desc.color_target_capacity, RhiTargetSlot{});
    for (RhiTargetSlot& target : targets_) {
        target.generation = generation_seed_;
    }

    capabilities_ = RhiCapabilities{
        RhiBackendKind::Null,
        RhiFormat::Rgba8Unorm,
        desc.color_target_capacity,
        desc.command_list_capacity,
        MAX_COLOR_TARGET_EXTENT,
        MAX_CAPTURE_FIXTURE_EXTENT,
        true};
    snapshot_ = RhiDeviceSnapshot{};
    snapshot_.color_target_capacity = desc.color_target_capacity;
    is_initialized_ = true;
    has_submitted_frame_ = false;
    has_presented_frame_ = false;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::CreateColorTarget(const RhiColorTargetDesc& desc, RhiTextureHandle& outHandle) {
    if (!is_initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (desc.format != RhiFormat::Rgba8Unorm) {
        return RecordFailure(RhiStatus::UnsupportedFormat);
    }

    if (!IsColorTargetDescValid(desc)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < targets_.size(); ++index) {
        RhiTargetSlot& slot = targets_[index];
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
        ++snapshot_.color_target_count;
        ++snapshot_.created_target_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus NullRhiDevice::DestroyTarget(RhiTextureHandle handle) {
    if (!IsTargetHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RhiTargetSlot& slot = targets_[handle.slot];
    slot.is_active = false;
    slot.bytes.clear();
    ++slot.generation;
    --snapshot_.color_target_count;
    ++snapshot_.destroyed_target_count;
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

    ++snapshot_.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::Submit(const RhiCommandList& commandList) {
    if (!commandList.IsComplete()) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (commandList.Capacity() > capabilities_.command_list_capacity) {
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

    snapshot_.command_storage_capacity_before_frame = commandList.Capacity();
    for (std::size_t index = 0U; index < commandList.CommandCount(); ++index) {
        const RhiCommandRecord& command = commandList.CommandAt(index);
        if (command.type == RhiCommandType::ClearColor) {
            ExecuteClear(command.target, command.color);
            continue;
        }
    }

    submitted_handle_ = target;
    has_submitted_frame_ = true;
    has_presented_frame_ = false;
    ++snapshot_.submit_count;
    snapshot_.command_storage_capacity_after_last_frame = commandList.Capacity();
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::Present() {
    if (!has_submitted_frame_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsTargetHandleValid(submitted_handle_)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    presented_handle_ = submitted_handle_;
    has_presented_frame_ = true;
    ++snapshot_.present_count;
    return RhiStatus::Success;
}

RhiCaptureResult NullRhiDevice::CapturePresentedTarget(std::span<std::uint8_t> destination) {
    if (!has_presented_frame_) {
        RecordFailure(RhiStatus::InvalidLifecycle);
        return RhiCaptureResult{RhiStatus::InvalidLifecycle, 0U};
    }

    if (!IsTargetHandleValid(presented_handle_)) {
        RecordFailure(RhiStatus::InvalidHandle);
        return RhiCaptureResult{RhiStatus::InvalidHandle, 0U};
    }

    const RhiTargetSlot& slot = targets_[presented_handle_.slot];
    if (slot.desc.extent.width > MAX_CAPTURE_FIXTURE_EXTENT || slot.desc.extent.height > MAX_CAPTURE_FIXTURE_EXTENT) {
        RecordFailure(RhiStatus::CapacityExceeded);
        snapshot_.last_capture_bytes_written = 0U;
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    const std::size_t byteCount = slot.bytes.size();
    if (destination.size() < byteCount) {
        RecordFailure(RhiStatus::CapacityExceeded);
        snapshot_.last_capture_bytes_written = 0U;
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    std::copy(slot.bytes.begin(), slot.bytes.end(), destination.begin());
    ++snapshot_.capture_count;
    snapshot_.last_capture_bytes_written = byteCount;
    return RhiCaptureResult{RhiStatus::Success, byteCount};
}

RhiCapabilities NullRhiDevice::Capabilities() const {
    return capabilities_;
}

RhiDeviceSnapshot NullRhiDevice::Snapshot() const {
    return snapshot_;
}

RhiStatus NullRhiDevice::RecordFailure(RhiStatus status) {
    ++snapshot_.failed_operation_count;
    return status;
}

bool NullRhiDevice::IsTargetHandleValid(RhiTextureHandle handle) const {
    if (!is_initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= targets_.size()) {
        return false;
    }

    const RhiTargetSlot& slot = targets_[handle.slot];
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

    RhiTargetSlot& slot = targets_[handle.slot];
    for (std::size_t index = 0U; index < slot.bytes.size(); index += RGBA8_BYTES_PER_PIXEL) {
        slot.bytes[index] = color.r;
        slot.bytes[index + 1U] = color.g;
        slot.bytes[index + 2U] = color.b;
        slot.bytes[index + 3U] = color.a;
    }
}
}
