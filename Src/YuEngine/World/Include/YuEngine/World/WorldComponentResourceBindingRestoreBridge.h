// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingRestoreBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentResourceBinding.h"
#include "YuEngine/World/WorldComponentResourceBindingResult.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreBridgeDesc.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreResult.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreSnapshot.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreStatus.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::world {
class WorldComponentAttachmentBridge;
class WorldComponentResourceBindingBridge;

class WorldComponentResourceBindingRestoreBridge final {
public:
    /**
     * @comment Constructs a world component resource binding restore bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldComponentResourceBindingRestoreBridge(
        WorldComponentResourceBindingRestoreBridgeDesc desc=
            WorldComponentResourceBindingRestoreBridgeDesc{});

    /**
     * @comment Applies caller-owned binding records to an empty destination bridge.
     * @param destination_bridge Caller-owned destination binding bridge.
     * @param attachment_source Caller-owned component attachment source.
     * @param resource_registry Caller-owned resource registry.
     * @param input_bindings Caller-owned input binding records.
     * @param input_binding_count Input binding record count.
     * @return Explicit operation result.
     */
    WorldComponentResourceBindingRestoreResult Restore(
        WorldComponentResourceBindingBridge *destination_bridge,
        const WorldComponentAttachmentBridge *attachment_source,
        yuengine::resource::ResourceRegistry *resource_registry,
        const WorldComponentResourceBinding *input_bindings,
        std::uint32_t input_binding_count);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldComponentResourceBindingRestoreSnapshot Snapshot() const;

private:
    WorldComponentResourceBindingRestoreResult RecordFailure(
        WorldComponentResourceBindingRestoreStatus status);
    WorldComponentResourceBindingRestoreResult RecordFailure(
        WorldComponentResourceBindingRestoreStatus status,
        WorldComponentResourceBindingStatus binding_status);
    WorldComponentResourceBindingRestoreResult RecordFailure(
        WorldComponentResourceBindingRestoreStatus status,
        yuengine::resource::ResourceStatus resource_status);
    WorldComponentResourceBindingRestoreResult RecordFailure(
        WorldComponentResourceBindingRestoreStatus status,
        WorldComponentResourceBindingStatus binding_status,
        yuengine::resource::ResourceStatus resource_status);
    WorldComponentResourceBindingRestoreResult RecordRejectedFailure(
        WorldComponentResourceBindingRestoreStatus status);
    WorldComponentResourceBindingRestoreResult RecordRejectedFailure(
        WorldComponentResourceBindingRestoreStatus status,
        yuengine::resource::ResourceStatus resource_status);
    WorldComponentResourceBindingRestoreResult RecordSuccess(
        const WorldComponentResourceBindingRestoreState &state);
    WorldComponentResourceBindingRestoreStatus ValidateBridgeCapacity() const;
    WorldComponentResourceBindingRestoreStatus ValidateDestination(
        const WorldComponentResourceBindingBridge &destination_bridge,
        std::uint32_t input_binding_count) const;
    WorldComponentResourceBindingRestoreStatus ValidateRecords(
        const WorldComponentAttachmentBridge &attachment_source,
        const yuengine::resource::ResourceRegistry &resource_registry,
        const WorldComponentResourceBinding *input_bindings,
        std::uint32_t input_binding_count,
        yuengine::resource::ResourceStatus *out_resource_status) const;
    WorldComponentResourceBindingRestoreStatus ValidateRecord(
        const WorldComponentAttachmentBridge &attachment_source,
        const yuengine::resource::ResourceRegistry &resource_registry,
        const WorldComponentResourceBinding *input_bindings,
        std::uint32_t record_index,
        yuengine::resource::ResourceStatus *out_resource_status) const;
    bool HasAttachmentTuple(
        const WorldComponentAttachmentBridge &attachment_source,
        const WorldComponentResourceBinding &binding) const;
    bool HasDuplicateInput(
        const WorldComponentResourceBinding *input_bindings,
        std::uint32_t record_index) const;
    std::uint32_t CountProjectedAcquire(
        const WorldComponentResourceBinding *input_bindings,
        std::uint32_t record_index) const;
    WorldComponentResourceBindingRestoreStatus MapBindingStatus(
        WorldComponentResourceBindingStatus binding_status) const;
    WorldComponentResourceBindingRestoreStatus MapResourceStatus(
        yuengine::resource::ResourceStatus resource_status) const;
    WorldComponentResourceBindingRestoreResult RollbackAndRecordApplyFailure(
        WorldComponentResourceBindingBridge *destination_bridge,
        yuengine::resource::ResourceRegistry *resource_registry,
        const WorldComponentResourceBindingRestoreState &state,
        const WorldComponentResourceBindingResult &binding_result);

    std::uint32_t binding_capacity_;
    WorldComponentResourceBindingRestoreSnapshot snapshot_;
};
}
