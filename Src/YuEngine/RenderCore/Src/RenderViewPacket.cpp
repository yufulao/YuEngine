// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/RenderViewPacket.cpp

#include "YuEngine/RenderCore/RenderViewPacket.h"

#include <cstddef>
#include <span>

#include "YuEngine/RenderCore/RenderMaterialConstants.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"

namespace yuengine::rendercore {
namespace {
RenderViewPacketDesc NormalizeDesc(RenderViewPacketDesc desc) {
    if (desc.view_record_capacity > MAX_RENDER_VIEW_PACKET_RECORDS) {
        desc.view_record_capacity = MAX_RENDER_VIEW_PACKET_RECORDS;
    }

    return desc;
}

bool IsTextureHandleSet(yuengine::rhi::RhiTextureHandle handle) {
    return handle.generation != 0U;
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

bool UsesBindingSpans(const RenderMaterialRequest &request) {
    if (!request.sampled_textures.empty()) {
        return true;
    }

    return !request.samplers.empty();
}

std::span<const yuengine::rhi::RhiSampledTextureBinding> SampledTextureBindings(
    const RenderMaterialRequest &request) {
    if (!request.sampled_textures.empty()) {
        return request.sampled_textures;
    }

    return std::span<const yuengine::rhi::RhiSampledTextureBinding>(&request.sampled_texture, 1U);
}

std::span<const yuengine::rhi::RhiSamplerBinding> SamplerBindings(const RenderMaterialRequest &request) {
    if (!request.samplers.empty()) {
        return request.samplers;
    }

    return std::span<const yuengine::rhi::RhiSamplerBinding>(&request.sampler, 1U);
}

bool AreBindingSpansValid(const RenderMaterialRequest &request) {
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

    return request.samplers.size() <= yuengine::rhi::MAX_RHI_SAMPLER_SLOTS;
}

RenderMaterialStatus ValidateMaterialRequest(const RenderMaterialRequest &request) {
    if (request.material_id == 0U) {
        return RenderMaterialStatus::InvalidMaterialId;
    }

    if (request.program_id == 0U) {
        return RenderMaterialStatus::InvalidProgramId;
    }

    if (!IsPipelineHandleSet(request.pipeline)) {
        return RenderMaterialStatus::InvalidPipeline;
    }

    if (!AreBindingSpansValid(request)) {
        return RenderMaterialStatus::InvalidTextureBinding;
    }

    const std::span<const yuengine::rhi::RhiSampledTextureBinding> sampled_textures =
        SampledTextureBindings(request);
    for (const yuengine::rhi::RhiSampledTextureBinding &binding : sampled_textures) {
        if (!IsSampledTextureBindingValid(binding)) {
            return RenderMaterialStatus::InvalidTextureBinding;
        }
    }

    const std::span<const yuengine::rhi::RhiSamplerBinding> samplers = SamplerBindings(request);
    for (const yuengine::rhi::RhiSamplerBinding &binding : samplers) {
        if (!IsSamplerBindingValid(binding)) {
            return RenderMaterialStatus::InvalidSamplerBinding;
        }
    }

    if (request.constant_bytes.size() > MAX_RENDER_MATERIAL_CONSTANT_BYTES) {
        return RenderMaterialStatus::OversizedConstants;
    }

    if (request.constant_bytes.size() == 0U) {
        return RenderMaterialStatus::Success;
    }

    if (request.constant_bytes.data() == nullptr) {
        return RenderMaterialStatus::InvalidArgument;
    }

    return RenderMaterialStatus::Success;
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

std::size_t IndexSizeBytes(yuengine::rhi::RhiIndexFormat format) {
    if (format == yuengine::rhi::RhiIndexFormat::Uint16) {
        return sizeof(std::uint16_t);
    }

    if (format == yuengine::rhi::RhiIndexFormat::Uint32) {
        return sizeof(std::uint32_t);
    }

    return 0U;
}

bool IsIndexBufferViewValid(const yuengine::rhi::RhiIndexBufferView &view) {
    if (!IsBufferHandleSet(view.buffer)) {
        return false;
    }

    const std::size_t index_size = IndexSizeBytes(view.format);
    if (index_size == 0U) {
        return false;
    }

    return view.size_bytes >= index_size;
}

bool IsDrawRangeValid(
    const yuengine::rhi::RhiIndexBufferView &index_buffer,
    const yuengine::rhi::RhiDrawIndexedDesc &draw) {
    const std::size_t index_size = IndexSizeBytes(index_buffer.format);
    if (index_size == 0U) {
        return false;
    }

    const std::size_t available_count = index_buffer.size_bytes / index_size;
    if (draw.first_index >= available_count) {
        return false;
    }

    const std::size_t remaining_count = available_count - draw.first_index;
    return draw.index_count <= remaining_count;
}

bool IsDrawValid(
    const yuengine::rhi::RhiIndexBufferView &index_buffer,
    const yuengine::rhi::RhiDrawIndexedDesc &draw) {
    if (draw.topology == yuengine::rhi::RhiPrimitiveTopology::Unsupported) {
        return false;
    }

    if (draw.index_count == 0U) {
        return false;
    }

    return IsDrawRangeValid(index_buffer, draw);
}

RenderDrawPacketStatus ValidateDrawRequest(const RenderDrawPacketRequest &request) {
    if (request.draw_id == 0U) {
        return RenderDrawPacketStatus::InvalidDrawId;
    }

    if (request.pass_id == 0U) {
        return RenderDrawPacketStatus::InvalidPassId;
    }

    if (request.material_id == 0U) {
        return RenderDrawPacketStatus::InvalidMaterialId;
    }

    if (!IsVertexBufferViewValid(request.vertex_buffer)) {
        return RenderDrawPacketStatus::MissingVertexBuffer;
    }

    if (!IsIndexBufferViewValid(request.index_buffer)) {
        return RenderDrawPacketStatus::MissingIndexBuffer;
    }

    if (!IsDrawValid(request.index_buffer, request.draw)) {
        return RenderDrawPacketStatus::InvalidDraw;
    }

    return RenderDrawPacketStatus::Success;
}

bool IsCaptureOutputValid(const RenderViewPacketRequest &request) {
    if (request.capture_byte_budget == 0U) {
        return false;
    }

    if (request.capture_output.data() == nullptr) {
        return false;
    }

    return request.capture_output.size() >= request.capture_byte_budget;
}
}

RenderViewPacket::RenderViewPacket(const RenderViewPacketDesc &desc)
    : desc_(NormalizeDesc(desc)) {
    Reset();
}

RenderViewPacketResult RenderViewPacket::BuildPassRequest(
    const RenderViewPacketRequest &request,
    RenderFixturePassRequest *out_request) {
    RenderViewPacketResult result{};
    result.view_id = request.view_id;
    result.frame_id = request.frame_id;
    result.pass_id = request.draw.pass_id;
    result.material_id = request.draw.material_id;
    result.draw_id = request.draw.draw_id;
    result.index_count = request.draw.draw.index_count;
    result.constant_byte_count = request.material.constant_bytes.size();
    result.capture_byte_budget = request.capture_byte_budget;

    if (out_request == nullptr) {
        result.status = RenderViewPacketStatus::InvalidArgument;
        RecordRejectedView(result);
        return result;
    }

    result.status = ValidateRequest(request, &result);
    if (result.status != RenderViewPacketStatus::Success) {
        RecordRejectedView(result);
        return result;
    }

    if (HasViewId(request.view_id)) {
        result.status = RenderViewPacketStatus::DuplicateViewId;
        RecordRejectedView(result);
        return result;
    }

    if (!HasRecordCapacity()) {
        result.status = RenderViewPacketStatus::ViewCapacityExceeded;
        RecordRejectedView(result);
        return result;
    }

    RenderFixturePassRequest built_request = *out_request;
    FillPassRequest(request, &built_request);
    *out_request = built_request;
    RecordAcceptedView(&result);
    return result;
}

RenderViewPacketSnapshot RenderViewPacket::Snapshot() const {
    return snapshot_;
}

void RenderViewPacket::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.view_record_capacity = desc_.view_record_capacity;
}

RenderViewPacketStatus RenderViewPacket::ValidateRequest(
    const RenderViewPacketRequest &request,
    RenderViewPacketResult *result) const {
    if (result == nullptr) {
        return RenderViewPacketStatus::InvalidArgument;
    }

    if (request.view_id == 0U) {
        return RenderViewPacketStatus::InvalidViewId;
    }

    if (request.frame_id == 0U) {
        return RenderViewPacketStatus::InvalidFrameId;
    }

    if (!IsTextureHandleSet(request.target)) {
        return RenderViewPacketStatus::InvalidTarget;
    }

    if (!IsCaptureOutputValid(request)) {
        return RenderViewPacketStatus::InvalidCaptureStorage;
    }

    result->material_status = ValidateMaterialRequest(request.material);
    if (result->material_status != RenderMaterialStatus::Success) {
        return RenderViewPacketStatus::MaterialFailed;
    }

    result->draw_status = ValidateDrawRequest(request.draw);
    if (result->draw_status != RenderDrawPacketStatus::Success) {
        return RenderViewPacketStatus::DrawFailed;
    }

    if (request.material.pass_id != request.draw.pass_id) {
        return RenderViewPacketStatus::MismatchedPassId;
    }

    if (request.material.material_id != request.draw.material_id) {
        return RenderViewPacketStatus::MismatchedMaterialId;
    }

    return RenderViewPacketStatus::Success;
}

bool RenderViewPacket::HasRecordCapacity() const {
    return snapshot_.view_record_count < desc_.view_record_capacity;
}

bool RenderViewPacket::HasViewId(std::uint32_t view_id) const {
    for (std::size_t index = 0U; index < snapshot_.view_record_count; ++index) {
        if (records_[index].result.view_id == view_id) {
            return true;
        }
    }

    return false;
}

void RenderViewPacket::FillPassRequest(
    const RenderViewPacketRequest &request,
    RenderFixturePassRequest *out_request) const {
    if (out_request == nullptr) {
        return;
    }

    out_request->target = request.target;
    out_request->pipeline = request.material.pipeline;
    out_request->vertex_buffer = request.draw.vertex_buffer;
    out_request->index_buffer = request.draw.index_buffer;
    out_request->sampled_texture = request.material.sampled_texture;
    out_request->sampler = request.material.sampler;
    out_request->sampled_textures = request.material.sampled_textures;
    out_request->samplers = request.material.samplers;
    out_request->constant_buffers = request.material.constant_buffers;
    out_request->draw = request.draw.draw;
    out_request->clear_color = request.clear_color;
    out_request->capture_output = request.capture_output;
    out_request->capture_byte_budget = request.capture_byte_budget;
    out_request->pass_id = request.draw.pass_id;
    out_request->material_id = request.draw.material_id;
    out_request->material_constant_byte_count = request.material.constant_bytes.size();
}

void RenderViewPacket::RecordAcceptedView(RenderViewPacketResult *result) {
    if (result == nullptr) {
        return;
    }

    if (snapshot_.view_record_count < records_.size()) {
        records_[snapshot_.view_record_count].result = *result;
    }

    ++snapshot_.view_record_count;
    ++snapshot_.accepted_view_count;
    snapshot_.last_view_id = result->view_id;
    snapshot_.last_frame_id = result->frame_id;
    snapshot_.last_pass_id = result->pass_id;
    snapshot_.last_material_id = result->material_id;
    snapshot_.last_draw_id = result->draw_id;
    snapshot_.last_index_count = result->index_count;
    snapshot_.last_constant_byte_count = result->constant_byte_count;
    snapshot_.last_capture_byte_budget = result->capture_byte_budget;
    snapshot_.last_status = RenderViewPacketStatus::Success;
    snapshot_.last_material_status = result->material_status;
    snapshot_.last_draw_status = result->draw_status;

    result->status = RenderViewPacketStatus::Success;
}

void RenderViewPacket::RecordRejectedView(const RenderViewPacketResult &result) {
    snapshot_.last_view_id = result.view_id;
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_material_id = result.material_id;
    snapshot_.last_draw_id = result.draw_id;
    snapshot_.last_index_count = result.index_count;
    snapshot_.last_constant_byte_count = result.constant_byte_count;
    snapshot_.last_capture_byte_budget = result.capture_byte_budget;
    snapshot_.last_status = result.status;
    snapshot_.last_material_status = result.material_status;
    snapshot_.last_draw_status = result.draw_status;

    if (result.status == RenderViewPacketStatus::MaterialFailed) {
        ++snapshot_.material_failure_count;
        return;
    }

    if (result.status == RenderViewPacketStatus::DrawFailed) {
        ++snapshot_.draw_failure_count;
        return;
    }

    if (result.status == RenderViewPacketStatus::DuplicateViewId) {
        ++snapshot_.duplicate_view_id_count;
        return;
    }

    if (result.status == RenderViewPacketStatus::ViewCapacityExceeded) {
        ++snapshot_.view_capacity_rejected_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}
}
