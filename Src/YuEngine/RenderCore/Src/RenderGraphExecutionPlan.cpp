// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/RenderGraphExecutionPlan.cpp

#include "YuEngine/RenderCore/RenderGraphExecutionPlan.h"

#include <cstddef>

#include "YuEngine/RenderCore/RenderFramePacketFixture.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureRequest.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"

namespace yuengine::rendercore {
namespace {
RenderGraphExecutionPlanDesc NormalizeDesc(RenderGraphExecutionPlanDesc desc) {
    if (desc.plan_record_capacity > MAX_RENDER_GRAPH_EXECUTION_PLAN_RECORDS) {
        desc.plan_record_capacity = MAX_RENDER_GRAPH_EXECUTION_PLAN_RECORDS;
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

std::size_t RequiredFramePacketRecordCount(const RenderFramePacketFixture &fixture) {
    const RenderFramePacketFixtureSnapshot snapshot = fixture.Snapshot();
    return snapshot.frame_packet_record_count + 1U;
}

std::size_t RequiredSubmissionRecordCount(
    const RenderSubmissionBatchFixture &fixture,
    std::size_t pass_count) {
    const RenderSubmissionBatchFixtureSnapshot snapshot = fixture.Snapshot();
    return snapshot.submission_record_count + pass_count;
}

std::size_t RemainingSubmissionRecordCapacity(const RenderSubmissionBatchFixtureSnapshot &snapshot) {
    if (snapshot.submission_record_count >= snapshot.submission_record_capacity) {
        return 0U;
    }

    return snapshot.submission_record_capacity - snapshot.submission_record_count;
}

bool IsCapacityEntryFailure(RenderGraphExecutionPlanStatus status) {
    if (status == RenderGraphExecutionPlanStatus::PlanCapacityExceeded) {
        return true;
    }

    if (status == RenderGraphExecutionPlanStatus::FramePacketCapacityExceeded) {
        return true;
    }

    return status == RenderGraphExecutionPlanStatus::SubmissionBatchCapacityExceeded;
}

void ClearCapacityEntryFailure(RenderGraphExecutionPlanSnapshot &snapshot) {
    snapshot.last_capacity_entry_plan_id = 0U;
    snapshot.last_capacity_entry_graph_id = 0U;
    snapshot.last_capacity_entry_frame_id = 0U;
    snapshot.last_capacity_entry_pass_id = 0U;
    snapshot.last_capacity_entry_material_id = 0U;
    snapshot.last_capacity_entry_pass_count = 0U;
    snapshot.last_capacity_entry_record_slot = 0U;
    snapshot.last_capacity_entry_plan_record_capacity = 0U;
    snapshot.last_capacity_entry_frame_packet_record_capacity = 0U;
    snapshot.last_capacity_entry_submission_record_capacity = 0U;
    snapshot.last_capacity_entry_current_plan_record_count = 0U;
    snapshot.last_capacity_entry_current_frame_packet_record_count = 0U;
    snapshot.last_capacity_entry_current_submission_record_count = 0U;
    snapshot.last_capacity_entry_required_plan_record_count = 0U;
    snapshot.last_capacity_entry_required_frame_packet_record_count = 0U;
    snapshot.last_capacity_entry_required_submission_record_count = 0U;
    snapshot.last_capacity_entry_completed_entry_count = 0U;
    snapshot.last_capacity_entry_failed_entry_count = 0U;
    snapshot.last_capacity_entry_failed_entry_index = 0U;
    snapshot.last_capacity_entry_status = RenderGraphExecutionPlanStatus::InvalidArgument;
}

void StoreCapacityEntryFailure(
    RenderGraphExecutionPlanSnapshot &snapshot,
    const RenderGraphExecutionPlanResult &result) {
    snapshot.last_capacity_entry_plan_id = result.plan_id;
    snapshot.last_capacity_entry_graph_id = result.graph_id;
    snapshot.last_capacity_entry_frame_id = result.frame_id;
    snapshot.last_capacity_entry_pass_id = result.pass_id;
    snapshot.last_capacity_entry_material_id = result.material_id;
    snapshot.last_capacity_entry_pass_count = result.pass_count;
    snapshot.last_capacity_entry_record_slot = result.record_slot;
    snapshot.last_capacity_entry_plan_record_capacity = result.plan_record_capacity;
    snapshot.last_capacity_entry_frame_packet_record_capacity =
        result.frame_packet_record_capacity;
    snapshot.last_capacity_entry_submission_record_capacity =
        result.submission_record_capacity;
    snapshot.last_capacity_entry_current_plan_record_count =
        result.current_plan_record_count;
    snapshot.last_capacity_entry_current_frame_packet_record_count =
        result.current_frame_packet_record_count;
    snapshot.last_capacity_entry_current_submission_record_count =
        result.current_submission_record_count;
    snapshot.last_capacity_entry_required_plan_record_count =
        result.required_plan_record_count;
    snapshot.last_capacity_entry_required_frame_packet_record_count =
        result.required_frame_packet_record_count;
    snapshot.last_capacity_entry_required_submission_record_count =
        result.required_submission_record_count;
    snapshot.last_capacity_entry_completed_entry_count = result.completed_entry_count;
    snapshot.last_capacity_entry_failed_entry_count = result.failed_entry_count;
    snapshot.last_capacity_entry_failed_entry_index = result.failed_entry_index;
    snapshot.last_capacity_entry_status = result.status;
}
}

RenderGraphExecutionPlan::RenderGraphExecutionPlan(const RenderGraphExecutionPlanDesc &desc)
    : desc_(NormalizeDesc(desc)) {
    Reset();
}

RenderGraphExecutionPlanResult RenderGraphExecutionPlan::Execute(
    const RenderGraphExecutionPlanRequest &request) {
    RenderGraphExecutionPlanResult result{};
    result.operation = RenderGraphExecutionPlanOperation::Execute;
    result.plan_id = request.plan_id;
    result.frame_id = request.frame_id;

    if (request.prepared_graph_result != nullptr) {
        result.graph_id = request.prepared_graph_result->graph_id;
        result.pass_count = request.prepared_graph_result->pass_count;
        result.graph_status = request.prepared_graph_result->status;
    }

    result.status = ValidateRequest(request, &result);
    if (result.status != RenderGraphExecutionPlanStatus::Success) {
        RecordRejectedResult(result);
        return result;
    }

    RenderFramePacketFixtureRequest frame_request{};
    frame_request.frame_id = request.frame_id;
    frame_request.submission_batch = request.submission_batch;
    frame_request.batch_request = &request.prepared_graph_result->submission_batch_request;

    const RenderFramePacketFixtureResult frame_result = request.frame_packet->Execute(frame_request);
    result.frame_status = frame_result.status;
    result.batch_status = frame_result.batch_status;
    result.pass_status = frame_result.pass_status;
    result.rhi_status = frame_result.rhi_status;
    result.completed_entry_count = frame_result.completed_entry_count;
    result.failed_entry_count = frame_result.failed_entry_count;
    result.failed_entry_index = frame_result.failed_entry_index;
    result.pass_id = frame_result.pass_id;
    result.material_id = frame_result.material_id;

    if (frame_result.status != RenderFramePacketFixtureStatus::Success) {
        result.status = RenderGraphExecutionPlanStatus::FrameExecutionFailed;
        RecordFrameFailedResult(result);
        return result;
    }

    result.status = RenderGraphExecutionPlanStatus::Success;
    RecordCompletedResult(result);
    return result;
}

std::size_t RenderGraphExecutionPlan::QueryRecords(std::span<RenderGraphExecutionPlanRecord> output) {
    std::size_t copied_count = 0U;
    const std::size_t record_count = snapshot_.plan_record_count;
    while (copied_count < output.size() && copied_count < record_count) {
        output[copied_count] = records_[copied_count].record;
        ++copied_count;
    }

    ++snapshot_.query_count;
    snapshot_.last_operation = RenderGraphExecutionPlanOperation::Query;
    snapshot_.last_status = RenderGraphExecutionPlanStatus::Success;
    snapshot_.last_plan_record_capacity = 0U;
    snapshot_.last_frame_packet_record_capacity = 0U;
    snapshot_.last_submission_record_capacity = 0U;
    snapshot_.last_current_plan_record_count = 0U;
    snapshot_.last_current_frame_packet_record_count = 0U;
    snapshot_.last_current_submission_record_count = 0U;
    snapshot_.last_required_plan_record_count = 0U;
    snapshot_.last_required_frame_packet_record_count = 0U;
    snapshot_.last_required_submission_record_count = 0U;
    ClearCapacityEntryFailure(snapshot_);
    return copied_count;
}

RenderGraphExecutionPlanStatus RenderGraphExecutionPlan::Release(std::uint32_t plan_id) {
    if (plan_id == 0U) {
        RecordReleaseResult(plan_id, RenderGraphExecutionPlanStatus::InvalidPlanId);
        return RenderGraphExecutionPlanStatus::InvalidPlanId;
    }

    for (std::size_t index = 0U; index < snapshot_.plan_record_count; ++index) {
        if (records_[index].record.plan_id != plan_id) {
            continue;
        }

        for (std::size_t next_index = index + 1U; next_index < snapshot_.plan_record_count; ++next_index) {
            records_[next_index - 1U] = records_[next_index];
        }

        records_[snapshot_.plan_record_count - 1U] = Record{};
        --snapshot_.plan_record_count;
        RecordReleaseResult(plan_id, RenderGraphExecutionPlanStatus::Success);
        return RenderGraphExecutionPlanStatus::Success;
    }

    RecordReleaseResult(plan_id, RenderGraphExecutionPlanStatus::PlanRecordNotFound);
    return RenderGraphExecutionPlanStatus::PlanRecordNotFound;
}

RenderGraphExecutionPlanSnapshot RenderGraphExecutionPlan::Snapshot() const {
    return snapshot_;
}

void RenderGraphExecutionPlan::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.plan_record_capacity = desc_.plan_record_capacity;
    snapshot_.reset_count = 1U;
    snapshot_.last_operation = RenderGraphExecutionPlanOperation::Reset;
    snapshot_.last_status = RenderGraphExecutionPlanStatus::Success;
    ClearCapacityEntryFailure(snapshot_);
}

RenderGraphExecutionPlanStatus RenderGraphExecutionPlan::ValidateRequest(
    const RenderGraphExecutionPlanRequest &request,
    RenderGraphExecutionPlanResult *result) const {
    if (result == nullptr) {
        return RenderGraphExecutionPlanStatus::InvalidArgument;
    }

    if (request.plan_id == 0U) {
        return RenderGraphExecutionPlanStatus::InvalidPlanId;
    }

    if (request.frame_id == 0U) {
        return RenderGraphExecutionPlanStatus::InvalidFrameId;
    }

    if (request.frame_packet == nullptr) {
        return RenderGraphExecutionPlanStatus::InvalidFrameExecutor;
    }

    if (request.submission_batch == nullptr) {
        return RenderGraphExecutionPlanStatus::InvalidSubmissionExecutor;
    }

    if (request.prepared_graph_result == nullptr) {
        return RenderGraphExecutionPlanStatus::InvalidArgument;
    }

    result->graph_id = request.prepared_graph_result->graph_id;
    result->pass_count = request.prepared_graph_result->pass_count;
    result->graph_status = request.prepared_graph_result->status;
    if (request.prepared_graph_result->status != RenderGraphSkeletonStatus::Success) {
        return RenderGraphExecutionPlanStatus::FailedSkeletonPrepare;
    }

    if (request.prepared_graph_result->graph_id == 0U) {
        return RenderGraphExecutionPlanStatus::InvalidGraphId;
    }

    if (HasPlanId(request.plan_id)) {
        return RenderGraphExecutionPlanStatus::DuplicatePlanId;
    }

    if (HasGraphId(request.prepared_graph_result->graph_id)) {
        return RenderGraphExecutionPlanStatus::DuplicateGraphExecution;
    }

    result->required_plan_record_count = snapshot_.plan_record_count + 1U;
    result->current_plan_record_count = snapshot_.plan_record_count;
    result->record_slot = snapshot_.plan_record_count;
    result->plan_record_capacity = desc_.plan_record_capacity;
    if (!HasRecordCapacity()) {
        return RenderGraphExecutionPlanStatus::PlanCapacityExceeded;
    }

    const RenderSubmissionBatchFixtureRequest &batch_request =
        request.prepared_graph_result->submission_batch_request;
    const std::size_t pass_count = batch_request.pass_requests.size();
    result->pass_count = pass_count;
    if (request.prepared_graph_result->pass_count == 0U || pass_count == 0U) {
        return RenderGraphExecutionPlanStatus::EmptyPreparedBatch;
    }

    if (batch_request.pass == nullptr) {
        return RenderGraphExecutionPlanStatus::MissingPreparedBatchRequest;
    }

    if (batch_request.pass_requests.data() == nullptr) {
        return RenderGraphExecutionPlanStatus::MissingPreparedBatchRequest;
    }

    if (pass_count != request.prepared_graph_result->pass_count) {
        return RenderGraphExecutionPlanStatus::InvalidPreparedBatch;
    }

    if (batch_request.pass_results.data() == nullptr) {
        return RenderGraphExecutionPlanStatus::MissingPassResultStorage;
    }

    if (batch_request.pass_results.size() < pass_count) {
        return RenderGraphExecutionPlanStatus::MissingPassResultStorage;
    }

    const RenderFramePacketFixtureSnapshot frame_snapshot =
        request.frame_packet->Snapshot();
    result->frame_packet_record_capacity =
        frame_snapshot.frame_packet_record_capacity;
    result->current_frame_packet_record_count =
        frame_snapshot.frame_packet_record_count;
    result->required_frame_packet_record_count =
        RequiredFramePacketRecordCount(*request.frame_packet);
    if (result->required_frame_packet_record_count >
        frame_snapshot.frame_packet_record_capacity) {
        result->frame_status = RenderFramePacketFixtureStatus::PacketCapacityExceeded;
        return RenderGraphExecutionPlanStatus::FramePacketCapacityExceeded;
    }

    const RenderSubmissionBatchFixtureSnapshot submission_snapshot =
        request.submission_batch->Snapshot();
    result->submission_record_capacity =
        submission_snapshot.submission_record_capacity;
    result->current_submission_record_count =
        submission_snapshot.submission_record_count;
    result->required_submission_record_count =
        RequiredSubmissionRecordCount(*request.submission_batch, pass_count);
    if (result->required_submission_record_count >
        submission_snapshot.submission_record_capacity) {
        const std::size_t remaining_submission_record_capacity =
            RemainingSubmissionRecordCapacity(submission_snapshot);
        const RenderFixturePassRequest &failed_request =
            batch_request.pass_requests[remaining_submission_record_capacity];
        result->batch_status = RenderSubmissionBatchFixtureStatus::BatchCapacityExceeded;
        result->failed_entry_index = remaining_submission_record_capacity;
        result->failed_entry_count = pass_count - remaining_submission_record_capacity;
        result->pass_id = failed_request.pass_id;
        result->material_id = failed_request.material_id;
        return RenderGraphExecutionPlanStatus::SubmissionBatchCapacityExceeded;
    }

    for (std::size_t index = 0U; index < pass_count; ++index) {
        const RenderFixturePassRequest &pass_request = batch_request.pass_requests[index];
        result->failed_entry_index = index;
        result->pass_id = pass_request.pass_id;
        result->material_id = pass_request.material_id;
        result->pass_status = ValidatePassRequest(pass_request);
        if (result->pass_status != RenderFixturePassStatus::Success) {
            return RenderGraphExecutionPlanStatus::InvalidPreparedBatch;
        }

        for (std::size_t next_index = index + 1U; next_index < pass_count; ++next_index) {
            const RenderFixturePassRequest &next_request = batch_request.pass_requests[next_index];
            if (next_request.pass_id != pass_request.pass_id) {
                continue;
            }

            result->failed_entry_index = next_index;
            result->pass_id = next_request.pass_id;
            result->material_id = next_request.material_id;
            return RenderGraphExecutionPlanStatus::InvalidPreparedBatch;
        }
    }

    result->record_slot = snapshot_.plan_record_count;
    result->failed_entry_index = 0U;
    result->pass_status = RenderFixturePassStatus::Success;
    return RenderGraphExecutionPlanStatus::Success;
}

bool RenderGraphExecutionPlan::HasPlanId(std::uint32_t plan_id) const {
    for (std::size_t index = 0U; index < snapshot_.plan_record_count; ++index) {
        if (records_[index].record.plan_id == plan_id) {
            return true;
        }
    }

    return false;
}

bool RenderGraphExecutionPlan::HasGraphId(std::uint32_t graph_id) const {
    for (std::size_t index = 0U; index < snapshot_.plan_record_count; ++index) {
        if (records_[index].record.graph_id == graph_id) {
            return true;
        }
    }

    return false;
}

bool RenderGraphExecutionPlan::HasRecordCapacity() const {
    return snapshot_.plan_record_count < desc_.plan_record_capacity;
}

void RenderGraphExecutionPlan::RecordRejectedResult(const RenderGraphExecutionPlanResult &result) {
    StoreLastResult(result);

    if (result.status == RenderGraphExecutionPlanStatus::DuplicatePlanId) {
        ++snapshot_.duplicate_plan_id_count;
        return;
    }

    if (result.status == RenderGraphExecutionPlanStatus::DuplicateGraphExecution) {
        ++snapshot_.duplicate_graph_execution_count;
        return;
    }

    if (result.status == RenderGraphExecutionPlanStatus::PlanCapacityExceeded) {
        ++snapshot_.plan_capacity_rejected_count;
        return;
    }

    if (result.status == RenderGraphExecutionPlanStatus::FramePacketCapacityExceeded) {
        ++snapshot_.frame_capacity_rejected_count;
        return;
    }

    if (result.status == RenderGraphExecutionPlanStatus::SubmissionBatchCapacityExceeded) {
        ++snapshot_.submission_capacity_rejected_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}

void RenderGraphExecutionPlan::RecordCompletedResult(const RenderGraphExecutionPlanResult &result) {
    StoreRecord(result);
    ++snapshot_.accepted_plan_count;
    ++snapshot_.completed_plan_count;
    StoreLastResult(result);
}

void RenderGraphExecutionPlan::RecordFrameFailedResult(const RenderGraphExecutionPlanResult &result) {
    StoreRecord(result);
    ++snapshot_.accepted_plan_count;
    ++snapshot_.frame_failed_plan_count;
    StoreLastResult(result);
}

void RenderGraphExecutionPlan::RecordReleaseResult(
    std::uint32_t plan_id,
    RenderGraphExecutionPlanStatus status) {
    if (status == RenderGraphExecutionPlanStatus::Success) {
        ++snapshot_.released_plan_count;
    }

    if (status != RenderGraphExecutionPlanStatus::Success) {
        ++snapshot_.failed_validation_count;
    }

    snapshot_.last_plan_id = plan_id;
    snapshot_.last_graph_id = 0U;
    snapshot_.last_frame_id = 0U;
    snapshot_.last_pass_count = 0U;
    snapshot_.last_record_slot = 0U;
    snapshot_.last_plan_record_capacity = 0U;
    snapshot_.last_frame_packet_record_capacity = 0U;
    snapshot_.last_submission_record_capacity = 0U;
    snapshot_.last_current_plan_record_count = 0U;
    snapshot_.last_current_frame_packet_record_count = 0U;
    snapshot_.last_current_submission_record_count = 0U;
    snapshot_.last_required_plan_record_count = 0U;
    snapshot_.last_required_frame_packet_record_count = 0U;
    snapshot_.last_required_submission_record_count = 0U;
    snapshot_.last_completed_entry_count = 0U;
    snapshot_.last_failed_entry_count = 0U;
    snapshot_.last_failed_entry_index = 0U;
    snapshot_.last_pass_id = 0U;
    snapshot_.last_material_id = 0U;
    snapshot_.last_status = status;
    snapshot_.last_operation = RenderGraphExecutionPlanOperation::Release;
    snapshot_.last_graph_status = RenderGraphSkeletonStatus::InvalidArgument;
    snapshot_.last_frame_status = RenderFramePacketFixtureStatus::InvalidArgument;
    snapshot_.last_batch_status = RenderSubmissionBatchFixtureStatus::InvalidArgument;
    snapshot_.last_pass_status = RenderFixturePassStatus::InvalidArgument;
    snapshot_.last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    ClearCapacityEntryFailure(snapshot_);
}

void RenderGraphExecutionPlan::StoreRecord(const RenderGraphExecutionPlanResult &result) {
    if (snapshot_.plan_record_count < records_.size()) {
        RenderGraphExecutionPlanRecord record{};
        record.plan_id = result.plan_id;
        record.graph_id = result.graph_id;
        record.frame_id = result.frame_id;
        record.pass_count = result.pass_count;
        record.record_slot = snapshot_.plan_record_count;
        record.completed_entry_count = result.completed_entry_count;
        record.failed_entry_count = result.failed_entry_count;
        record.failed_entry_index = result.failed_entry_index;
        record.pass_id = result.pass_id;
        record.material_id = result.material_id;
        record.status = result.status;
        record.frame_status = result.frame_status;
        record.batch_status = result.batch_status;
        record.pass_status = result.pass_status;
        record.rhi_status = result.rhi_status;
        records_[snapshot_.plan_record_count].record = record;
    }

    ++snapshot_.plan_record_count;
}

void RenderGraphExecutionPlan::StoreLastResult(const RenderGraphExecutionPlanResult &result) {
    snapshot_.last_plan_id = result.plan_id;
    snapshot_.last_graph_id = result.graph_id;
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_pass_count = result.pass_count;
    snapshot_.last_record_slot = result.record_slot;
    snapshot_.last_plan_record_capacity = result.plan_record_capacity;
    snapshot_.last_frame_packet_record_capacity = result.frame_packet_record_capacity;
    snapshot_.last_submission_record_capacity = result.submission_record_capacity;
    snapshot_.last_current_plan_record_count = result.current_plan_record_count;
    snapshot_.last_current_frame_packet_record_count =
        result.current_frame_packet_record_count;
    snapshot_.last_current_submission_record_count =
        result.current_submission_record_count;
    snapshot_.last_required_plan_record_count = result.required_plan_record_count;
    snapshot_.last_required_frame_packet_record_count =
        result.required_frame_packet_record_count;
    snapshot_.last_required_submission_record_count =
        result.required_submission_record_count;
    snapshot_.last_completed_entry_count = result.completed_entry_count;
    snapshot_.last_failed_entry_count = result.failed_entry_count;
    snapshot_.last_failed_entry_index = result.failed_entry_index;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_material_id = result.material_id;
    snapshot_.last_status = result.status;
    snapshot_.last_operation = result.operation;
    snapshot_.last_graph_status = result.graph_status;
    snapshot_.last_frame_status = result.frame_status;
    snapshot_.last_batch_status = result.batch_status;
    snapshot_.last_pass_status = result.pass_status;
    snapshot_.last_rhi_status = result.rhi_status;
    if (IsCapacityEntryFailure(result.status)) {
        StoreCapacityEntryFailure(snapshot_, result);
        return;
    }

    ClearCapacityEntryFailure(snapshot_);
}
}
