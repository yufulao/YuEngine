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
    ResourceStatus Acquire(ResourceHandle handle, ResourceTypeId expectedType);
    ResourceStatus Release(ResourceHandle handle);
    ResourceStatus Retire(ResourceHandle handle);
    ResourceSnapshot Snapshot() const;

private:
    ResourceStatus RecordFailure(ResourceStatus status);
    void RecordSuccess();
    ResourceStatus ResolveHandle(ResourceHandle handle, std::size_t& outIndex) const;
    ResourceStatus RegisterTypeIfNeeded(ResourceTypeId type);
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
