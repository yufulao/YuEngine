// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingRestoreBridge.h

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
     * @comment 构造 world component resource binding restore bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldComponentResourceBindingRestoreBridge(
        WorldComponentResourceBindingRestoreBridgeDesc desc=
            WorldComponentResourceBindingRestoreBridgeDesc{});

    /**
     * @comment 将调用方持有的 binding records 应用到空 destination bridge。
     * @param destination_bridge 调用方持有的 destination binding bridge。
     * @param attachment_source 调用方持有的 component attachment source。
     * @param resource_registry 调用方持有的 resource registry。
     * @param input_bindings 调用方持有的 input binding records。
     * @param input_binding_count 输入 binding record count。
     * @return 显式操作结果。
     */
    WorldComponentResourceBindingRestoreResult Restore(
        WorldComponentResourceBindingBridge *destination_bridge,
        const WorldComponentAttachmentBridge *attachment_source,
        yuengine::resource::ResourceRegistry *resource_registry,
        const WorldComponentResourceBinding *input_bindings,
        std::uint32_t input_binding_count);
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
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
    WorldComponentResourceBindingRestoreResult RecordFailure(
        WorldComponentResourceBindingRestoreStatus status,
        WorldComponentResourceBindingStatus binding_status,
        yuengine::resource::ResourceStatus resource_status,
        const WorldComponentResourceBindingRestoreState &state);
    WorldComponentResourceBindingRestoreResult RecordRejectedFailure(
        WorldComponentResourceBindingRestoreStatus status);
    WorldComponentResourceBindingRestoreResult RecordRejectedFailure(
        WorldComponentResourceBindingRestoreStatus status,
        const WorldComponentResourceBindingRestoreState &state);
    WorldComponentResourceBindingRestoreResult RecordRejectedFailure(
        WorldComponentResourceBindingRestoreStatus status,
        yuengine::resource::ResourceStatus resource_status);
    WorldComponentResourceBindingRestoreResult RecordRejectedFailure(
        WorldComponentResourceBindingRestoreStatus status,
        yuengine::resource::ResourceStatus resource_status,
        const WorldComponentResourceBindingRestoreState &state);
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
        yuengine::resource::ResourceStatus *out_resource_status,
        WorldComponentResourceBindingRestoreState *state) const;
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
