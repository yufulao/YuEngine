// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/MaterialBindingFixture.cpp

#include "YuEngine/RenderCore/MaterialBindingFixture.h"

#include <algorithm>
#include <cstddef>
#include <span>

#include "YuEngine/Rhi/RhiConstants.h"

namespace yuengine::rendercore {
namespace {
MaterialBindingFixtureDesc NormalizeDesc(MaterialBindingFixtureDesc desc) {
    if (desc.binding_record_capacity > MAX_MATERIAL_BINDING_FIXTURE_RECORDS) {
        desc.binding_record_capacity = MAX_MATERIAL_BINDING_FIXTURE_RECORDS;
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

bool UsesBindingSpans(const MaterialBindingFixtureRequest &request) {
    if (!request.sampled_textures.empty()) {
        return true;
    }

    return !request.samplers.empty();
}

std::span<const yuengine::rhi::RhiSampledTextureBinding> SampledTextureBindings(
    const MaterialBindingFixtureRequest &request) {
    if (!request.sampled_textures.empty()) {
        return request.sampled_textures;
    }

    return std::span<const yuengine::rhi::RhiSampledTextureBinding>(&request.sampled_texture, 1U);
}

std::span<const yuengine::rhi::RhiSamplerBinding> SamplerBindings(
    const MaterialBindingFixtureRequest &request) {
    if (!request.samplers.empty()) {
        return request.samplers;
    }

    return std::span<const yuengine::rhi::RhiSamplerBinding>(&request.sampler, 1U);
}

bool AreBindingSpansValid(const MaterialBindingFixtureRequest &request) {
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
}

MaterialBindingFixture::MaterialBindingFixture(const MaterialBindingFixtureDesc &desc)
    : desc_(NormalizeDesc(desc)) {
    Reset();
}

MaterialBindingFixtureResult MaterialBindingFixture::Bind(
    const MaterialBindingFixtureRequest &request,
    RenderFixturePassRequest *pass_request) {
    MaterialBindingFixtureResult result{};
    result.material_id = request.material_id;
    result.pass_id = request.pass_id;
    result.constant_byte_count = request.constant_bytes.size();

    if (pass_request == nullptr) {
        result.status = MaterialBindingFixtureStatus::InvalidArgument;
        RecordRejectedBinding(result);
        return result;
    }

    result.status = ValidateRequest(request);
    if (result.status != MaterialBindingFixtureStatus::Success) {
        RecordRejectedBinding(result);
        return result;
    }

    if (HasMaterialId(request.material_id)) {
        result.status = MaterialBindingFixtureStatus::DuplicateMaterialId;
        RecordRejectedBinding(result);
        return result;
    }

    if (!HasRecordCapacity()) {
        result.status = MaterialBindingFixtureStatus::BindingCapacityExceeded;
        RecordRejectedBinding(result);
        return result;
    }

    FillPassRequest(request, pass_request);
    RecordAcceptedBinding(request, &result);
    return result;
}

MaterialBindingFixtureResult MaterialBindingFixture::BindAndExecute(
    const MaterialBindingFixtureRequest &request,
    RenderFixturePass *pass,
    RenderFixturePassRequest *pass_request) {
    if (pass == nullptr) {
        MaterialBindingFixtureResult result{};
        result.material_id = request.material_id;
        result.pass_id = request.pass_id;
        result.constant_byte_count = request.constant_bytes.size();
        result.status = MaterialBindingFixtureStatus::InvalidArgument;
        RecordRejectedBinding(result);
        return result;
    }

    MaterialBindingFixtureResult result = Bind(request, pass_request);
    if (result.status != MaterialBindingFixtureStatus::Success) {
        return result;
    }

    const RenderFixturePassResult pass_result = pass->Execute(*pass_request);
    if (pass_result.status != RenderFixturePassStatus::Success) {
        RecordRenderFailure(pass_result, &result);
        return result;
    }

    RecordRenderSuccess(pass_result, &result);
    return result;
}

MaterialBindingFixtureSnapshot MaterialBindingFixture::Snapshot() const {
    return snapshot_;
}

void MaterialBindingFixture::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.binding_record_capacity = desc_.binding_record_capacity;
}

MaterialBindingFixtureStatus MaterialBindingFixture::ValidateRequest(
    const MaterialBindingFixtureRequest &request) const {
    if (request.material_id == 0U) {
        return MaterialBindingFixtureStatus::InvalidMaterialId;
    }

    if (!IsPipelineHandleSet(request.pipeline)) {
        return MaterialBindingFixtureStatus::InvalidPipeline;
    }

    if (!AreBindingSpansValid(request)) {
        return MaterialBindingFixtureStatus::InvalidTextureBinding;
    }

    const std::span<const yuengine::rhi::RhiSampledTextureBinding> sampled_textures =
        SampledTextureBindings(request);
    for (const yuengine::rhi::RhiSampledTextureBinding &binding : sampled_textures) {
        if (!IsSampledTextureBindingValid(binding)) {
            return MaterialBindingFixtureStatus::InvalidTextureBinding;
        }
    }

    const std::span<const yuengine::rhi::RhiSamplerBinding> samplers = SamplerBindings(request);
    for (const yuengine::rhi::RhiSamplerBinding &binding : samplers) {
        if (!IsSamplerBindingValid(binding)) {
            return MaterialBindingFixtureStatus::InvalidSamplerBinding;
        }
    }

    if (request.constant_bytes.size() > MAX_MATERIAL_BINDING_FIXTURE_CONSTANT_BYTES) {
        return MaterialBindingFixtureStatus::OversizedConstants;
    }

    if (request.constant_bytes.size() == 0U) {
        return MaterialBindingFixtureStatus::Success;
    }

    if (request.constant_bytes.data() == nullptr) {
        return MaterialBindingFixtureStatus::InvalidArgument;
    }

    return MaterialBindingFixtureStatus::Success;
}

bool MaterialBindingFixture::HasRecordCapacity() const {
    return snapshot_.binding_record_count < desc_.binding_record_capacity;
}

bool MaterialBindingFixture::HasMaterialId(std::uint32_t material_id) const {
    for (std::size_t index = 0U; index < snapshot_.binding_record_count; ++index) {
        if (records_[index].material_id == material_id) {
            return true;
        }
    }

    return false;
}

void MaterialBindingFixture::FillPassRequest(
    const MaterialBindingFixtureRequest &request,
    RenderFixturePassRequest *pass_request) const {
    if (pass_request == nullptr) {
        return;
    }

    pass_request->pipeline = request.pipeline;
    pass_request->sampled_texture = request.sampled_texture;
    pass_request->sampler = request.sampler;
    pass_request->sampled_textures = request.sampled_textures;
    pass_request->samplers = request.samplers;
    pass_request->material_id = request.material_id;
    pass_request->material_constant_byte_count = request.constant_bytes.size();
    pass_request->pass_id = request.pass_id;
}

void MaterialBindingFixture::RecordAcceptedBinding(
    const MaterialBindingFixtureRequest &request,
    MaterialBindingFixtureResult *result) {
    if (result == nullptr) {
        return;
    }

    Record record{};
    record.material_id = request.material_id;
    record.pipeline = request.pipeline;
    record.sampled_texture = request.sampled_texture;
    record.sampler = request.sampler;
    record.constant_byte_count = request.constant_bytes.size();
    record.pass_id = request.pass_id;
    std::copy(request.constant_bytes.begin(), request.constant_bytes.end(), record.constant_bytes.begin());

    if (snapshot_.binding_record_count < records_.size()) {
        records_[snapshot_.binding_record_count] = record;
    }

    ++snapshot_.binding_record_count;
    ++snapshot_.accepted_binding_count;
    snapshot_.last_material_id = request.material_id;
    snapshot_.last_pass_id = request.pass_id;
    snapshot_.last_constant_byte_count = request.constant_bytes.size();
    snapshot_.last_status = MaterialBindingFixtureStatus::Success;
    snapshot_.last_pass_status = result->pass_status;
    snapshot_.last_rhi_status = result->rhi_status;

    result->status = MaterialBindingFixtureStatus::Success;
}

void MaterialBindingFixture::RecordRejectedBinding(const MaterialBindingFixtureResult &result) {
    snapshot_.last_material_id = result.material_id;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_constant_byte_count = result.constant_byte_count;
    snapshot_.last_status = result.status;
    snapshot_.last_pass_status = result.pass_status;
    snapshot_.last_rhi_status = result.rhi_status;

    if (result.status == MaterialBindingFixtureStatus::DuplicateMaterialId) {
        ++snapshot_.duplicate_material_id_count;
        return;
    }

    if (result.status == MaterialBindingFixtureStatus::BindingCapacityExceeded) {
        ++snapshot_.binding_capacity_rejected_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}

void MaterialBindingFixture::RecordRenderSuccess(
    const RenderFixturePassResult &pass_result,
    MaterialBindingFixtureResult *result) {
    if (result == nullptr) {
        return;
    }

    result->pass_status = pass_result.status;
    result->rhi_status = pass_result.rhi_status;
    ++snapshot_.executed_pass_count;
    ++snapshot_.completed_pass_count;
    snapshot_.last_status = result->status;
    snapshot_.last_pass_status = result->pass_status;
    snapshot_.last_rhi_status = result->rhi_status;
}

void MaterialBindingFixture::RecordRenderFailure(
    const RenderFixturePassResult &pass_result,
    MaterialBindingFixtureResult *result) {
    if (result == nullptr) {
        return;
    }

    result->status = MaterialBindingFixtureStatus::RenderFixturePassFailed;
    result->pass_status = pass_result.status;
    result->rhi_status = pass_result.rhi_status;
    ++snapshot_.executed_pass_count;
    ++snapshot_.render_pass_failure_count;
    snapshot_.last_status = result->status;
    snapshot_.last_pass_status = result->pass_status;
    snapshot_.last_rhi_status = result->rhi_status;
}
}
