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
    explicit ResourceRegistry(resource_registry_desc_t desc);

    resource_registration_result_t RegisterSyntheticDescriptor(const resource_descriptor_t& descriptor);
    ResourceStatus AddDependency(resource_handle_t dependent, resource_handle_t dependency);
    ResourceStatus Acquire(resource_handle_t handle, resource_type_id_t expectedType);
    ResourceStatus Release(resource_handle_t handle);
    ResourceStatus Retire(resource_handle_t handle);
    resource_snapshot_t Snapshot() const;

private:
    ResourceStatus RecordFailure(ResourceStatus status);
    void RecordSuccess();
    ResourceStatus ResolveHandle(resource_handle_t handle, std::size_t& outIndex) const;
    ResourceStatus RegisterTypeIfNeeded(resource_type_id_t type);
    bool HasType(resource_type_id_t type) const;
    bool HasDuplicateActiveResource(const resource_descriptor_t& descriptor) const;
    bool HasInboundEdge(std::size_t slotIndex) const;
    bool HasDependencyPath(std::size_t startSlot, std::size_t targetSlot) const;
    void ClearOutboundEdges(std::size_t slotIndex);
    void AdvanceGeneration(resource_slot_t& slot);

    std::array<resource_slot_t, MAX_RESOURCE_COUNT> _slots;
    std::array<resource_dependency_edge_t, MAX_DEPENDENCY_EDGE_COUNT> _dependencyEdges;
    std::array<resource_type_id_t, MAX_RESOURCE_TYPE_COUNT> _types;
    resource_snapshot_t _snapshot;
};
}
