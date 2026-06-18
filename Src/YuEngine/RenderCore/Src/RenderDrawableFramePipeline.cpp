// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Src/RenderDrawableFramePipeline.cpp

#include "YuEngine/RenderCore/RenderDrawableFramePipeline.h"

#include <array>
#include <cstddef>
#include <span>

#include "YuEngine/RenderCore/MaterialBindingFixtureRequest.h"
#include "YuEngine/RenderCore/MaterialBindingFixtureStatus.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureRequest.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"

namespace yuengine::rendercore {
namespace {
RenderDrawableFramePipelineDesc NormalizeDesc(RenderDrawableFramePipelineDesc desc) {
    if (desc.frame_record_capacity > MAX_RENDER_DRAWABLE_FRAME_RECORDS) {
        desc.frame_record_capacity = MAX_RENDER_DRAWABLE_FRAME_RECORDS;
    }

    return desc;
}

bool IsCaptureOutputValid(const RenderDrawableFramePipelineRequest &request) {
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

RenderDrawableFramePipeline::RenderDrawableFramePipeline(
    const RenderDrawableFramePipelineDesc &desc)
    : desc_(NormalizeDesc(desc)),
      material_binding_(desc_.material_binding_desc),
      fixture_pass_(desc_.fixture_pass_desc),
      submission_batch_(desc_.submission_batch_desc),
      frame_packet_(desc_.frame_packet_desc) {
    Reset();
}

RenderDrawableFramePipelineResult RenderDrawableFramePipeline::Execute(
    const RenderDrawableFramePipelineRequest &request) {
    RenderDrawableFramePipelineResult result{};
    result.frame_id = request.frame_id;
    result.pass_id = request.pass_id;
    result.material_id = request.material_id;

    result.status = ValidateRequest(request);
    if (result.status != RenderDrawableFramePipelineStatus::Success) {
        RecordRejectedResult(result);
        return result;
    }

    if (!HasRecordCapacity()) {
        result.status = RenderDrawableFramePipelineStatus::FrameRecordCapacityExceeded;
        RecordRejectedResult(result);
        return result;
    }

    yuengine::rhi::RhiTextureHandle target{};
    yuengine::rhi::RhiStatus rhi_status = request.rhi_device->GetSwapchainColorTarget(target);
    result.target = target;
    result.rhi_status = rhi_status;
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RenderDrawableFramePipelineStatus::RhiFailure;
        RecordRhiFailureResult(result);
        return result;
    }

    RenderFixturePassRequest pass_request{};
    pass_request.rhi_device = request.rhi_device;
    pass_request.target = target;
    pass_request.vertex_buffer = request.vertex_buffer;
    pass_request.index_buffer = request.index_buffer;
    pass_request.draw = request.draw;
    pass_request.clear_color = request.clear_color;
    pass_request.capture_output = request.capture_output;
    pass_request.capture_byte_budget = request.capture_byte_budget;

    MaterialBindingFixtureRequest material_request{};
    material_request.material_id = request.material_id;
    material_request.pipeline = request.pipeline;
    material_request.sampled_texture = request.sampled_texture;
    material_request.sampler = request.sampler;
    material_request.constant_bytes = request.material_constant_bytes;
    material_request.pass_id = request.pass_id;

    result.material_result = material_binding_.Bind(material_request, &pass_request);
    if (result.material_result.status != MaterialBindingFixtureStatus::Success) {
        result.status = RenderDrawableFramePipelineStatus::MaterialBindingFailed;
        result.rhi_status = result.material_result.rhi_status;
        RecordMaterialFailureResult(result);
        return result;
    }

    std::array<RenderFixturePassRequest, RENDER_DRAWABLE_FRAME_PASS_COUNT> pass_requests{};
    std::array<RenderFixturePassResult, RENDER_DRAWABLE_FRAME_PASS_COUNT> pass_results{};
    pass_requests[0U] = pass_request;

    RenderSubmissionBatchFixtureRequest batch_request{};
    batch_request.pass = &fixture_pass_;
    batch_request.pass_requests = std::span<const RenderFixturePassRequest>(pass_requests.data(), pass_requests.size());
    batch_request.pass_results = std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size());

    RenderFramePacketFixtureRequest frame_request{};
    frame_request.frame_id = request.frame_id;
    frame_request.submission_batch = &submission_batch_;
    frame_request.batch_request = &batch_request;

    result.frame_result = frame_packet_.Execute(frame_request);
    result.pass_result = pass_results[0U];
    result.rhi_status = result.frame_result.rhi_status;
    result.recorded_command_count = result.pass_result.recorded_command_count;
    result.capture_bytes_written = result.pass_result.capture_bytes_written;
    if (result.frame_result.status != RenderFramePacketFixtureStatus::Success) {
        result.status = RenderDrawableFramePipelineStatus::FramePacketFailed;
        RecordFrameFailureResult(result);
        return result;
    }

    const yuengine::rhi::RhiDeviceSnapshot rhi_snapshot = request.rhi_device->Snapshot();
    result.swapchain_snapshot = rhi_snapshot.swapchain;
    result.status = RenderDrawableFramePipelineStatus::Success;
    result.rhi_status = yuengine::rhi::RhiStatus::Success;
    RecordSuccessResult(result);
    return result;
}

RenderDrawableFramePipelineSnapshot RenderDrawableFramePipeline::Snapshot() const {
    return snapshot_;
}

void RenderDrawableFramePipeline::Reset() {
    records_ = {};
    material_binding_.Reset();
    fixture_pass_.Reset();
    submission_batch_.Reset();
    frame_packet_.Reset();
    snapshot_ = {};
    snapshot_.frame_record_capacity = desc_.frame_record_capacity;
}

RenderDrawableFramePipelineStatus RenderDrawableFramePipeline::ValidateRequest(
    const RenderDrawableFramePipelineRequest &request) const {
    if (request.rhi_device == nullptr) {
        return RenderDrawableFramePipelineStatus::InvalidArgument;
    }

    if (request.frame_id == 0U) {
        return RenderDrawableFramePipelineStatus::InvalidFrameId;
    }

    if (request.pass_id == 0U) {
        return RenderDrawableFramePipelineStatus::InvalidPassId;
    }

    if (request.material_id == 0U) {
        return RenderDrawableFramePipelineStatus::InvalidMaterialId;
    }

    if (!IsCaptureOutputValid(request)) {
        return RenderDrawableFramePipelineStatus::InvalidArgument;
    }

    const yuengine::rhi::RhiDeviceSnapshot rhi_snapshot = request.rhi_device->Snapshot();
    if (!IsSwapchainValid(rhi_snapshot.swapchain)) {
        return RenderDrawableFramePipelineStatus::InvalidSwapchain;
    }

    return RenderDrawableFramePipelineStatus::Success;
}

bool RenderDrawableFramePipeline::HasRecordCapacity() const {
    return snapshot_.frame_record_count < desc_.frame_record_capacity;
}

void RenderDrawableFramePipeline::RecordRejectedResult(
    const RenderDrawableFramePipelineResult &result) {
    StoreLastResult(result);

    if (result.status == RenderDrawableFramePipelineStatus::FrameRecordCapacityExceeded) {
        ++snapshot_.frame_record_capacity_rejected_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}

void RenderDrawableFramePipeline::RecordRhiFailureResult(
    const RenderDrawableFramePipelineResult &result) {
    if (snapshot_.frame_record_count < records_.size()) {
        records_[snapshot_.frame_record_count].result = result;
    }

    ++snapshot_.frame_record_count;
    ++snapshot_.accepted_frame_count;
    ++snapshot_.rhi_failure_count;
    StoreLastResult(result);
}

void RenderDrawableFramePipeline::RecordMaterialFailureResult(
    const RenderDrawableFramePipelineResult &result) {
    StoreLastResult(result);
    ++snapshot_.material_failure_count;
}

void RenderDrawableFramePipeline::RecordFrameFailureResult(
    const RenderDrawableFramePipelineResult &result) {
    if (snapshot_.frame_record_count < records_.size()) {
        records_[snapshot_.frame_record_count].result = result;
    }

    ++snapshot_.frame_record_count;
    ++snapshot_.accepted_frame_count;
    ++snapshot_.frame_packet_failure_count;
    StoreLastResult(result);
}

void RenderDrawableFramePipeline::RecordSuccessResult(
    const RenderDrawableFramePipelineResult &result) {
    if (snapshot_.frame_record_count < records_.size()) {
        records_[snapshot_.frame_record_count].result = result;
    }

    ++snapshot_.frame_record_count;
    ++snapshot_.accepted_frame_count;
    ++snapshot_.completed_frame_count;
    if (result.capture_bytes_written != 0U) {
        ++snapshot_.capture_count;
    }

    StoreLastResult(result);
}

void RenderDrawableFramePipeline::StoreLastResult(
    const RenderDrawableFramePipelineResult &result) {
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_material_id = result.material_id;
    snapshot_.last_recorded_command_count = result.recorded_command_count;
    snapshot_.last_capture_bytes_written = result.capture_bytes_written;
    snapshot_.last_status = result.status;
    snapshot_.last_rhi_status = result.rhi_status;
    snapshot_.last_swapchain_snapshot = result.swapchain_snapshot;
}
}
