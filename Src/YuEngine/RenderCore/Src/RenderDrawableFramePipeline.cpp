// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/RenderDrawableFramePipeline.cpp

#include "YuEngine/RenderCore/RenderDrawableFramePipeline.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderCore/MaterialBindingFixtureStatus.h"
#include "YuEngine/RenderCore/RenderDrawPacketRequest.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureRequest.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"
#include "YuEngine/RenderCore/RenderMaterialConstants.h"
#include "YuEngine/RenderCore/RenderMaterialRequest.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h"
#include "YuEngine/RenderCore/RenderViewPacketRequest.h"
#include "YuEngine/RenderCore/RenderViewPacketStatus.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferUsage.h"
#include "YuEngine/Rhi/RhiConstantBufferBinding.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiShaderStage.h"

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

std::size_t AlignConstantBufferByteCount(std::size_t byte_count) {
    const std::size_t remainder = byte_count % yuengine::rhi::RHI_CONSTANT_BUFFER_ALIGNMENT;
    if (remainder == 0U) {
        return byte_count;
    }

    return byte_count + yuengine::rhi::RHI_CONSTANT_BUFFER_ALIGNMENT - remainder;
}

yuengine::rhi::RhiStatus DestroyMaterialConstantBuffer(
    yuengine::rhi::IRhiDevice *device,
    yuengine::rhi::RhiBufferHandle handle,
    bool has_buffer) {
    if (!has_buffer) {
        return yuengine::rhi::RhiStatus::Success;
    }

    if (device == nullptr) {
        return yuengine::rhi::RhiStatus::InvalidHandle;
    }

    return device->DestroyBuffer(handle);
}

MaterialBindingFixtureStatus MaterialStatusToBindingStatus(RenderMaterialStatus status) {
    if (status == RenderMaterialStatus::Success) {
        return MaterialBindingFixtureStatus::Success;
    }

    if (status == RenderMaterialStatus::InvalidMaterialId) {
        return MaterialBindingFixtureStatus::InvalidMaterialId;
    }

    if (status == RenderMaterialStatus::InvalidPipeline) {
        return MaterialBindingFixtureStatus::InvalidPipeline;
    }

    if (status == RenderMaterialStatus::InvalidBlendState) {
        return MaterialBindingFixtureStatus::InvalidBlendState;
    }

    if (status == RenderMaterialStatus::InvalidTextureBinding) {
        return MaterialBindingFixtureStatus::InvalidTextureBinding;
    }

    if (status == RenderMaterialStatus::InvalidSamplerBinding) {
        return MaterialBindingFixtureStatus::InvalidSamplerBinding;
    }

    if (status == RenderMaterialStatus::OversizedConstants) {
        return MaterialBindingFixtureStatus::OversizedConstants;
    }

    if (status == RenderMaterialStatus::DuplicateMaterialId) {
        return MaterialBindingFixtureStatus::DuplicateMaterialId;
    }

    if (status == RenderMaterialStatus::MaterialCapacityExceeded) {
        return MaterialBindingFixtureStatus::BindingCapacityExceeded;
    }

    return MaterialBindingFixtureStatus::InvalidArgument;
}
}

RenderDrawableFramePipeline::RenderDrawableFramePipeline(
    const RenderDrawableFramePipelineDesc &desc)
    : desc_(NormalizeDesc(desc)),
      view_packet_(desc_.view_packet_desc),
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

    result.required_frame_record_count = snapshot_.frame_record_count + 1U;
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

    RenderViewPacketRequest view_request{};
    view_request.view_id = request.pass_id;
    view_request.frame_id = request.frame_id;
    view_request.target = target;
    view_request.clear_color = request.clear_color;
    view_request.capture_output = request.capture_output;
    view_request.capture_byte_budget = request.capture_byte_budget;
    view_request.material.material_id = request.material_id;
    view_request.material.program_id = request.material_id;
    view_request.material.pipeline = request.pipeline;
    view_request.material.blend_state = request.blend_state;
    view_request.material.sampled_texture = request.sampled_texture;
    view_request.material.sampler = request.sampler;
    view_request.material.sampled_textures = request.sampled_textures;
    view_request.material.samplers = request.samplers;
    view_request.material.constant_bytes = request.material_constant_bytes;
    view_request.material.pass_id = request.pass_id;
    view_request.draw.draw_id = request.pass_id;
    view_request.draw.pass_id = request.pass_id;
    view_request.draw.material_id = request.material_id;
    view_request.draw.vertex_buffer = request.vertex_buffer;
    view_request.draw.index_buffer = request.index_buffer;
    view_request.draw.draw = request.draw;

    RenderFixturePassRequest pass_request{};
    pass_request.rhi_device = request.rhi_device;
    result.view_result = view_packet_.BuildPassRequest(view_request, &pass_request);
    result.material_result.material_id = request.material_id;
    result.material_result.pass_id = request.pass_id;
    result.material_result.constant_byte_count = request.material_constant_bytes.size();
    result.material_result.status = MaterialStatusToBindingStatus(result.view_result.material_status);
    if (result.view_result.status != RenderViewPacketStatus::Success) {
        if (result.view_result.status != RenderViewPacketStatus::MaterialFailed) {
            result.status = RenderDrawableFramePipelineStatus::ViewPacketFailed;
            RecordViewPacketFailureResult(result);
            return result;
        }

        result.status = RenderDrawableFramePipelineStatus::MaterialBindingFailed;
        result.rhi_status = result.material_result.rhi_status;
        RecordMaterialFailureResult(result);
        return result;
    }

    std::array<std::uint8_t, MAX_RENDER_MATERIAL_CONSTANT_BYTES> material_constant_buffer_bytes{};
    std::array<yuengine::rhi::RhiConstantBufferBinding, 1U> material_constant_buffers{};
    yuengine::rhi::RhiBufferHandle material_constant_buffer{};
    bool has_material_constant_buffer = false;
    if (!request.material_constant_bytes.empty()) {
        const std::size_t aligned_constant_byte_count =
            AlignConstantBufferByteCount(request.material_constant_bytes.size());
        std::copy(
            request.material_constant_bytes.begin(),
            request.material_constant_bytes.end(),
            material_constant_buffer_bytes.begin());

        yuengine::rhi::RhiBufferDesc constant_buffer_desc{};
        constant_buffer_desc.usage = yuengine::rhi::RhiBufferUsage::Constant;
        constant_buffer_desc.size_bytes = aligned_constant_byte_count;
        const std::span<const std::uint8_t> initial_bytes(
            material_constant_buffer_bytes.data(),
            aligned_constant_byte_count);
        rhi_status = request.rhi_device->CreateBuffer(
            constant_buffer_desc,
            initial_bytes,
            material_constant_buffer);
        if (rhi_status != yuengine::rhi::RhiStatus::Success) {
            result.status = RenderDrawableFramePipelineStatus::RhiFailure;
            result.rhi_status = rhi_status;
            RecordRhiFailureResult(result);
            return result;
        }

        material_constant_buffers[0U].buffer = material_constant_buffer;
        material_constant_buffers[0U].stage = yuengine::rhi::RhiShaderStage::Pixel;
        material_constant_buffers[0U].slot = 0U;
        pass_request.constant_buffers =
            std::span<const yuengine::rhi::RhiConstantBufferBinding>(material_constant_buffers.data(), 1U);
        has_material_constant_buffer = true;
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
    result.capture_extent = result.pass_result.capture_extent;
    const yuengine::rhi::RhiStatus destroy_status = DestroyMaterialConstantBuffer(
        request.rhi_device,
        material_constant_buffer,
        has_material_constant_buffer);
    if (result.frame_result.status != RenderFramePacketFixtureStatus::Success) {
        result.status = RenderDrawableFramePipelineStatus::FramePacketFailed;
        RecordFrameFailureResult(result);
        return result;
    }

    if (destroy_status != yuengine::rhi::RhiStatus::Success) {
        result.status = RenderDrawableFramePipelineStatus::RhiFailure;
        result.rhi_status = destroy_status;
        RecordRhiFailureResult(result);
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
    view_packet_.Reset();
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

void RenderDrawableFramePipeline::RecordViewPacketFailureResult(
    const RenderDrawableFramePipelineResult &result) {
    StoreLastResult(result);
    ++snapshot_.view_packet_failure_count;
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
    snapshot_.last_required_frame_record_count = result.required_frame_record_count;
    snapshot_.last_recorded_command_count = result.recorded_command_count;
    snapshot_.last_capture_bytes_written = result.capture_bytes_written;
    snapshot_.last_capture_extent = result.capture_extent;
    snapshot_.last_status = result.status;
    snapshot_.last_rhi_status = result.rhi_status;
    snapshot_.last_swapchain_snapshot = result.swapchain_snapshot;
}
}
