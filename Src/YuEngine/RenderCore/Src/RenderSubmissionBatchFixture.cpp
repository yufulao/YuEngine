// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Src/RenderSubmissionBatchFixture.cpp

#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"

#include <cstddef>
#include <span>

#include "YuEngine/RenderCore/RenderFixturePass.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"

namespace yuengine::rendercore {
namespace {
RenderSubmissionBatchFixtureDesc NormalizeDesc(RenderSubmissionBatchFixtureDesc desc) {
    if (desc.submission_record_capacity > MAX_RENDER_SUBMISSION_BATCH_FIXTURE_RECORDS) {
        desc.submission_record_capacity = MAX_RENDER_SUBMISSION_BATCH_FIXTURE_RECORDS;
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

RenderFixturePassStatus ValidatePassRequest(const RenderFixturePassRequest &request) {
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

    if (!IsSampledTextureBindingValid(request.sampled_texture)) {
        return RenderFixturePassStatus::InvalidTextureBinding;
    }

    if (!IsSamplerBindingValid(request.sampler)) {
        return RenderFixturePassStatus::InvalidSamplerBinding;
    }

    if (!IsDrawValid(request.draw)) {
        return RenderFixturePassStatus::InvalidDraw;
    }

    if (!IsCaptureOutputValid(request)) {
        return RenderFixturePassStatus::InsufficientCaptureStorage;
    }

    return RenderFixturePassStatus::Success;
}
}

RenderSubmissionBatchFixture::RenderSubmissionBatchFixture(const RenderSubmissionBatchFixtureDesc &desc)
    : desc_(NormalizeDesc(desc)) {
    Reset();
}

RenderSubmissionBatchFixtureResult RenderSubmissionBatchFixture::Execute(
    const RenderSubmissionBatchFixtureRequest &request) {
    RenderSubmissionBatchFixtureResult result{};
    result.entry_count = request.pass_requests.size();

    result.status = ValidateRequest(request, &result);
    if (result.status != RenderSubmissionBatchFixtureStatus::Success) {
        RecordRejectedBatch(result);
        return result;
    }

    for (std::size_t index = 0U; index < request.pass_requests.size(); ++index) {
        const RenderFixturePassRequest &pass_request = request.pass_requests[index];
        const RenderFixturePassResult pass_result = request.pass->Execute(pass_request);
        request.pass_results[index] = pass_result;
        if (pass_result.status != RenderFixturePassStatus::Success) {
            RecordRenderFailure(pass_request, pass_result, index, &result);
            return result;
        }

        RecordRenderSuccess(pass_request, pass_result, index);
        ++result.completed_entry_count;
        result.pass_id = pass_request.pass_id;
        result.material_id = pass_request.material_id;
        result.pass_status = pass_result.status;
        result.rhi_status = pass_result.rhi_status;
    }

    result.status = RenderSubmissionBatchFixtureStatus::Success;
    return result;
}

RenderSubmissionBatchFixtureSnapshot RenderSubmissionBatchFixture::Snapshot() const {
    return snapshot_;
}

void RenderSubmissionBatchFixture::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.submission_record_capacity = desc_.submission_record_capacity;
}

RenderSubmissionBatchFixtureStatus RenderSubmissionBatchFixture::ValidateRequest(
    const RenderSubmissionBatchFixtureRequest &request,
    RenderSubmissionBatchFixtureResult *result) const {
    if (result == nullptr) {
        return RenderSubmissionBatchFixtureStatus::InvalidArgument;
    }

    if (request.pass == nullptr) {
        return RenderSubmissionBatchFixtureStatus::InvalidArgument;
    }

    if (request.pass_requests.size() == 0U) {
        return RenderSubmissionBatchFixtureStatus::EmptyBatch;
    }

    if (request.pass_requests.data() == nullptr) {
        return RenderSubmissionBatchFixtureStatus::InvalidRequestStorage;
    }

    if (request.pass_results.data() == nullptr) {
        return RenderSubmissionBatchFixtureStatus::InvalidResultStorage;
    }

    if (request.pass_results.size() < request.pass_requests.size()) {
        return RenderSubmissionBatchFixtureStatus::InvalidResultStorage;
    }

    if (!HasRecordCapacity(request.pass_requests.size())) {
        return RenderSubmissionBatchFixtureStatus::BatchCapacityExceeded;
    }

    for (std::size_t index = 0U; index < request.pass_requests.size(); ++index) {
        const RenderFixturePassRequest &pass_request = request.pass_requests[index];
        const RenderFixturePassStatus pass_status = ValidatePassRequest(pass_request);
        result->failed_entry_index = index;
        result->pass_id = pass_request.pass_id;
        result->material_id = pass_request.material_id;
        result->pass_status = pass_status;
        if (pass_status != RenderFixturePassStatus::Success) {
            return RenderSubmissionBatchFixtureStatus::InvalidPassRequest;
        }

        if (HasPassId(pass_request.pass_id)) {
            return RenderSubmissionBatchFixtureStatus::DuplicatePassId;
        }

        for (std::size_t next_index = index + 1U; next_index < request.pass_requests.size(); ++next_index) {
            const RenderFixturePassRequest &next_request = request.pass_requests[next_index];
            if (next_request.pass_id == pass_request.pass_id) {
                result->failed_entry_index = next_index;
                result->pass_id = next_request.pass_id;
                result->material_id = next_request.material_id;
                return RenderSubmissionBatchFixtureStatus::DuplicatePassId;
            }
        }
    }

    result->failed_entry_index = 0U;
    result->pass_status = RenderFixturePassStatus::Success;
    return RenderSubmissionBatchFixtureStatus::Success;
}

bool RenderSubmissionBatchFixture::HasRecordCapacity(std::size_t entry_count) const {
    const std::size_t remaining_capacity = desc_.submission_record_capacity - snapshot_.submission_record_count;
    return entry_count <= remaining_capacity;
}

bool RenderSubmissionBatchFixture::HasPassId(std::uint32_t pass_id) const {
    for (std::size_t index = 0U; index < snapshot_.submission_record_count; ++index) {
        if (records_[index].pass_result.pass_id == pass_id) {
            return true;
        }
    }

    return false;
}

void RenderSubmissionBatchFixture::RecordRejectedBatch(const RenderSubmissionBatchFixtureResult &result) {
    snapshot_.last_entry_index = result.failed_entry_index;
    snapshot_.last_status = result.status;
    snapshot_.last_pass_status = result.pass_status;
    snapshot_.last_rhi_status = result.rhi_status;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_material_id = result.material_id;

    if (result.status == RenderSubmissionBatchFixtureStatus::DuplicatePassId) {
        ++snapshot_.duplicate_pass_id_count;
        return;
    }

    if (result.status == RenderSubmissionBatchFixtureStatus::BatchCapacityExceeded) {
        ++snapshot_.batch_capacity_rejected_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}

void RenderSubmissionBatchFixture::RecordRenderSuccess(
    const RenderFixturePassRequest &request,
    const RenderFixturePassResult &pass_result,
    std::size_t entry_index) {
    if (snapshot_.submission_record_count < records_.size()) {
        records_[snapshot_.submission_record_count].pass_result = pass_result;
        records_[snapshot_.submission_record_count].material_id = request.material_id;
    }

    ++snapshot_.submission_record_count;
    ++snapshot_.accepted_entry_count;
    ++snapshot_.executed_entry_count;
    ++snapshot_.completed_entry_count;
    snapshot_.last_entry_index = entry_index;
    snapshot_.last_recorded_command_count = pass_result.recorded_command_count;
    snapshot_.last_capture_bytes_written = pass_result.capture_bytes_written;
    snapshot_.last_pass_id = request.pass_id;
    snapshot_.last_material_id = request.material_id;
    snapshot_.last_status = RenderSubmissionBatchFixtureStatus::Success;
    snapshot_.last_pass_status = pass_result.status;
    snapshot_.last_rhi_status = pass_result.rhi_status;
}

void RenderSubmissionBatchFixture::RecordRenderFailure(
    const RenderFixturePassRequest &request,
    const RenderFixturePassResult &pass_result,
    std::size_t entry_index,
    RenderSubmissionBatchFixtureResult *result) {
    if (result == nullptr) {
        return;
    }

    if (snapshot_.submission_record_count < records_.size()) {
        records_[snapshot_.submission_record_count].pass_result = pass_result;
        records_[snapshot_.submission_record_count].material_id = request.material_id;
    }

    ++snapshot_.submission_record_count;
    ++snapshot_.accepted_entry_count;
    ++snapshot_.executed_entry_count;
    ++snapshot_.render_pass_failure_count;
    snapshot_.last_entry_index = entry_index;
    snapshot_.last_recorded_command_count = pass_result.recorded_command_count;
    snapshot_.last_capture_bytes_written = pass_result.capture_bytes_written;
    snapshot_.last_pass_id = request.pass_id;
    snapshot_.last_material_id = request.material_id;
    snapshot_.last_status = RenderSubmissionBatchFixtureStatus::RenderFixturePassFailed;
    snapshot_.last_pass_status = pass_result.status;
    snapshot_.last_rhi_status = pass_result.rhi_status;

    result->status = RenderSubmissionBatchFixtureStatus::RenderFixturePassFailed;
    result->failed_entry_index = entry_index;
    result->pass_id = request.pass_id;
    result->material_id = request.material_id;
    result->pass_status = pass_result.status;
    result->rhi_status = pass_result.rhi_status;
}
}
