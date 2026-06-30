// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/RenderFramePacketFixture.cpp

#include "YuEngine/RenderCore/RenderFramePacketFixture.h"

#include <cstddef>

#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"

namespace yuengine::rendercore {
namespace {
RenderFramePacketFixtureDesc NormalizeDesc(RenderFramePacketFixtureDesc desc) {
    if (desc.frame_packet_record_capacity > MAX_RENDER_FRAME_PACKET_FIXTURE_RECORDS) {
        desc.frame_packet_record_capacity = MAX_RENDER_FRAME_PACKET_FIXTURE_RECORDS;
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

std::size_t RemainingSubmissionRecordCapacity(const RenderSubmissionBatchFixture &fixture) {
    const RenderSubmissionBatchFixtureSnapshot snapshot = fixture.Snapshot();
    return snapshot.submission_record_capacity - snapshot.submission_record_count;
}

RenderSubmissionBatchFixtureStatus ValidateBatchRequest(
    const RenderSubmissionBatchFixture &fixture,
    const RenderSubmissionBatchFixtureRequest &request,
    RenderFramePacketFixtureResult *result) {
    if (result == nullptr) {
        return RenderSubmissionBatchFixtureStatus::InvalidArgument;
    }

    result->entry_count = request.pass_requests.size();

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

    const std::size_t remaining_record_capacity = RemainingSubmissionRecordCapacity(fixture);
    if (request.pass_requests.size() > remaining_record_capacity) {
        const RenderFixturePassRequest &failed_request = request.pass_requests[remaining_record_capacity];
        result->failed_entry_index = remaining_record_capacity;
        result->pass_id = failed_request.pass_id;
        result->material_id = failed_request.material_id;
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

std::size_t FailedEntryCountFromBatchResult(const RenderSubmissionBatchFixtureResult &batch_result) {
    if (batch_result.status == RenderSubmissionBatchFixtureStatus::Success) {
        return 0U;
    }

    if (batch_result.entry_count == 0U) {
        return 0U;
    }

    return batch_result.entry_count - batch_result.completed_entry_count;
}
}

RenderFramePacketFixture::RenderFramePacketFixture(const RenderFramePacketFixtureDesc &desc)
    : desc_(NormalizeDesc(desc)) {
    Reset();
}

RenderFramePacketFixtureResult RenderFramePacketFixture::Execute(
    const RenderFramePacketFixtureRequest &request) {
    RenderFramePacketFixtureResult result{};
    result.frame_id = request.frame_id;

    if (request.batch_request != nullptr) {
        result.entry_count = request.batch_request->pass_requests.size();
    }

    result.status = ValidateRequest(request, &result);
    if (result.status != RenderFramePacketFixtureStatus::Success) {
        RecordRejectedPacket(result);
        return result;
    }

    const RenderSubmissionBatchFixtureResult batch_result = request.submission_batch->Execute(*request.batch_request);
    result.batch_status = batch_result.status;
    result.pass_status = batch_result.pass_status;
    result.rhi_status = batch_result.rhi_status;
    result.entry_count = batch_result.entry_count;
    result.completed_entry_count = batch_result.completed_entry_count;
    result.failed_entry_count = FailedEntryCountFromBatchResult(batch_result);
    result.failed_entry_index = batch_result.failed_entry_index;
    result.pass_id = batch_result.pass_id;
    result.material_id = batch_result.material_id;

    if (batch_result.status != RenderSubmissionBatchFixtureStatus::Success) {
        result.status = RenderFramePacketFixtureStatus::SubmissionBatchFailed;
        RecordSubmissionBatchFailure(result);
        return result;
    }

    result.status = RenderFramePacketFixtureStatus::Success;
    RecordCompletedPacket(result);
    return result;
}

RenderFramePacketFixtureSnapshot RenderFramePacketFixture::Snapshot() const {
    return snapshot_;
}

void RenderFramePacketFixture::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.frame_packet_record_capacity = desc_.frame_packet_record_capacity;
}

RenderFramePacketFixtureStatus RenderFramePacketFixture::ValidateRequest(
    const RenderFramePacketFixtureRequest &request,
    RenderFramePacketFixtureResult *result) const {
    if (result == nullptr) {
        return RenderFramePacketFixtureStatus::InvalidArgument;
    }

    if (request.frame_id == 0U) {
        return RenderFramePacketFixtureStatus::InvalidFrameId;
    }

    if (request.submission_batch == nullptr) {
        return RenderFramePacketFixtureStatus::InvalidArgument;
    }

    if (request.batch_request == nullptr) {
        result->batch_status = RenderSubmissionBatchFixtureStatus::InvalidArgument;
        return RenderFramePacketFixtureStatus::InvalidBatchRequest;
    }

    if (HasFrameId(request.frame_id)) {
        return RenderFramePacketFixtureStatus::DuplicateFrameId;
    }

    if (!HasRecordCapacity()) {
        return RenderFramePacketFixtureStatus::PacketCapacityExceeded;
    }

    result->batch_status = ValidateBatchRequest(*request.submission_batch, *request.batch_request, result);
    if (result->batch_status != RenderSubmissionBatchFixtureStatus::Success) {
        result->failed_entry_count = 1U;
        return RenderFramePacketFixtureStatus::InvalidBatchRequest;
    }

    return RenderFramePacketFixtureStatus::Success;
}

bool RenderFramePacketFixture::HasRecordCapacity() const {
    return snapshot_.frame_packet_record_count < desc_.frame_packet_record_capacity;
}

bool RenderFramePacketFixture::HasFrameId(std::uint32_t frame_id) const {
    for (std::size_t index = 0U; index < snapshot_.frame_packet_record_count; ++index) {
        if (records_[index].result.frame_id == frame_id) {
            return true;
        }
    }

    return false;
}

void RenderFramePacketFixture::RecordRejectedPacket(const RenderFramePacketFixtureResult &result) {
    ++snapshot_.failed_packet_count;
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_entry_count = result.entry_count;
    snapshot_.last_completed_entry_count = result.completed_entry_count;
    snapshot_.last_failed_entry_count = result.failed_entry_count;
    snapshot_.last_failed_entry_index = result.failed_entry_index;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_material_id = result.material_id;
    snapshot_.last_status = result.status;
    snapshot_.last_batch_status = result.batch_status;
    snapshot_.last_pass_status = result.pass_status;
    snapshot_.last_rhi_status = result.rhi_status;

    if (result.status == RenderFramePacketFixtureStatus::DuplicateFrameId) {
        ++snapshot_.duplicate_frame_id_count;
        return;
    }

    if (result.status == RenderFramePacketFixtureStatus::PacketCapacityExceeded) {
        ++snapshot_.packet_capacity_rejected_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}

void RenderFramePacketFixture::RecordCompletedPacket(const RenderFramePacketFixtureResult &result) {
    if (snapshot_.frame_packet_record_count < records_.size()) {
        records_[snapshot_.frame_packet_record_count].result = result;
    }

    ++snapshot_.frame_packet_record_count;
    ++snapshot_.accepted_packet_count;
    ++snapshot_.completed_packet_count;
    snapshot_.completed_entry_count += result.completed_entry_count;
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_entry_count = result.entry_count;
    snapshot_.last_completed_entry_count = result.completed_entry_count;
    snapshot_.last_failed_entry_count = result.failed_entry_count;
    snapshot_.last_failed_entry_index = result.failed_entry_index;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_material_id = result.material_id;
    snapshot_.last_status = result.status;
    snapshot_.last_batch_status = result.batch_status;
    snapshot_.last_pass_status = result.pass_status;
    snapshot_.last_rhi_status = result.rhi_status;
}

void RenderFramePacketFixture::RecordSubmissionBatchFailure(const RenderFramePacketFixtureResult &result) {
    if (snapshot_.frame_packet_record_count < records_.size()) {
        records_[snapshot_.frame_packet_record_count].result = result;
    }

    ++snapshot_.frame_packet_record_count;
    ++snapshot_.accepted_packet_count;
    ++snapshot_.failed_packet_count;
    ++snapshot_.submission_batch_failure_count;
    snapshot_.completed_entry_count += result.completed_entry_count;
    snapshot_.failed_entry_count += result.failed_entry_count;
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_entry_count = result.entry_count;
    snapshot_.last_completed_entry_count = result.completed_entry_count;
    snapshot_.last_failed_entry_count = result.failed_entry_count;
    snapshot_.last_failed_entry_index = result.failed_entry_index;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_material_id = result.material_id;
    snapshot_.last_status = result.status;
    snapshot_.last_batch_status = result.batch_status;
    snapshot_.last_pass_status = result.pass_status;
    snapshot_.last_rhi_status = result.rhi_status;
}
}
