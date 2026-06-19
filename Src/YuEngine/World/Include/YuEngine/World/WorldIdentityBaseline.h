// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldIdentityBaseline.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Object/ObjectRegistry.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"
#include "YuEngine/World/WorldIdentityBaselineDesc.h"
#include "YuEngine/World/WorldIdentityBaselineObjectDesc.h"
#include "YuEngine/World/WorldIdentityBaselineRecord.h"
#include "YuEngine/World/WorldIdentityBaselineResult.h"
#include "YuEngine/World/WorldIdentityBaselineSnapshot.h"
#include "YuEngine/World/WorldIdentityBaselineStatus.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectIdentityBridge.h"
#include "YuEngine/World/WorldTransformBridge.h"

namespace yuengine::world {
class WorldIdentityBaseline final {
public:
    /**
     * @comment 构造 L1 object/world identity baseline。
     * @param desc 输入 baseline descriptor。
     */
    explicit WorldIdentityBaseline(WorldIdentityBaselineDesc desc=WorldIdentityBaselineDesc{});

    /**
     * @comment 创建 object registry、world identity、transform 和 component identity record。
     * @param desc 输入 object baseline descriptor。
     * @return 显式操作结果。
     */
    WorldIdentityBaselineResult CreateObject(const WorldIdentityBaselineObjectDesc &desc);
    /**
     * @comment 查询 active object baseline record。
     * @param world_object_id 输入 world object id。
     * @return 显式操作结果。
     */
    WorldIdentityBaselineResult QueryObject(WorldObjectId world_object_id);
    /**
     * @comment 销毁 object baseline 并清理所有 sidecar identity records。
     * @param world_object_id 输入 world object id。
     * @return 显式操作状态。
     */
    WorldIdentityBaselineStatus DestroyObject(WorldObjectId world_object_id);
    /**
     * @comment 按 deterministic slot order 导出 active baseline records。
     * @param output_records 调用方持有的 output buffer。
     * @param output_capacity output buffer capacity。
     * @return active record 总数。
     */
    std::uint32_t ExportRecords(
        WorldIdentityBaselineRecord *output_records,
        std::uint32_t output_capacity) const;
    /**
     * @comment 返回当前 baseline 状态快照。
     * @return 快照值。
     */
    WorldIdentityBaselineSnapshot Snapshot() const;

private:
    WorldIdentityBaselineStatus RecordFailure(WorldIdentityBaselineStatus status);
    WorldIdentityBaselineResult RecordFailureResult(WorldIdentityBaselineStatus status);
    void RecordSuccess();
    WorldIdentityBaselineStatus ValidateBaselineCapacity() const;
    WorldIdentityBaselineStatus ValidateObjectDesc(const WorldIdentityBaselineObjectDesc &desc) const;
    WorldIdentityBaselineStatus ValidateRecordForMutation(
        WorldObjectId world_object_id,
        WorldIdentityBaselineRecord *&out_record);
    WorldIdentityBaselineRecord *FindRecordByWorldObjectId(WorldObjectId world_object_id);
    const WorldIdentityBaselineRecord *FindRecordByWorldObjectId(WorldObjectId world_object_id) const;
    WorldIdentityBaselineRecord *FindFreeRecord();
    void ClearRecord(WorldIdentityBaselineRecord &record);
    void RecountRecords();
    void RollbackCreatedObject(WorldObjectId world_object_id, yuengine::object::ObjectHandle object_handle);
    void RollbackComponent(WorldObjectId world_object_id, WorldComponentTypeId component_type_id);
    void RollbackIdentity(WorldObjectId world_object_id);
    void RollbackTransform(WorldObjectId world_object_id);
    void RollbackWorldObject(WorldObjectId world_object_id);
    void RollbackObjectHandle(yuengine::object::ObjectHandle object_handle);

    yuengine::object::ObjectRegistry object_registry_;
    WorldInstance world_;
    WorldObjectIdentityBridge identity_bridge_;
    WorldTransformBridge transform_bridge_;
    WorldComponentAttachmentBridge component_bridge_;
    std::array<WorldIdentityBaselineRecord, MAX_WORLD_OBJECT_COUNT> records_;
    WorldIdentityBaselineSnapshot snapshot_;
};
}
