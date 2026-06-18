// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceRegistry.h

#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

#include "YuEngine/Resource/ResourceCachePayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceCachePayloadOperation.h"
#include "YuEngine/Resource/ResourceCachePayloadRecord.h"
#include "YuEngine/Resource/ResourceCachePayloadRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadSnapshot.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodedPayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodedPayloadOperation.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRecord.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRequest.h"
#include "YuEngine/Resource/ResourceDecodedPayloadSnapshot.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodePlanBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodePlanOperation.h"
#include "YuEngine/Resource/ResourceDecodePlanRecord.h"
#include "YuEngine/Resource/ResourceDecodePlanRequest.h"
#include "YuEngine/Resource/ResourceDecodePlanSnapshot.h"
#include "YuEngine/Resource/ResourceDecodePlanStatus.h"
#include "YuEngine/Resource/ResourceDecodeResultBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodeResultOperation.h"
#include "YuEngine/Resource/ResourceDecodeResultRecord.h"
#include "YuEngine/Resource/ResourceDecodeResultRequest.h"
#include "YuEngine/Resource/ResourceDecodeResultSnapshot.h"
#include "YuEngine/Resource/ResourceDecodeResultStatus.h"
#include "YuEngine/Resource/ResourceDependencyEdge.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceLoadCommitRecord.h"
#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceRegistrationResult.h"
#include "YuEngine/Resource/ResourceRegistryDesc.h"
#include "YuEngine/Resource/ResourceResidencyBudgetDesc.h"
#include "YuEngine/Resource/ResourceResidencyOperation.h"
#include "YuEngine/Resource/ResourceResidencyRecord.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Resource/ResourceResidencySnapshot.h"
#include "YuEngine/Resource/ResourceResidencyState.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"
#include "YuEngine/Resource/ResourceSlot.h"
#include "YuEngine/Resource/ResourceSnapshot.h"

namespace yuengine::resource {
class ResourceRegistry final {
public:
    /**
     * @comment Constructs a ResourceRegistry instance.
     */
    ResourceRegistry();
    /**
     * @comment Constructs a ResourceRegistry instance.
     * @param desc Input descriptor.
     */
    explicit ResourceRegistry(ResourceRegistryDesc desc);

    /**
     * @comment Registers synthetic descriptor.
     * @param descriptor Input descriptor.
     * @return Explicit operation result.
     */
    ResourceRegistrationResult RegisterSyntheticDescriptor(const ResourceDescriptor& descriptor);
    /**
     * @comment Adds dependency.
     * @param dependent Input dependent.
     * @param dependency Input dependency.
     * @return Explicit operation status.
     */
    ResourceStatus AddDependency(ResourceHandle dependent, ResourceHandle dependency);
    /**
     * @comment Acquires the operation.
     * @param handle Input handle.
     * @param expected_type Input expected type.
     * @return Explicit operation status.
     */
    ResourceStatus Acquire(ResourceHandle handle, ResourceTypeId expected_type);
    /**
     * @comment Validates that a projected acquire can succeed without mutating registry state.
     * @param handle Input handle.
     * @param expected_type Input expected type.
     * @param projected_acquire_count Input acquire count budget.
     * @return Explicit operation status.
     */
    ResourceStatus ValidateAcquire(
        ResourceHandle handle,
        ResourceTypeId expected_type,
        std::uint32_t projected_acquire_count) const;
    /**
     * @comment Commits a terminal Resource load state from an upload completion value.
     * @param request Input commit request.
     * @return Explicit operation status.
     */
    ResourceLoadCommitStatus CommitUploadCompletion(const ResourceLoadCommitRequest &request);
    /**
     * @comment Returns the Resource load state for a valid handle.
     * @param handle Input handle.
     * @param expected_type Input expected type.
     * @param output_state Output load state.
     * @return Explicit operation status.
     */
    ResourceLoadCommitStatus GetLoadState(
        ResourceHandle handle,
        ResourceTypeId expected_type,
        ResourceLoadState *output_state) const;
    /**
     * @comment Configures the Resource-owned residency budget.
     * @param desc Input budget descriptor.
     * @return Explicit residency operation status.
     */
    ResourceResidencyStatus SetResidencyBudget(ResourceResidencyBudgetDesc desc);
    /**
     * @comment Admits an uploaded Resource slot into resident state.
     * @param request Input residency request.
     * @return Explicit residency operation status.
     */
    ResourceResidencyStatus AdmitResident(const ResourceResidencyRequest &request);
    /**
     * @comment Pins a resident Resource slot.
     * @param request Input residency request.
     * @return Explicit residency operation status.
     */
    ResourceResidencyStatus PinResident(const ResourceResidencyRequest &request);
    /**
     * @comment Unpins a pinned Resource slot.
     * @param request Input residency request.
     * @return Explicit residency operation status.
     */
    ResourceResidencyStatus UnpinResident(const ResourceResidencyRequest &request);
    /**
     * @comment Marks a resident Resource slot as evicted in Resource-owned state.
     * @param request Input residency request.
     * @return Explicit residency operation status.
     */
    ResourceResidencyStatus EvictResident(const ResourceResidencyRequest &request);
    /**
     * @comment Selects a deterministic Resource-owned eviction candidate.
     * @param output_handle Output candidate handle.
     * @return Explicit residency operation status.
     */
    ResourceResidencyStatus SelectEvictionCandidate(ResourceHandle *output_handle);
    /**
     * @comment Returns Resource-owned residency state for a valid handle.
     * @param handle Input handle.
     * @param expected_type Input expected type.
     * @param output_state Output residency state.
     * @return Explicit residency operation status.
     */
    ResourceResidencyStatus GetResidencyState(
        ResourceHandle handle,
        ResourceTypeId expected_type,
        ResourceResidencyState *output_state) const;
    /**
     * @comment Returns a snapshot of Resource-owned residency counters.
     * @return Residency snapshot value.
     */
    ResourceResidencySnapshot ResidencySnapshot() const;
    /**
     * @comment Configures the Resource-owned cache payload budget.
     * @param desc Input budget descriptor.
     * @return Explicit cache payload operation status.
     */
    ResourceCachePayloadStatus SetCachePayloadBudget(ResourceCachePayloadBudgetDesc desc);
    /**
     * @comment Stores caller-provided opaque bytes in Resource-owned cache payload storage.
     * @param request Input cache payload request.
     * @return Explicit cache payload operation status.
     */
    ResourceCachePayloadStatus StoreCachePayload(const ResourceCachePayloadRequest &request);
    /**
     * @comment Reads Resource-owned opaque cache payload bytes into caller-owned output storage.
     * @param request Input cache payload request.
     * @param output_bytes Output byte storage.
     * @param output_byte_capacity Output byte capacity.
     * @param output_byte_count Output byte count.
     * @return Explicit cache payload operation status.
     */
    ResourceCachePayloadStatus ReadCachePayload(
        const ResourceCachePayloadRequest &request,
        std::uint8_t *output_bytes,
        std::uint32_t output_byte_capacity,
        std::uint32_t *output_byte_count);
    /**
     * @comment Releases Resource-owned cache payload bytes without changing load or residency state.
     * @param request Input cache payload request.
     * @return Explicit cache payload operation status.
     */
    ResourceCachePayloadStatus ReleaseCachePayload(const ResourceCachePayloadRequest &request);
    /**
     * @comment Returns a snapshot of Resource-owned cache payload counters.
     * @return Cache payload snapshot value.
     */
    ResourceCachePayloadSnapshot CachePayloadSnapshot() const;
    /**
     * @comment Configures the Resource-owned decode plan budget.
     * @param desc Input budget descriptor.
     * @return Explicit decode plan operation status.
     */
    ResourceDecodePlanStatus SetDecodePlanBudget(ResourceDecodePlanBudgetDesc desc);
    /**
     * @comment Creates a Resource-owned decode plan over an existing cache payload record.
     * @param request Input decode plan request.
     * @return Explicit decode plan operation status.
     */
    ResourceDecodePlanStatus CreateDecodePlan(const ResourceDecodePlanRequest &request);
    /**
     * @comment Queries a Resource-owned decode plan record.
     * @param request Input decode plan request.
     * @param output_record Output decode plan record.
     * @return Explicit decode plan operation status.
     */
    ResourceDecodePlanStatus QueryDecodePlan(
        const ResourceDecodePlanRequest &request,
        ResourceDecodePlanRecord *output_record);
    /**
     * @comment Releases a Resource-owned decode plan record without changing payload or residency state.
     * @param request Input decode plan request.
     * @return Explicit decode plan operation status.
     */
    ResourceDecodePlanStatus ReleaseDecodePlan(const ResourceDecodePlanRequest &request);
    /**
     * @comment Returns a snapshot of Resource-owned decode plan counters.
     * @return Decode plan snapshot value.
     */
    ResourceDecodePlanSnapshot DecodePlanSnapshot() const;
    /**
     * @comment Configures the Resource-owned decode result budget.
     * @param desc Input budget descriptor.
     * @return Explicit decode result operation status.
     */
    ResourceDecodeResultStatus SetDecodeResultBudget(ResourceDecodeResultBudgetDesc desc);
    /**
     * @comment Commits a Resource-owned import-ready decode result over an existing decode plan.
     * @param request Input decode result request.
     * @return Explicit decode result operation status.
     */
    ResourceDecodeResultStatus CommitDecodeResult(const ResourceDecodeResultRequest &request);
    /**
     * @comment Queries a Resource-owned decode result record.
     * @param request Input decode result request.
     * @param output_record Output decode result record.
     * @return Explicit decode result operation status.
     */
    ResourceDecodeResultStatus QueryDecodeResult(
        const ResourceDecodeResultRequest &request,
        ResourceDecodeResultRecord *output_record);
    /**
     * @comment Releases a Resource-owned decode result record without changing payload or residency state.
     * @param request Input decode result request.
     * @return Explicit decode result operation status.
     */
    ResourceDecodeResultStatus ReleaseDecodeResult(const ResourceDecodeResultRequest &request);
    /**
     * @comment Returns a snapshot of Resource-owned decode result counters.
     * @return Decode result snapshot value.
     */
    ResourceDecodeResultSnapshot DecodeResultSnapshot() const;
    /**
     * @comment Configures the Resource-owned decoded payload byte budget.
     * @param desc Input budget descriptor.
     * @return Explicit decoded payload operation status.
     */
    ResourceDecodedPayloadStatus SetDecodedPayloadBudget(ResourceDecodedPayloadBudgetDesc desc);
    /**
     * @comment Stores caller-provided decoded bytes in Resource-owned decoded payload storage.
     * @param request Input decoded payload request.
     * @return Explicit decoded payload operation status.
     */
    ResourceDecodedPayloadStatus StoreDecodedPayload(const ResourceDecodedPayloadRequest &request);
    /**
     * @comment Queries a Resource-owned decoded payload record.
     * @param request Input decoded payload request.
     * @param output_record Output decoded payload record.
     * @return Explicit decoded payload operation status.
     */
    ResourceDecodedPayloadStatus QueryDecodedPayload(
        const ResourceDecodedPayloadRequest &request,
        ResourceDecodedPayloadRecord *output_record);
    /**
     * @comment Reads Resource-owned decoded payload bytes into caller-owned output storage.
     * @param request Input decoded payload request.
     * @param output_bytes Output byte storage.
     * @param output_byte_capacity Output byte capacity.
     * @param output_byte_count Output byte count.
     * @return Explicit decoded payload operation status.
     */
    ResourceDecodedPayloadStatus ReadDecodedPayload(
        const ResourceDecodedPayloadRequest &request,
        std::uint8_t *output_bytes,
        std::uint32_t output_byte_capacity,
        std::uint32_t *output_byte_count);
    /**
     * @comment Releases Resource-owned decoded payload bytes without changing earlier Resource state.
     * @param request Input decoded payload request.
     * @return Explicit decoded payload operation status.
     */
    ResourceDecodedPayloadStatus ReleaseDecodedPayload(const ResourceDecodedPayloadRequest &request);
    /**
     * @comment Returns a snapshot of Resource-owned decoded payload counters.
     * @return Decoded payload snapshot value.
     */
    ResourceDecodedPayloadSnapshot DecodedPayloadSnapshot() const;
    /**
     * @comment Releases the operation.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    ResourceStatus Release(ResourceHandle handle);
    /**
     * @comment Retires the operation.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    ResourceStatus Retire(ResourceHandle handle);
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    ResourceSnapshot Snapshot() const;

private:
    ResourceStatus RecordFailure(ResourceStatus status);
    void RecordSuccess();
    ResourceLoadCommitStatus RecordLoadCommitRejected(ResourceLoadCommitStatus status);
    void RecordLoadCommitSuccess(ResourceLoadState load_state);
    ResourceResidencyStatus RecordResidencyRejected(
        ResourceResidencyOperation operation,
        const ResourceResidencyRequest &request,
        ResourceResidencyStatus status);
    void RecordResidencySuccess(
        ResourceResidencyOperation operation,
        const ResourceResidencyRequest &request,
        ResourceResidencyState state);
    ResourceCachePayloadStatus RecordCachePayloadRejected(
        ResourceCachePayloadOperation operation,
        const ResourceCachePayloadRequest &request,
        ResourceCachePayloadStatus status);
    void RecordCachePayloadSuccess(
        ResourceCachePayloadOperation operation,
        const ResourceCachePayloadRequest &request,
        std::uint32_t cache_slot_index,
        std::uint32_t payload_byte_count);
    ResourceDecodePlanStatus RecordDecodePlanRejected(
        ResourceDecodePlanOperation operation,
        const ResourceDecodePlanRequest &request,
        ResourceDecodePlanStatus status);
    void RecordDecodePlanSuccess(
        ResourceDecodePlanOperation operation,
        const ResourceDecodePlanRequest &request,
        std::uint32_t cache_slot_index,
        std::uint32_t header_version);
    ResourceDecodeResultStatus RecordDecodeResultRejected(
        ResourceDecodeResultOperation operation,
        const ResourceDecodeResultRequest &request,
        ResourceDecodeResultStatus status);
    void RecordDecodeResultSuccess(
        ResourceDecodeResultOperation operation,
        const ResourceDecodeResultRequest &request,
        std::uint32_t decode_plan_slot_index);
    ResourceDecodedPayloadStatus RecordDecodedPayloadRejected(
        ResourceDecodedPayloadOperation operation,
        const ResourceDecodedPayloadRequest &request,
        ResourceDecodedPayloadStatus status);
    void RecordDecodedPayloadSuccess(
        ResourceDecodedPayloadOperation operation,
        const ResourceDecodedPayloadRequest &request,
        std::uint32_t decode_result_slot_index,
        std::uint32_t decoded_payload_slot_index,
        std::uint32_t decoded_byte_count);
    ResourceLoadCommitStatus ValidateLoadCommitRequest(
        const ResourceLoadCommitRequest &request,
        std::size_t *out_slot_index) const;
    ResourceResidencyStatus ValidateResidencyRequest(
        const ResourceResidencyRequest &request,
        std::size_t *out_slot_index) const;
    ResourceCachePayloadStatus ValidateCachePayloadRequest(
        const ResourceCachePayloadRequest &request,
        std::size_t *out_slot_index) const;
    ResourceDecodePlanStatus ValidateDecodePlanRequest(
        const ResourceDecodePlanRequest &request,
        std::size_t *out_slot_index) const;
    ResourceDecodePlanStatus ValidateDecodePlanHeader(
        const ResourceDecodePlanRequest &request,
        const ResourceCachePayloadRecord &cache_payload_record,
        std::uint32_t *out_header_version) const;
    ResourceDecodeResultStatus ValidateDecodeResultRequest(
        const ResourceDecodeResultRequest &request,
        std::size_t *out_slot_index) const;
    ResourceDecodeResultStatus ValidateDecodeResultPlan(
        const ResourceDecodeResultRequest &request,
        const ResourceDecodePlanRecord &decode_plan_record) const;
    ResourceDecodedPayloadStatus ValidateDecodedPayloadRequest(
        const ResourceDecodedPayloadRequest &request,
        std::size_t *out_slot_index) const;
    ResourceDecodedPayloadStatus ValidateDecodedPayloadResult(
        const ResourceDecodedPayloadRequest &request,
        const ResourceDecodeResultRecord &decode_result_record) const;
    ResourceLoadCommitStatus MapHandleStatus(ResourceStatus status) const;
    ResourceStatus MapLoadCommitStatus(ResourceLoadCommitStatus status) const;
    ResourceResidencyStatus MapHandleResidencyStatus(ResourceStatus status) const;
    ResourceStatus MapResidencyStatus(ResourceResidencyStatus status) const;
    ResourceCachePayloadStatus MapHandleCachePayloadStatus(ResourceStatus status) const;
    ResourceStatus MapCachePayloadStatus(ResourceCachePayloadStatus status) const;
    ResourceDecodePlanStatus MapHandleDecodePlanStatus(ResourceStatus status) const;
    ResourceStatus MapDecodePlanStatus(ResourceDecodePlanStatus status) const;
    ResourceDecodeResultStatus MapHandleDecodeResultStatus(ResourceStatus status) const;
    ResourceStatus MapDecodeResultStatus(ResourceDecodeResultStatus status) const;
    ResourceDecodedPayloadStatus MapHandleDecodedPayloadStatus(ResourceStatus status) const;
    ResourceStatus MapDecodedPayloadStatus(ResourceDecodedPayloadStatus status) const;
    ResourceStatus ResolveHandle(ResourceHandle handle, std::size_t& out_index) const;
    ResourceStatus RegisterTypeIfNeeded(ResourceTypeId type);
    bool HasType(ResourceTypeId type) const;
    bool HasDuplicateActiveResource(const ResourceDescriptor& descriptor) const;
    bool HasLoadCommitId(std::uint64_t commit_id) const;
    bool HasCachePayloadId(std::uint64_t payload_id) const;
    bool HasDecodePlanId(std::uint64_t decode_plan_id) const;
    bool HasDecodeResultId(std::uint64_t decode_result_id) const;
    bool HasDecodedPayloadId(std::uint64_t decoded_payload_id) const;
    bool FindCachePayloadRecord(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::size_t *out_record_index) const;
    bool FindFreeCachePayloadRecord(std::size_t *out_record_index) const;
    bool FindDecodePlanRecord(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::uint64_t decode_plan_id,
        std::size_t *out_record_index) const;
    bool FindFreeDecodePlanRecord(std::size_t *out_record_index) const;
    bool FindDecodeResultRecord(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::uint64_t decode_plan_id,
        std::uint64_t decode_result_id,
        std::size_t *out_record_index) const;
    bool FindFreeDecodeResultRecord(std::size_t *out_record_index) const;
    bool FindDecodedPayloadRecord(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::uint64_t decode_plan_id,
        std::uint64_t decode_result_id,
        std::uint64_t decoded_payload_id,
        std::size_t *out_record_index) const;
    bool FindFreeDecodedPayloadRecord(std::size_t *out_record_index) const;
    bool StoreLoadCommitRecord(const ResourceLoadCommitRequest &request);
    bool StoreResidencyRecord(
        ResourceResidencyOperation operation,
        const ResourceResidencyRequest &request,
        ResourceResidencyStatus status,
        ResourceResidencyState state);
    bool IsResidentState(ResourceResidencyState state) const;
    bool IsEvictionCandidate(const ResourceSlot &slot) const;
    void RemoveResidentCounters(const ResourceSlot &slot);
    void RefreshEvictableState(ResourceSlot &slot);
    void ClearResidencySlot(ResourceSlot &slot);
    void ClearDecodePlanRecord(std::size_t record_index);
    void ClearDecodePlansForCachePayload(ResourceHandle resource, std::uint64_t payload_id);
    void ClearDecodedPayloadRecord(std::size_t record_index);
    void ClearDecodedPayloadsForDecodeResult(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::uint64_t decode_plan_id,
        std::uint64_t decode_result_id);
    void ClearDecodeResultRecord(std::size_t record_index);
    void ClearDecodeResultsForDecodePlan(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::uint64_t decode_plan_id);
    void ClearDecodeResultsForCachePayload(ResourceHandle resource, std::uint64_t payload_id);
    void ClearCachePayloadRecord(std::size_t record_index);
    void ClearCachePayloadForSlot(std::size_t slot_index);
    bool HasInboundEdge(std::size_t slot_index) const;
    bool HasDependencyPath(std::size_t start_slot, std::size_t target_slot) const;
    void ClearOutboundEdges(std::size_t slot_index);
    void AdvanceGeneration(ResourceSlot& slot);

    std::array<ResourceSlot, MAX_RESOURCE_COUNT> slots_;
    std::array<ResourceDependencyEdge, MAX_DEPENDENCY_EDGE_COUNT> dependency_edges_;
    std::array<ResourceLoadCommitRecord, MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT> load_commit_records_;
    std::array<ResourceResidencyRecord, MAX_RESOURCE_RESIDENCY_RECORD_COUNT> residency_records_;
    std::array<ResourceCachePayloadRecord, MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT> cache_payload_records_;
    std::array<ResourceDecodePlanRecord, MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT> decode_plan_records_;
    std::array<ResourceDecodeResultRecord, MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT> decode_result_records_;
    std::array<ResourceDecodedPayloadRecord, MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT> decoded_payload_records_;
    std::array<
        std::array<std::uint8_t, MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD>,
        MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT> cache_payload_bytes_;
    std::array<
        std::array<std::uint8_t, MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD>,
        MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT> decoded_payload_bytes_;
    std::array<ResourceTypeId, MAX_RESOURCE_TYPE_COUNT> types_;
    ResourceSnapshot snapshot_;
    ResourceResidencySnapshot residency_snapshot_;
    ResourceCachePayloadSnapshot cache_payload_snapshot_;
    ResourceDecodePlanSnapshot decode_plan_snapshot_;
    ResourceDecodeResultSnapshot decode_result_snapshot_;
    ResourceDecodedPayloadSnapshot decoded_payload_snapshot_;
};
}
