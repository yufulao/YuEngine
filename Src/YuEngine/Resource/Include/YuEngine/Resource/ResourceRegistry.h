#pragma once

#include <array>
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
    ResourceRegistry();
    explicit ResourceRegistry(ResourceRegistryDesc desc);

    ResourceRegistrationResult RegisterSyntheticDescriptor(const ResourceDescriptor& descriptor);
    ResourceStatus AddDependency(ResourceHandle dependent, ResourceHandle dependency);
    ResourceStatus Acquire(ResourceHandle handle, ResourceTypeId expected_type);
    ResourceStatus Release(ResourceHandle handle);
    ResourceStatus Retire(ResourceHandle handle);
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
