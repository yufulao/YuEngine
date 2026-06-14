#pragma once

#include <array>
#include <cstddef>

#include "yuengine/resource/resource_constants.h"
#include "yuengine/resource/resource_dependency_edge.h"
#include "yuengine/resource/resource_descriptor.h"
#include "yuengine/resource/resource_registration_result.h"
#include "yuengine/resource/resource_registry_desc.h"
#include "yuengine/resource/resource_slot.h"
#include "yuengine/resource/resource_snapshot.h"

namespace yuengine::resource {
class ResourceRegistry final {
public:
    ResourceRegistry();
    explicit ResourceRegistry(ResourceRegistryDesc desc);

    ResourceRegistrationResult RegisterSyntheticDescriptor(const ResourceDescriptor& descriptor);
    RESOURCE_STATUS AddDependency(ResourceHandle dependent, ResourceHandle dependency);
    RESOURCE_STATUS Acquire(ResourceHandle handle, ResourceTypeId expectedType);
    RESOURCE_STATUS Release(ResourceHandle handle);
    RESOURCE_STATUS Retire(ResourceHandle handle);
    ResourceSnapshot Snapshot() const;

private:
    RESOURCE_STATUS RecordFailure(RESOURCE_STATUS status);
    void RecordSuccess();
    RESOURCE_STATUS ResolveHandle(ResourceHandle handle, std::size_t& outIndex) const;
    RESOURCE_STATUS RegisterTypeIfNeeded(ResourceTypeId type);
    bool HasType(ResourceTypeId type) const;
    bool HasDuplicateActiveResource(const ResourceDescriptor& descriptor) const;
    bool HasInboundEdge(std::size_t slotIndex) const;
    bool HasDependencyPath(std::size_t startSlot, std::size_t targetSlot) const;
    void ClearOutboundEdges(std::size_t slotIndex);
    void AdvanceGeneration(ResourceSlot& slot);

    std::array<ResourceSlot, MAX_RESOURCE_COUNT> _slots;
    std::array<ResourceDependencyEdge, MAX_DEPENDENCY_EDGE_COUNT> _dependencyEdges;
    std::array<ResourceTypeId, MAX_RESOURCE_TYPE_COUNT> _types;
    ResourceSnapshot _snapshot;
};
}
