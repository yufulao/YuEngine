// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/RenderMaterial.cpp

#include "YuEngine/RenderCore/RenderMaterial.h"

#include <algorithm>
#include <cstddef>
#include <span>

#include "YuEngine/Rhi/RhiConstants.h"

namespace yuengine::rendercore {
namespace {
RenderMaterialDesc NormalizeDesc(RenderMaterialDesc desc) {
    if (desc.material_record_capacity > MAX_RENDER_MATERIAL_RECORDS) {
        desc.material_record_capacity = MAX_RENDER_MATERIAL_RECORDS;
    }

    return desc;
}

bool IsPipelineHandleSet(yuengine::rhi::RhiPipelineHandle handle) {
    return handle.generation != 0U;
}

bool IsTextureHandleSet(yuengine::rhi::RhiTextureHandle handle) {
    return handle.generation != 0U;
}

bool IsSamplerHandleSet(yuengine::rhi::RhiSamplerHandle handle) {
    return handle.generation != 0U;
}

bool IsBlendStateValid(const yuengine::rhi::RhiBlendStateDesc &blend_state) {
    if (blend_state.mode == yuengine::rhi::RhiBlendMode::Opaque) {
        return true;
    }

    return blend_state.mode == yuengine::rhi::RhiBlendMode::AlphaOver;
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

void ClearCapacityEntryFailure(RenderMaterialSnapshot &snapshot) {
    snapshot.last_failed_entry_index = 0U;
    snapshot.last_failed_material_id = 0U;
    snapshot.last_failed_program_id = 0U;
    snapshot.last_failed_pass_id = 0U;
    snapshot.last_capacity_entry_material_record_capacity = 0U;
    snapshot.last_capacity_entry_current_material_record_count = 0U;
    snapshot.last_capacity_entry_required_material_record_count = 0U;
    snapshot.last_capacity_entry_failed_entry_index = 0U;
    snapshot.last_capacity_entry_material_id = 0U;
    snapshot.last_capacity_entry_program_id = 0U;
    snapshot.last_capacity_entry_pass_id = 0U;
    snapshot.last_capacity_entry_status = RenderMaterialStatus::InvalidArgument;
}

void StoreCapacityEntryFailure(
    RenderMaterialSnapshot &snapshot,
    const RenderMaterialResult &result) {
    snapshot.last_capacity_entry_material_record_capacity =
        result.material_record_capacity;
    snapshot.last_capacity_entry_current_material_record_count =
        result.current_material_record_count;
    snapshot.last_capacity_entry_required_material_record_count =
        result.required_material_record_count;
    snapshot.last_capacity_entry_failed_entry_index = result.failed_entry_index;
    snapshot.last_capacity_entry_material_id = result.failed_material_id;
    snapshot.last_capacity_entry_program_id = result.failed_program_id;
    snapshot.last_capacity_entry_pass_id = result.failed_pass_id;
    snapshot.last_capacity_entry_status = result.status;
}
}

RenderMaterial::RenderMaterial(const RenderMaterialDesc &desc)
    : desc_(NormalizeDesc(desc)) {
    Reset();
}

RenderMaterialResult RenderMaterial::BuildBindingRequest(
    const RenderMaterialRequest &request,
    MaterialBindingFixtureRequest *out_request) {
    RenderMaterialResult result{};
    result.material_id = request.material_id;
    result.program_id = request.program_id;
    result.pass_id = request.pass_id;
    result.constant_byte_count = request.constant_bytes.size();

    if (out_request == nullptr) {
        result.status = RenderMaterialStatus::InvalidArgument;
        RecordRejectedMaterial(result);
        return result;
    }

    result.status = ValidateRequest(request);
    if (result.status != RenderMaterialStatus::Success) {
        RecordRejectedMaterial(result);
        return result;
    }

    if (HasMaterialId(request.material_id)) {
        result.status = RenderMaterialStatus::DuplicateMaterialId;
        RecordRejectedMaterial(result);
        return result;
    }

    result.material_record_capacity = desc_.material_record_capacity;
    result.current_material_record_count = snapshot_.material_record_count;
    result.required_material_record_count = RequiredMaterialRecordCount();
    if (!HasRecordCapacity()) {
        result.status = RenderMaterialStatus::MaterialCapacityExceeded;
        result.failed_entry_index = snapshot_.material_record_count;
        result.failed_material_id = request.material_id;
        result.failed_program_id = request.program_id;
        result.failed_pass_id = request.pass_id;
        RecordRejectedMaterial(result);
        return result;
    }

    FillBindingRequest(request, out_request);
    RecordAcceptedMaterial(request, &result);
    return result;
}

RenderMaterialSnapshot RenderMaterial::Snapshot() const {
    return snapshot_;
}

void RenderMaterial::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.material_record_capacity = desc_.material_record_capacity;
    snapshot_.required_material_record_count = 1U;
    ClearCapacityEntryFailure(snapshot_);
}

RenderMaterialStatus RenderMaterial::ValidateRequest(const RenderMaterialRequest &request) const {
    if (request.material_id == 0U) {
        return RenderMaterialStatus::InvalidMaterialId;
    }

    if (request.program_id == 0U) {
        return RenderMaterialStatus::InvalidProgramId;
    }

    if (!IsPipelineHandleSet(request.pipeline)) {
        return RenderMaterialStatus::InvalidPipeline;
    }

    if (!IsBlendStateValid(request.blend_state)) {
        return RenderMaterialStatus::InvalidBlendState;
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

bool RenderMaterial::HasRecordCapacity() const {
    return snapshot_.material_record_count < desc_.material_record_capacity;
}

std::size_t RenderMaterial::RequiredMaterialRecordCount() const {
    return snapshot_.material_record_count + 1U;
}

bool RenderMaterial::HasMaterialId(std::uint32_t material_id) const {
    for (std::size_t index = 0U; index < snapshot_.material_record_count; ++index) {
        if (records_[index].material_id == material_id) {
            return true;
        }
    }

    return false;
}

void RenderMaterial::FillBindingRequest(
    const RenderMaterialRequest &request,
    MaterialBindingFixtureRequest *out_request) const {
    if (out_request == nullptr) {
        return;
    }

    out_request->material_id = request.material_id;
    out_request->pipeline = request.pipeline;
    out_request->blend_state = request.blend_state;
    out_request->sampled_texture = request.sampled_texture;
    out_request->sampler = request.sampler;
    out_request->sampled_textures = request.sampled_textures;
    out_request->samplers = request.samplers;
    out_request->constant_buffers = request.constant_buffers;
    out_request->constant_bytes = request.constant_bytes;
    out_request->pass_id = request.pass_id;
}

void RenderMaterial::RecordAcceptedMaterial(
    const RenderMaterialRequest &request,
    RenderMaterialResult *result) {
    if (result == nullptr) {
        return;
    }

    Record record{};
    record.material_id = request.material_id;
    record.program_id = request.program_id;
    record.pipeline = request.pipeline;
    record.blend_state = request.blend_state;
    record.sampled_texture = request.sampled_texture;
    record.sampler = request.sampler;
    record.constant_byte_count = request.constant_bytes.size();
    record.pass_id = request.pass_id;
    std::copy(request.constant_bytes.begin(), request.constant_bytes.end(), record.constant_bytes.begin());

    if (snapshot_.material_record_count < records_.size()) {
        records_[snapshot_.material_record_count] = record;
    }

    ++snapshot_.material_record_count;
    ++snapshot_.accepted_material_count;
    snapshot_.required_material_record_count = result->required_material_record_count;
    snapshot_.last_material_id = request.material_id;
    snapshot_.last_program_id = request.program_id;
    snapshot_.last_pass_id = request.pass_id;
    snapshot_.last_constant_byte_count = request.constant_bytes.size();
    snapshot_.last_status = RenderMaterialStatus::Success;
    ClearCapacityEntryFailure(snapshot_);

    result->status = RenderMaterialStatus::Success;
}

void RenderMaterial::RecordRejectedMaterial(const RenderMaterialResult &result) {
    snapshot_.last_material_id = result.material_id;
    snapshot_.last_program_id = result.program_id;
    snapshot_.last_pass_id = result.pass_id;
    if (result.required_material_record_count > 0U) {
        snapshot_.required_material_record_count = result.required_material_record_count;
    }
    snapshot_.last_constant_byte_count = result.constant_byte_count;
    snapshot_.last_status = result.status;

    if (result.status == RenderMaterialStatus::DuplicateMaterialId) {
        ClearCapacityEntryFailure(snapshot_);
        ++snapshot_.duplicate_material_id_count;
        return;
    }

    if (result.status == RenderMaterialStatus::MaterialCapacityExceeded) {
        ++snapshot_.material_capacity_rejected_count;
        snapshot_.last_failed_entry_index = result.failed_entry_index;
        snapshot_.last_failed_material_id = result.failed_material_id;
        snapshot_.last_failed_program_id = result.failed_program_id;
        snapshot_.last_failed_pass_id = result.failed_pass_id;
        StoreCapacityEntryFailure(snapshot_, result);
        return;
    }

    ClearCapacityEntryFailure(snapshot_);
    ++snapshot_.failed_validation_count;
}
}
