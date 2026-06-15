// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceRegistry.h

#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDependencyEdge.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceRegistrationResult.h"
#include "YuEngine/Resource/ResourceRegistryDesc.h"
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
    ResourceStatus ResolveHandle(ResourceHandle handle, std::size_t& out_index) const;
    ResourceStatus RegisterTypeIfNeeded(ResourceTypeId type);
    bool HasType(ResourceTypeId type) const;
    bool HasDuplicateActiveResource(const ResourceDescriptor& descriptor) const;
    bool HasInboundEdge(std::size_t slot_index) const;
    bool HasDependencyPath(std::size_t start_slot, std::size_t target_slot) const;
    void ClearOutboundEdges(std::size_t slot_index);
    void AdvanceGeneration(ResourceSlot& slot);

    std::array<ResourceSlot, MAX_RESOURCE_COUNT> slots_;
    std::array<ResourceDependencyEdge, MAX_DEPENDENCY_EDGE_COUNT> dependency_edges_;
    std::array<ResourceTypeId, MAX_RESOURCE_TYPE_COUNT> types_;
    ResourceSnapshot snapshot_;
};
}
