// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceRegistry.h

#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

#include "YuEngine/Resource/ResourceConstants.h"
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
    ResourceLoadCommitStatus ValidateLoadCommitRequest(
        const ResourceLoadCommitRequest &request,
        std::size_t *out_slot_index) const;
    ResourceResidencyStatus ValidateResidencyRequest(
        const ResourceResidencyRequest &request,
        std::size_t *out_slot_index) const;
    ResourceLoadCommitStatus MapHandleStatus(ResourceStatus status) const;
    ResourceStatus MapLoadCommitStatus(ResourceLoadCommitStatus status) const;
    ResourceResidencyStatus MapHandleResidencyStatus(ResourceStatus status) const;
    ResourceStatus MapResidencyStatus(ResourceResidencyStatus status) const;
    ResourceStatus ResolveHandle(ResourceHandle handle, std::size_t& out_index) const;
    ResourceStatus RegisterTypeIfNeeded(ResourceTypeId type);
    bool HasType(ResourceTypeId type) const;
    bool HasDuplicateActiveResource(const ResourceDescriptor& descriptor) const;
    bool HasLoadCommitId(std::uint64_t commit_id) const;
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
    bool HasInboundEdge(std::size_t slot_index) const;
    bool HasDependencyPath(std::size_t start_slot, std::size_t target_slot) const;
    void ClearOutboundEdges(std::size_t slot_index);
    void AdvanceGeneration(ResourceSlot& slot);

    std::array<ResourceSlot, MAX_RESOURCE_COUNT> slots_;
    std::array<ResourceDependencyEdge, MAX_DEPENDENCY_EDGE_COUNT> dependency_edges_;
    std::array<ResourceLoadCommitRecord, MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT> load_commit_records_;
    std::array<ResourceResidencyRecord, MAX_RESOURCE_RESIDENCY_RECORD_COUNT> residency_records_;
    std::array<ResourceTypeId, MAX_RESOURCE_TYPE_COUNT> types_;
    ResourceSnapshot snapshot_;
    ResourceResidencySnapshot residency_snapshot_;
};
}
