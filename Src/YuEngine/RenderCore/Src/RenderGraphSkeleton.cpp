// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/RenderGraphSkeleton.cpp

#include "YuEngine/RenderCore/RenderGraphSkeleton.h"

#include <array>
#include <cstddef>

#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"

namespace yuengine::rendercore {
namespace {
RenderGraphSkeletonDesc NormalizeDesc(RenderGraphSkeletonDesc desc) {
    if (desc.graph_record_capacity > MAX_RENDER_GRAPH_SKELETON_RECORDS) {
        desc.graph_record_capacity = MAX_RENDER_GRAPH_SKELETON_RECORDS;
    }

    if (desc.pass_record_capacity > MAX_RENDER_GRAPH_SKELETON_PASSES) {
        desc.pass_record_capacity = MAX_RENDER_GRAPH_SKELETON_PASSES;
    }

    if (desc.dependency_record_capacity > MAX_RENDER_GRAPH_SKELETON_DEPENDENCIES) {
        desc.dependency_record_capacity = MAX_RENDER_GRAPH_SKELETON_DEPENDENCIES;
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

std::size_t FindPassIndex(
    const std::array<std::uint32_t, MAX_RENDER_GRAPH_SKELETON_PASSES> &pass_ids,
    std::size_t pass_count,
    std::uint32_t pass_id) {
    for (std::size_t index = 0U; index < pass_count; ++index) {
        if (pass_ids[index] == pass_id) {
            return index;
        }
    }

    return pass_count;
}

bool HasDependencyCycle(
    const RenderGraphSkeletonRequest &request,
    const std::array<std::uint32_t, MAX_RENDER_GRAPH_SKELETON_PASSES> &pass_ids,
    std::size_t pass_count) {
    std::array<std::uint32_t, MAX_RENDER_GRAPH_SKELETON_PASSES> indegrees{};
    std::array<bool, MAX_RENDER_GRAPH_SKELETON_PASSES> processed{};

    for (std::size_t index = 0U; index < request.dependency_declarations.size(); ++index) {
        const RenderGraphSkeletonDependencyDeclaration &dependency = request.dependency_declarations[index];
        const std::size_t after_index = FindPassIndex(pass_ids, pass_count, dependency.after_pass_id);
        ++indegrees[after_index];
    }

    std::size_t processed_count = 0U;
    while (processed_count < pass_count) {
        bool found_pass = false;
        std::size_t selected_index = pass_count;
        for (std::size_t index = 0U; index < pass_count; ++index) {
            if (processed[index]) {
                continue;
            }

            if (indegrees[index] != 0U) {
                continue;
            }

            selected_index = index;
            found_pass = true;
            break;
        }

        if (!found_pass) {
            return true;
        }

        processed[selected_index] = true;
        ++processed_count;

        for (std::size_t dependency_index = 0U; dependency_index < request.dependency_declarations.size(); ++dependency_index) {
            const RenderGraphSkeletonDependencyDeclaration &dependency = request.dependency_declarations[dependency_index];
            if (dependency.before_pass_id != pass_ids[selected_index]) {
                continue;
            }

            const std::size_t after_index = FindPassIndex(pass_ids, pass_count, dependency.after_pass_id);
            if (indegrees[after_index] == 0U) {
                continue;
            }

            --indegrees[after_index];
        }
    }

    return false;
}
}

RenderGraphSkeleton::RenderGraphSkeleton(const RenderGraphSkeletonDesc &desc)
    : desc_(NormalizeDesc(desc)) {
    Reset();
}

RenderGraphSkeletonResult RenderGraphSkeleton::Prepare(const RenderGraphSkeletonRequest &request) {
    RenderGraphSkeletonResult result{};
    result.operation = RenderGraphSkeletonOperation::Prepare;
    result.graph_id = request.graph_id;
    result.pass_count = request.pass_declarations.size();
    result.dependency_count = request.dependency_declarations.size();
    result.required_pass_record_count = request.pass_declarations.size();
    result.required_dependency_record_count = request.dependency_declarations.size();

    result.status = ValidateRequest(request, &result);
    if (result.status != RenderGraphSkeletonStatus::Success) {
        RecordRejectedResult(result);
        return result;
    }

    for (std::size_t index = 0U; index < request.pass_declarations.size(); ++index) {
        request.prepared_pass_requests[index] = request.pass_declarations[index].pass_request;
    }

    result.submission_batch_request.pass = request.pass;
    result.submission_batch_request.pass_requests = std::span<const RenderFixturePassRequest>(
        request.prepared_pass_requests.data(),
        request.pass_declarations.size());
    result.submission_batch_request.pass_results = std::span<RenderFixturePassResult>(
        request.pass_results.data(),
        request.pass_declarations.size());
    result.status = RenderGraphSkeletonStatus::Success;
    result.pass_status = RenderFixturePassStatus::Success;
    RecordPreparedResult(result);
    return result;
}

std::size_t RenderGraphSkeleton::QueryRecords(std::span<RenderGraphSkeletonRecord> output) const {
    std::size_t copied_count = 0U;
    const std::size_t record_count = snapshot_.graph_record_count;
    while (copied_count < output.size() && copied_count < record_count) {
        output[copied_count] = records_[copied_count].record;
        ++copied_count;
    }

    return copied_count;
}

RenderGraphSkeletonStatus RenderGraphSkeleton::Release(std::uint32_t graph_id) {
    if (graph_id == 0U) {
        RecordReleaseResult(graph_id, RenderGraphSkeletonStatus::InvalidGraphId);
        return RenderGraphSkeletonStatus::InvalidGraphId;
    }

    for (std::size_t index = 0U; index < snapshot_.graph_record_count; ++index) {
        if (records_[index].record.graph_id != graph_id) {
            continue;
        }

        for (std::size_t next_index = index + 1U; next_index < snapshot_.graph_record_count; ++next_index) {
            records_[next_index - 1U] = records_[next_index];
        }

        records_[snapshot_.graph_record_count - 1U] = Record{};
        --snapshot_.graph_record_count;
        RecordReleaseResult(graph_id, RenderGraphSkeletonStatus::Success);
        return RenderGraphSkeletonStatus::Success;
    }

    RecordReleaseResult(graph_id, RenderGraphSkeletonStatus::GraphRecordNotFound);
    return RenderGraphSkeletonStatus::GraphRecordNotFound;
}

RenderGraphSkeletonSnapshot RenderGraphSkeleton::Snapshot() const {
    return snapshot_;
}

void RenderGraphSkeleton::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.graph_record_capacity = desc_.graph_record_capacity;
    snapshot_.pass_record_capacity = desc_.pass_record_capacity;
    snapshot_.dependency_record_capacity = desc_.dependency_record_capacity;
    snapshot_.last_operation = RenderGraphSkeletonOperation::Reset;
}

RenderGraphSkeletonStatus RenderGraphSkeleton::ValidateRequest(
    const RenderGraphSkeletonRequest &request,
    RenderGraphSkeletonResult *result) const {
    if (result == nullptr) {
        return RenderGraphSkeletonStatus::InvalidArgument;
    }

    if (request.graph_id == 0U) {
        return RenderGraphSkeletonStatus::InvalidGraphId;
    }

    if (request.pass == nullptr) {
        return RenderGraphSkeletonStatus::InvalidArgument;
    }

    if (request.pass_declarations.size() == 0U) {
        return RenderGraphSkeletonStatus::EmptyGraph;
    }

    if (request.pass_declarations.data() == nullptr) {
        return RenderGraphSkeletonStatus::InvalidDeclarationStorage;
    }

    if (request.prepared_pass_requests.data() == nullptr) {
        return RenderGraphSkeletonStatus::InvalidPreparedRequestStorage;
    }

    if (request.prepared_pass_requests.size() < request.pass_declarations.size()) {
        return RenderGraphSkeletonStatus::InvalidPreparedRequestStorage;
    }

    if (request.pass_results.data() == nullptr) {
        return RenderGraphSkeletonStatus::InvalidResultStorage;
    }

    if (request.pass_results.size() < request.pass_declarations.size()) {
        return RenderGraphSkeletonStatus::InvalidResultStorage;
    }

    if (request.dependency_declarations.size() != 0U && request.dependency_declarations.data() == nullptr) {
        return RenderGraphSkeletonStatus::InvalidDeclarationStorage;
    }

    if (request.pass_declarations.size() > desc_.pass_record_capacity) {
        const std::size_t failed_index = desc_.pass_record_capacity;
        const RenderGraphSkeletonPassDeclaration &declaration = request.pass_declarations[failed_index];
        result->failed_pass_index = failed_index;
        result->pass_id = declaration.pass_id;
        return RenderGraphSkeletonStatus::PassCapacityExceeded;
    }

    if (request.dependency_declarations.size() > desc_.dependency_record_capacity) {
        const std::size_t failed_index = desc_.dependency_record_capacity;
        const RenderGraphSkeletonDependencyDeclaration &dependency = request.dependency_declarations[failed_index];
        result->failed_dependency_index = failed_index;
        result->dependency_before_pass_id = dependency.before_pass_id;
        result->dependency_after_pass_id = dependency.after_pass_id;
        return RenderGraphSkeletonStatus::DependencyCapacityExceeded;
    }

    if (HasGraphId(request.graph_id)) {
        return RenderGraphSkeletonStatus::DuplicateGraphId;
    }

    if (!HasRecordCapacity()) {
        return RenderGraphSkeletonStatus::PassCapacityExceeded;
    }

    std::array<std::uint32_t, MAX_RENDER_GRAPH_SKELETON_PASSES> pass_ids{};
    for (std::size_t index = 0U; index < request.pass_declarations.size(); ++index) {
        const RenderGraphSkeletonPassDeclaration &declaration = request.pass_declarations[index];
        result->failed_pass_index = index;
        result->pass_id = declaration.pass_id;
        if (declaration.pass_id == 0U) {
            return RenderGraphSkeletonStatus::InvalidPassId;
        }

        if (declaration.pass_request.pass_id != declaration.pass_id) {
            return RenderGraphSkeletonStatus::InvalidPassId;
        }

        for (std::size_t previous_index = 0U; previous_index < index; ++previous_index) {
            if (pass_ids[previous_index] == declaration.pass_id) {
                return RenderGraphSkeletonStatus::DuplicatePassId;
            }
        }

        result->pass_status = ValidatePassRequest(declaration.pass_request);
        if (result->pass_status != RenderFixturePassStatus::Success) {
            return RenderGraphSkeletonStatus::InvalidPassRequest;
        }

        pass_ids[index] = declaration.pass_id;
    }

    result->failed_pass_index = 0U;
    result->pass_status = RenderFixturePassStatus::Success;
    for (std::size_t index = 0U; index < request.dependency_declarations.size(); ++index) {
        const RenderGraphSkeletonDependencyDeclaration &dependency = request.dependency_declarations[index];
        result->failed_dependency_index = index;
        result->dependency_before_pass_id = dependency.before_pass_id;
        result->dependency_after_pass_id = dependency.after_pass_id;

        if (dependency.before_pass_id == dependency.after_pass_id) {
            return RenderGraphSkeletonStatus::SelfDependency;
        }

        const std::size_t before_index = FindPassIndex(pass_ids, request.pass_declarations.size(), dependency.before_pass_id);
        if (before_index == request.pass_declarations.size()) {
            return RenderGraphSkeletonStatus::MissingDependency;
        }

        const std::size_t after_index = FindPassIndex(pass_ids, request.pass_declarations.size(), dependency.after_pass_id);
        if (after_index == request.pass_declarations.size()) {
            return RenderGraphSkeletonStatus::MissingDependency;
        }
    }

    result->failed_dependency_index = 0U;
    if (HasDependencyCycle(request, pass_ids, request.pass_declarations.size())) {
        return RenderGraphSkeletonStatus::DependencyCycle;
    }

    return RenderGraphSkeletonStatus::Success;
}

bool RenderGraphSkeleton::HasGraphId(std::uint32_t graph_id) const {
    for (std::size_t index = 0U; index < snapshot_.graph_record_count; ++index) {
        if (records_[index].record.graph_id == graph_id) {
            return true;
        }
    }

    return false;
}

bool RenderGraphSkeleton::HasRecordCapacity() const {
    return snapshot_.graph_record_count < desc_.graph_record_capacity;
}

void RenderGraphSkeleton::RecordRejectedResult(const RenderGraphSkeletonResult &result) {
    snapshot_.last_graph_id = result.graph_id;
    snapshot_.last_pass_count = result.pass_count;
    snapshot_.last_dependency_count = result.dependency_count;
    snapshot_.last_required_pass_record_count = result.required_pass_record_count;
    snapshot_.last_required_dependency_record_count =
        result.required_dependency_record_count;
    snapshot_.last_failed_pass_index = result.failed_pass_index;
    snapshot_.last_failed_dependency_index = result.failed_dependency_index;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_dependency_before_pass_id = result.dependency_before_pass_id;
    snapshot_.last_dependency_after_pass_id = result.dependency_after_pass_id;
    snapshot_.last_status = result.status;
    snapshot_.last_operation = result.operation;
    snapshot_.last_pass_status = result.pass_status;

    if (result.status == RenderGraphSkeletonStatus::DuplicateGraphId) {
        ++snapshot_.duplicate_graph_id_count;
        return;
    }

    if (result.status == RenderGraphSkeletonStatus::DuplicatePassId) {
        ++snapshot_.duplicate_pass_id_count;
        return;
    }

    if (result.status == RenderGraphSkeletonStatus::MissingDependency) {
        ++snapshot_.missing_dependency_count;
        return;
    }

    if (result.status == RenderGraphSkeletonStatus::SelfDependency) {
        ++snapshot_.self_dependency_count;
        return;
    }

    if (result.status == RenderGraphSkeletonStatus::DependencyCycle) {
        ++snapshot_.dependency_cycle_count;
        return;
    }

    if (result.status == RenderGraphSkeletonStatus::PassCapacityExceeded) {
        ++snapshot_.pass_capacity_rejected_count;
        return;
    }

    if (result.status == RenderGraphSkeletonStatus::DependencyCapacityExceeded) {
        ++snapshot_.dependency_capacity_rejected_count;
        return;
    }

    if (result.status == RenderGraphSkeletonStatus::InvalidPassRequest) {
        ++snapshot_.invalid_pass_request_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}

void RenderGraphSkeleton::RecordPreparedResult(const RenderGraphSkeletonResult &result) {
    if (snapshot_.graph_record_count < records_.size()) {
        RenderGraphSkeletonRecord record{};
        record.graph_id = result.graph_id;
        record.pass_count = result.pass_count;
        record.dependency_count = result.dependency_count;
        record.first_pass_id = result.submission_batch_request.pass_requests[0U].pass_id;
        record.last_pass_id = result.submission_batch_request.pass_requests[result.pass_count - 1U].pass_id;
        record.status = result.status;
        records_[snapshot_.graph_record_count].record = record;
    }

    ++snapshot_.graph_record_count;
    ++snapshot_.accepted_graph_count;
    ++snapshot_.prepared_graph_count;
    snapshot_.last_graph_id = result.graph_id;
    snapshot_.last_pass_count = result.pass_count;
    snapshot_.last_dependency_count = result.dependency_count;
    snapshot_.last_required_pass_record_count = result.required_pass_record_count;
    snapshot_.last_required_dependency_record_count =
        result.required_dependency_record_count;
    snapshot_.last_pass_id = result.submission_batch_request.pass_requests[result.pass_count - 1U].pass_id;
    snapshot_.last_status = result.status;
    snapshot_.last_operation = result.operation;
    snapshot_.last_pass_status = result.pass_status;
}

void RenderGraphSkeleton::RecordReleaseResult(std::uint32_t graph_id, RenderGraphSkeletonStatus status) {
    if (status == RenderGraphSkeletonStatus::Success) {
        ++snapshot_.released_graph_count;
    }

    if (status != RenderGraphSkeletonStatus::Success) {
        ++snapshot_.failed_validation_count;
    }

    snapshot_.last_graph_id = graph_id;
    snapshot_.last_pass_count = 0U;
    snapshot_.last_dependency_count = 0U;
    snapshot_.last_required_pass_record_count = 0U;
    snapshot_.last_required_dependency_record_count = 0U;
    snapshot_.last_failed_pass_index = 0U;
    snapshot_.last_failed_dependency_index = 0U;
    snapshot_.last_pass_id = 0U;
    snapshot_.last_dependency_before_pass_id = 0U;
    snapshot_.last_dependency_after_pass_id = 0U;
    snapshot_.last_status = status;
    snapshot_.last_operation = RenderGraphSkeletonOperation::Release;
    snapshot_.last_pass_status = RenderFixturePassStatus::InvalidArgument;
}
}
