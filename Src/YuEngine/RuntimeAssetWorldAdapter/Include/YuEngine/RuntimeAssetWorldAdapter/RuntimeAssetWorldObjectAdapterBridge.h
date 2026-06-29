// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterIdentityRecord.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterResult.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterSnapshot.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::runtimeasset {
struct RuntimeAssetRuntimeInstanceMappingRecord;
}

namespace yuengine::runtimeassetworldadapter {
class RuntimeAssetWorldObjectAdapterBridge final {
public:
    /**
     * @comment 从 RuntimeAsset runtime instance mapping 生成 World restore records。
     * @param request 调用方持有的输入和输出数组。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectAdapterResult BuildRestoreRecords(
        const RuntimeAssetWorldObjectAdapterRequest &request);

    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    RuntimeAssetWorldObjectAdapterSnapshot Snapshot() const;

private:
    RuntimeAssetWorldObjectAdapterResult RecordFailure(
        RuntimeAssetWorldObjectAdapterStatus status);
    RuntimeAssetWorldObjectAdapterResult RecordFailure(
        RuntimeAssetWorldObjectAdapterStatus status,
        std::uint32_t failed_mapping_index,
        std::uint64_t failed_target_id);
    RuntimeAssetWorldObjectAdapterResult RecordSuccess(
        const RuntimeAssetWorldObjectAdapterState &state);
    RuntimeAssetWorldObjectAdapterStatus ValidateRequest(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint32_t *out_failed_mapping_index,
        std::uint64_t *out_failed_target_id) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateRuntimeInstanceMapping(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint32_t mapping_index) const;
    const RuntimeAssetWorldObjectAdapterIdentityRecord *FindIdentityRecord(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint64_t target_id) const;
    bool HasDuplicateMappingTargetId(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint32_t mapping_index) const;
    bool HasDuplicateWorldObjectId(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint32_t mapping_index,
        yuengine::world::WorldObjectId world_object_id) const;
    bool HasDuplicateObjectHandle(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint32_t mapping_index,
        yuengine::object::ObjectHandle object_handle) const;
    bool ObjectHandlesMatch(
        yuengine::object::ObjectHandle left,
        yuengine::object::ObjectHandle right) const;

    RuntimeAssetWorldObjectAdapterSnapshot snapshot_;
};
}
