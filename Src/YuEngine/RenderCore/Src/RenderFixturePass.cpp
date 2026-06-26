// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/RenderFixturePass.cpp

#include "YuEngine/RenderCore/RenderFixturePass.h"

#include <cstddef>
#include <span>

#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
namespace {
bool IsTextureHandleSet(yuengine::rhi::RhiTextureHandle handle) {
    return handle.generation != 0U;
}

RenderFixturePassDesc NormalizeDesc(RenderFixturePassDesc desc) {
    if (desc.pass_record_capacity > MAX_RENDER_FIXTURE_PASS_RECORDS) {
        desc.pass_record_capacity = MAX_RENDER_FIXTURE_PASS_RECORDS;
    }

    if (desc.command_capacity > DEFAULT_RENDER_FIXTURE_PASS_COMMAND_CAPACITY) {
        desc.command_capacity = DEFAULT_RENDER_FIXTURE_PASS_COMMAND_CAPACITY;
    }

    return desc;
}

bool IsBufferHandleSet(yuengine::rhi::RhiBufferHandle handle) {
    return handle.generation != 0U;
}

bool IsPipelineHandleSet(yuengine::rhi::RhiPipelineHandle handle) {
    return handle.generation != 0U;
}

bool IsSamplerHandleSet(yuengine::rhi::RhiSamplerHandle handle) {
    return handle.generation != 0U;
}

bool IsConstantBufferShaderStageValid(yuengine::rhi::RhiShaderStage stage) {
    if (stage == yuengine::rhi::RhiShaderStage::Vertex) {
        return true;
    }

    return stage == yuengine::rhi::RhiShaderStage::Pixel;
}

bool IsVertexBufferViewValid(const yuengine::rhi::RhiVertexBufferView &view) {
    if (!IsBufferHandleSet(view.buffer)) {
        return false;
    }

    if (view.stride_bytes == 0U) {
        return false;
    }

    return view.size_bytes != 0U;
}

bool IsIndexBufferViewValid(const yuengine::rhi::RhiIndexBufferView &view) {
    if (!IsBufferHandleSet(view.buffer)) {
        return false;
    }

    if (view.format == yuengine::rhi::RhiIndexFormat::Unsupported) {
        return false;
    }

    return view.size_bytes != 0U;
}

bool IsSampledTextureBindingValid(const yuengine::rhi::RhiSampledTextureBinding &binding) {
    if (!IsTextureHandleSet(binding.texture)) {
        return false;
    }

    return binding.slot < yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS;
}

bool IsSamplerBindingValid(const yuengine::rhi::RhiSamplerBinding &binding) {
    if (!IsSamplerHandleSet(binding.sampler)) {
        return false;
    }

    return binding.slot < yuengine::rhi::MAX_RHI_SAMPLER_SLOTS;
}

bool IsConstantBufferBindingValid(const yuengine::rhi::RhiConstantBufferBinding &binding) {
    if (!IsBufferHandleSet(binding.buffer)) {
        return false;
    }

    if (!IsConstantBufferShaderStageValid(binding.stage)) {
        return false;
    }

    return binding.slot < yuengine::rhi::MAX_RHI_CONSTANT_BUFFER_SLOTS;
}

bool UsesBindingSpans(const RenderFixturePassRequest &request) {
    if (!request.sampled_textures.empty()) {
        return true;
    }

    return !request.samplers.empty();
}

std::span<const yuengine::rhi::RhiSampledTextureBinding> SampledTextureBindings(
    const RenderFixturePassRequest &request) {
    if (!request.sampled_textures.empty()) {
        return request.sampled_textures;
    }

    return std::span<const yuengine::rhi::RhiSampledTextureBinding>(&request.sampled_texture, 1U);
}

std::span<const yuengine::rhi::RhiSamplerBinding> SamplerBindings(
    const RenderFixturePassRequest &request) {
    if (!request.samplers.empty()) {
        return request.samplers;
    }

    return std::span<const yuengine::rhi::RhiSamplerBinding>(&request.sampler, 1U);
}

std::size_t RequiredCommandCount(const RenderFixturePassRequest &request) {
    const std::size_t binding_count = SampledTextureBindings(request).size();
    const std::size_t extra_binding_count = binding_count > 0U ? binding_count - 1U : 0U;
    return RENDER_FIXTURE_PASS_COMMAND_COUNT + extra_binding_count * 2U + request.constant_buffers.size();
}

bool AreBindingSpansValid(const RenderFixturePassRequest &request) {
    if (!UsesBindingSpans(request)) {
        return true;
    }

    if (request.sampled_textures.data() == nullptr) {
        return false;
    }

    if (request.samplers.data() == nullptr) {
        return false;
    }

    if (request.sampled_textures.size() == 0U) {
        return false;
    }

    if (request.sampled_textures.size() != request.samplers.size()) {
        return false;
    }

    if (request.sampled_textures.size() > yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS) {
        return false;
    }

    if (request.samplers.size() > yuengine::rhi::MAX_RHI_SAMPLER_SLOTS) {
        return false;
    }

    return true;
}

bool AreConstantBufferBindingsValid(const RenderFixturePassRequest &request) {
    if (request.constant_buffers.empty()) {
        return true;
    }

    if (request.constant_buffers.data() == nullptr) {
        return false;
    }

    if (request.constant_buffers.size() > yuengine::rhi::MAX_RHI_CONSTANT_BUFFER_SLOTS) {
        return false;
    }

    for (const yuengine::rhi::RhiConstantBufferBinding &binding : request.constant_buffers) {
        if (!IsConstantBufferBindingValid(binding)) {
            return false;
        }
    }

    return true;
}

bool IsDrawValid(const yuengine::rhi::RhiDrawIndexedDesc &draw) {
    if (draw.topology == yuengine::rhi::RhiPrimitiveTopology::Unsupported) {
        return false;
    }

    return draw.index_count != 0U;
}

bool IsCaptureOutputValid(const RenderFixturePassRequest &request) {
    if (request.capture_byte_budget == 0U) {
        return false;
    }

    if (request.capture_output.data() == nullptr) {
        return false;
    }

    return request.capture_output.size() >= request.capture_byte_budget;
}
}

RenderFixturePass::RenderFixturePass(const RenderFixturePassDesc &desc)
    : desc_(NormalizeDesc(desc)),
      command_list_(desc_.command_capacity) {
    Reset();
}

RenderFixturePassResult RenderFixturePass::Execute(const RenderFixturePassRequest &request) {
    RenderFixturePassResult result{};
    result.pass_id = request.pass_id;

    result.status = ValidateRequest(request);
    if (result.status != RenderFixturePassStatus::Success) {
        RecordRejectedResult(result);
        return result;
    }

    snapshot_.required_command_count = RequiredCommandCount(request);
    if (desc_.command_capacity < snapshot_.required_command_count) {
        result.status = RenderFixturePassStatus::CommandCapacityExceeded;
        RecordRejectedResult(result);
        return result;
    }

    if (!HasRecordCapacity()) {
        result.status = RenderFixturePassStatus::PassRecordCapacityExceeded;
        RecordRejectedResult(result);
        return result;
    }

    yuengine::rhi::RhiStatus rhi_status = command_list_.Reset();
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = rhi_status;
        RecordRhiFailureResult(&result);
        return result;
    }

    rhi_status = command_list_.BeginFrame(request.target);
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = rhi_status;
        result.recorded_command_count = command_list_.CommandCount();
        RecordRhiFailureResult(&result);
        return result;
    }

    rhi_status = request.rhi_device->RecordClear(command_list_, request.target, request.clear_color);
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = rhi_status;
        result.recorded_command_count = command_list_.CommandCount();
        RecordRhiFailureResult(&result);
        return result;
    }

    rhi_status = request.rhi_device->RecordBindPipeline(command_list_, request.pipeline);
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = rhi_status;
        result.recorded_command_count = command_list_.CommandCount();
        RecordRhiFailureResult(&result);
        return result;
    }

    rhi_status = request.rhi_device->RecordBindVertexBuffer(command_list_, request.vertex_buffer);
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = rhi_status;
        result.recorded_command_count = command_list_.CommandCount();
        RecordRhiFailureResult(&result);
        return result;
    }

    rhi_status = request.rhi_device->RecordBindIndexBuffer(command_list_, request.index_buffer);
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = rhi_status;
        result.recorded_command_count = command_list_.CommandCount();
        RecordRhiFailureResult(&result);
        return result;
    }

    const std::span<const yuengine::rhi::RhiSampledTextureBinding> sampled_textures =
        SampledTextureBindings(request);
    for (const yuengine::rhi::RhiSampledTextureBinding &binding : sampled_textures) {
        rhi_status = request.rhi_device->RecordBindSampledTexture(command_list_, binding);
        if (rhi_status != yuengine::rhi::RhiStatus::Success) {
            result.rhi_status = rhi_status;
            result.recorded_command_count = command_list_.CommandCount();
            RecordRhiFailureResult(&result);
            return result;
        }
    }

    const std::span<const yuengine::rhi::RhiSamplerBinding> samplers = SamplerBindings(request);
    for (const yuengine::rhi::RhiSamplerBinding &binding : samplers) {
        rhi_status = request.rhi_device->RecordBindSampler(command_list_, binding);
        if (rhi_status != yuengine::rhi::RhiStatus::Success) {
            result.rhi_status = rhi_status;
            result.recorded_command_count = command_list_.CommandCount();
            RecordRhiFailureResult(&result);
            return result;
        }
    }

    for (const yuengine::rhi::RhiConstantBufferBinding &binding : request.constant_buffers) {
        rhi_status = request.rhi_device->RecordBindConstantBuffer(command_list_, binding);
        if (rhi_status != yuengine::rhi::RhiStatus::Success) {
            result.rhi_status = rhi_status;
            result.recorded_command_count = command_list_.CommandCount();
            RecordRhiFailureResult(&result);
            return result;
        }
    }

    rhi_status = request.rhi_device->RecordDrawIndexed(command_list_, request.draw);
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = rhi_status;
        result.recorded_command_count = command_list_.CommandCount();
        RecordRhiFailureResult(&result);
        return result;
    }

    rhi_status = command_list_.EndFrame();
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = rhi_status;
        result.recorded_command_count = command_list_.CommandCount();
        RecordRhiFailureResult(&result);
        return result;
    }

    result.recorded_command_count = command_list_.CommandCount();
    rhi_status = request.rhi_device->Submit(command_list_);
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = rhi_status;
        RecordRhiFailureResult(&result);
        return result;
    }

    rhi_status = request.rhi_device->Present();
    if (rhi_status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = rhi_status;
        RecordRhiFailureResult(&result);
        return result;
    }

    std::span<std::uint8_t> capture_span(request.capture_output.data(), request.capture_byte_budget);
    const yuengine::rhi::RhiCaptureResult capture_result = request.rhi_device->CapturePresentedTarget(capture_span);
    if (capture_result.status != yuengine::rhi::RhiStatus::Success) {
        result.rhi_status = capture_result.status;
        result.capture_bytes_written = capture_result.bytes_written;
        result.capture_extent = capture_result.extent;
        RecordRhiFailureResult(&result);
        return result;
    }

    result.status = RenderFixturePassStatus::Success;
    result.rhi_status = yuengine::rhi::RhiStatus::Success;
    result.capture_bytes_written = capture_result.bytes_written;
    result.capture_extent = capture_result.extent;
    RecordSuccessResult(result);
    return result;
}

RenderFixturePassSnapshot RenderFixturePass::Snapshot() const {
    return snapshot_;
}

void RenderFixturePass::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.pass_record_capacity = desc_.pass_record_capacity;
    snapshot_.command_capacity = desc_.command_capacity;
    snapshot_.required_command_count = RENDER_FIXTURE_PASS_COMMAND_COUNT;
}

RenderFixturePassStatus RenderFixturePass::ValidateRequest(const RenderFixturePassRequest &request) const {
    if (request.rhi_device == nullptr) {
        return RenderFixturePassStatus::InvalidArgument;
    }

    if (!IsTextureHandleSet(request.target)) {
        return RenderFixturePassStatus::InvalidTarget;
    }

    if (!IsPipelineHandleSet(request.pipeline)) {
        return RenderFixturePassStatus::InvalidPipeline;
    }

    if (!IsVertexBufferViewValid(request.vertex_buffer)) {
        return RenderFixturePassStatus::MissingVertexBuffer;
    }

    if (!IsIndexBufferViewValid(request.index_buffer)) {
        return RenderFixturePassStatus::MissingIndexBuffer;
    }

    if (!AreBindingSpansValid(request)) {
        return RenderFixturePassStatus::InvalidTextureBinding;
    }

    const std::span<const yuengine::rhi::RhiSampledTextureBinding> sampled_textures =
        SampledTextureBindings(request);
    const std::span<const yuengine::rhi::RhiSamplerBinding> samplers = SamplerBindings(request);
    for (const yuengine::rhi::RhiSampledTextureBinding &binding : sampled_textures) {
        if (!IsSampledTextureBindingValid(binding)) {
            return RenderFixturePassStatus::InvalidTextureBinding;
        }
    }

    for (const yuengine::rhi::RhiSamplerBinding &binding : samplers) {
        if (!IsSamplerBindingValid(binding)) {
            return RenderFixturePassStatus::InvalidSamplerBinding;
        }
    }

    if (sampled_textures.size() != samplers.size()) {
        return RenderFixturePassStatus::InvalidSamplerBinding;
    }

    if (!AreConstantBufferBindingsValid(request)) {
        return RenderFixturePassStatus::InvalidConstantBufferBinding;
    }

    if (!IsDrawValid(request.draw)) {
        return RenderFixturePassStatus::InvalidDraw;
    }

    if (!IsCaptureOutputValid(request)) {
        return RenderFixturePassStatus::InsufficientCaptureStorage;
    }

    return RenderFixturePassStatus::Success;
}

bool RenderFixturePass::HasRecordCapacity() const {
    return snapshot_.pass_record_count < desc_.pass_record_capacity;
}

void RenderFixturePass::RecordRejectedResult(const RenderFixturePassResult &result) {
    snapshot_.last_status = result.status;
    snapshot_.last_rhi_status = result.rhi_status;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_recorded_command_count = result.recorded_command_count;
    snapshot_.last_capture_bytes_written = result.capture_bytes_written;
    snapshot_.last_capture_extent = result.capture_extent;
    if (result.status == RenderFixturePassStatus::CommandCapacityExceeded) {
        ++snapshot_.command_capacity_rejected_count;
        return;
    }

    if (result.status == RenderFixturePassStatus::PassRecordCapacityExceeded) {
        ++snapshot_.pass_record_capacity_rejected_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}

void RenderFixturePass::RecordRhiFailureResult(RenderFixturePassResult *result) {
    if (result == nullptr) {
        return;
    }

    result->status = RenderFixturePassStatus::RhiFailure;
    if (snapshot_.pass_record_count < records_.size()) {
        records_[snapshot_.pass_record_count] = *result;
    }

    ++snapshot_.pass_record_count;
    ++snapshot_.executed_pass_count;
    ++snapshot_.rhi_failure_count;
    snapshot_.last_status = result->status;
    snapshot_.last_rhi_status = result->rhi_status;
    snapshot_.last_pass_id = result->pass_id;
    snapshot_.last_recorded_command_count = result->recorded_command_count;
    snapshot_.last_capture_bytes_written = result->capture_bytes_written;
    snapshot_.last_capture_extent = result->capture_extent;
}

void RenderFixturePass::RecordSuccessResult(const RenderFixturePassResult &result) {
    if (snapshot_.pass_record_count < records_.size()) {
        records_[snapshot_.pass_record_count] = result;
    }

    ++snapshot_.pass_record_count;
    ++snapshot_.executed_pass_count;
    ++snapshot_.completed_pass_count;
    snapshot_.last_status = result.status;
    snapshot_.last_rhi_status = result.rhi_status;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_recorded_command_count = result.recorded_command_count;
    snapshot_.last_capture_bytes_written = result.capture_bytes_written;
    snapshot_.last_capture_extent = result.capture_extent;
}
}
