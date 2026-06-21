// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/RenderSwapchainFramePipeline.cpp

#include "YuEngine/RenderCore/RenderSwapchainFramePipeline.h"

#include <cstddef>
#include <span>

#include "YuEngine/Rhi/RhiDeviceSnapshot.h"

namespace yuengine::rendercore {
namespace {
RenderSwapchainFramePipelineDesc NormalizeDesc(RenderSwapchainFramePipelineDesc desc) {
    if (desc.frame_record_capacity > MAX_RENDER_SWAPCHAIN_FRAME_RECORDS) {
        desc.frame_record_capacity = MAX_RENDER_SWAPCHAIN_FRAME_RECORDS;
    }

    if (desc.command_capacity > DEFAULT_RENDER_SWAPCHAIN_FRAME_COMMAND_CAPACITY) {
        desc.command_capacity = DEFAULT_RENDER_SWAPCHAIN_FRAME_COMMAND_CAPACITY;
    }

    return desc;
}

bool IsCaptureOutputValid(const RenderSwapchainFramePipelineRequest &request) {
    if (request.capture_byte_budget == 0U) {
        return false;
    }

    if (request.capture_output.data() == nullptr) {
        return false;
    }

    return request.capture_output.size() >= request.capture_byte_budget;
}

bool IsSwapchainValid(const yuengine::rhi::RhiSwapchainSnapshot &snapshot) {
    if (!snapshot.valid) {
        return false;
    }

    if (snapshot.extent.width == 0U) {
        return false;
    }

    if (snapshot.extent.height == 0U) {
        return false;
    }

    return snapshot.color_target.generation != 0U;
}
}

RenderSwapchainFramePipeline::RenderSwapchainFramePipeline(
    const RenderSwapchainFramePipelineDesc &desc)
    : desc_(NormalizeDesc(desc)),
      command_list_(desc_.command_capacity) {
    Reset();
}

RenderSwapchainFramePipelineResult RenderSwapchainFramePipeline::Execute(
    const RenderSwapchainFramePipelineRequest &request) {
    RenderSwapchainFramePipelineResult result{};
    result.frame_id = request.frame_id;
    result.resize_requested = request.resize_before_submit;

    result.status = ValidateRequest(request);
    if (result.status != RenderSwapchainFramePipelineStatus::Success) {
        RecordRejectedResult(result);
        return result;
    }

    if (desc_.command_capacity < RENDER_SWAPCHAIN_FRAME_COMMAND_COUNT) {
        result.status = RenderSwapchainFramePipelineStatus::CommandCapacityExceeded;
        RecordRejectedResult(result);
        return result;
    }

    if (!HasRecordCapacity()) {
        result.status = RenderSwapchainFramePipelineStatus::FrameRecordCapacityExceeded;
        RecordRejectedResult(result);
        return result;
    }

    if (request.resize_before_submit) {
        const yuengine::rhi::RhiStatus resize_status =
            request.rhi_device->ResizeSwapchain(request.resize_request, result.resize_result);
        result.rhi_status = resize_status;
        result.resized = result.resize_result.resized;
        result.swapchain_snapshot = result.resize_result.snapshot;
        if (resize_status != yuengine::rhi::RhiStatus::Success) {
            result.status = RenderSwapchainFramePipelineStatus::RhiFailure;
            RecordRhiFailureResult(result);
            return result;
        }
    }

    yuengine::rhi::RhiTextureHandle target{};
    yuengine::rhi::RhiStatus rhi_status = request.rhi_device->GetSwapchainColorTarget(target);
    result.target = target;
    result.rhi_status = rhi_status;
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RenderSwapchainFramePipelineStatus::RhiFailure;
        RecordRhiFailureResult(result);
        return result;
    }

    rhi_status = command_list_.Reset();
    result.rhi_status = rhi_status;
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RenderSwapchainFramePipelineStatus::RhiFailure;
        RecordRhiFailureResult(result);
        return result;
    }

    rhi_status = command_list_.BeginFrame(target);
    result.rhi_status = rhi_status;
    result.recorded_command_count = command_list_.CommandCount();
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RenderSwapchainFramePipelineStatus::RhiFailure;
        RecordRhiFailureResult(result);
        return result;
    }

    rhi_status = request.rhi_device->RecordClear(command_list_, target, request.clear_color);
    result.rhi_status = rhi_status;
    result.recorded_command_count = command_list_.CommandCount();
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RenderSwapchainFramePipelineStatus::RhiFailure;
        RecordRhiFailureResult(result);
        return result;
    }

    rhi_status = command_list_.EndFrame();
    result.rhi_status = rhi_status;
    result.recorded_command_count = command_list_.CommandCount();
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RenderSwapchainFramePipelineStatus::RhiFailure;
        RecordRhiFailureResult(result);
        return result;
    }

    rhi_status = request.rhi_device->Submit(command_list_);
    result.rhi_status = rhi_status;
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RenderSwapchainFramePipelineStatus::RhiFailure;
        RecordRhiFailureResult(result);
        return result;
    }

    rhi_status = request.rhi_device->Present();
    result.rhi_status = rhi_status;
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RenderSwapchainFramePipelineStatus::RhiFailure;
        RecordRhiFailureResult(result);
        return result;
    }

    std::span<std::uint8_t> capture_span(request.capture_output.data(), request.capture_byte_budget);
    const yuengine::rhi::RhiCaptureResult capture_result = request.rhi_device->CapturePresentedTarget(capture_span);
    result.rhi_status = capture_result.status;
    result.capture_bytes_written = capture_result.bytes_written;
    result.capture_extent = capture_result.extent;
    if (capture_result.status != yuengine::rhi::RhiStatus::Success) {
        result.status = RenderSwapchainFramePipelineStatus::RhiFailure;
        RecordRhiFailureResult(result);
        return result;
    }

    const yuengine::rhi::RhiDeviceSnapshot rhi_snapshot = request.rhi_device->Snapshot();
    result.swapchain_snapshot = rhi_snapshot.swapchain;
    result.status = RenderSwapchainFramePipelineStatus::Success;
    result.rhi_status = yuengine::rhi::RhiStatus::Success;
    RecordSuccessResult(result);
    return result;
}

RenderSwapchainFramePipelineSnapshot RenderSwapchainFramePipeline::Snapshot() const {
    return snapshot_;
}

void RenderSwapchainFramePipeline::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.frame_record_capacity = desc_.frame_record_capacity;
    snapshot_.command_capacity = desc_.command_capacity;
    snapshot_.required_command_count = RENDER_SWAPCHAIN_FRAME_COMMAND_COUNT;
}

RenderSwapchainFramePipelineStatus RenderSwapchainFramePipeline::ValidateRequest(
    const RenderSwapchainFramePipelineRequest &request) const {
    if (request.rhi_device == nullptr) {
        return RenderSwapchainFramePipelineStatus::InvalidArgument;
    }

    if (request.frame_id == 0U) {
        return RenderSwapchainFramePipelineStatus::InvalidFrameId;
    }

    if (HasFrameId(request.frame_id)) {
        return RenderSwapchainFramePipelineStatus::DuplicateFrameId;
    }

    if (!IsCaptureOutputValid(request)) {
        return RenderSwapchainFramePipelineStatus::InsufficientCaptureStorage;
    }

    const yuengine::rhi::RhiDeviceSnapshot rhi_snapshot = request.rhi_device->Snapshot();
    if (!IsSwapchainValid(rhi_snapshot.swapchain)) {
        return RenderSwapchainFramePipelineStatus::InvalidSwapchain;
    }

    return RenderSwapchainFramePipelineStatus::Success;
}

bool RenderSwapchainFramePipeline::HasFrameId(std::uint32_t frame_id) const {
    for (std::size_t index = 0U; index < snapshot_.frame_record_count; ++index) {
        if (records_[index].result.frame_id == frame_id) {
            return true;
        }
    }

    return false;
}

bool RenderSwapchainFramePipeline::HasRecordCapacity() const {
    return snapshot_.frame_record_count < desc_.frame_record_capacity;
}

void RenderSwapchainFramePipeline::RecordRejectedResult(
    const RenderSwapchainFramePipelineResult &result) {
    StoreLastResult(result);

    if (result.status == RenderSwapchainFramePipelineStatus::DuplicateFrameId) {
        ++snapshot_.duplicate_frame_id_count;
        return;
    }

    if (result.status == RenderSwapchainFramePipelineStatus::CommandCapacityExceeded) {
        ++snapshot_.command_capacity_rejected_count;
        return;
    }

    if (result.status == RenderSwapchainFramePipelineStatus::FrameRecordCapacityExceeded) {
        ++snapshot_.frame_record_capacity_rejected_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}

void RenderSwapchainFramePipeline::RecordRhiFailureResult(
    const RenderSwapchainFramePipelineResult &result) {
    if (snapshot_.frame_record_count < records_.size()) {
        records_[snapshot_.frame_record_count].result = result;
    }

    ++snapshot_.frame_record_count;
    ++snapshot_.accepted_frame_count;
    ++snapshot_.rhi_failure_count;
    if (result.resize_requested) {
        ++snapshot_.resize_request_count;
    }

    StoreLastResult(result);
}

void RenderSwapchainFramePipeline::RecordSuccessResult(
    const RenderSwapchainFramePipelineResult &result) {
    if (snapshot_.frame_record_count < records_.size()) {
        records_[snapshot_.frame_record_count].result = result;
    }

    ++snapshot_.frame_record_count;
    ++snapshot_.accepted_frame_count;
    ++snapshot_.completed_frame_count;
    if (result.resize_requested) {
        ++snapshot_.resize_request_count;
    }

    if (result.resized) {
        ++snapshot_.resized_frame_count;
    }

    if (result.capture_bytes_written != 0U) {
        ++snapshot_.capture_count;
    }

    StoreLastResult(result);
}

void RenderSwapchainFramePipeline::StoreLastResult(
    const RenderSwapchainFramePipelineResult &result) {
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_recorded_command_count = result.recorded_command_count;
    snapshot_.last_capture_bytes_written = result.capture_bytes_written;
    snapshot_.last_capture_extent = result.capture_extent;
    snapshot_.last_status = result.status;
    snapshot_.last_rhi_status = result.rhi_status;
    snapshot_.last_swapchain_snapshot = result.swapchain_snapshot;
}
}
