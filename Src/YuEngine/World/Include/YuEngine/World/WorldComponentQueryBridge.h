// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentQueryBridge.h

#pragma once

#include "YuEngine/World/WorldComponentQueryDesc.h"
#include "YuEngine/World/WorldComponentQueryResult.h"
#include "YuEngine/World/WorldComponentQuerySnapshot.h"
#include "YuEngine/World/WorldComponentQueryStatus.h"

namespace yuengine::world {
class WorldComponentQueryBridge final {
public:
    /**
     * @comment 构造 world component query bridge。
     */
    WorldComponentQueryBridge();

    /**
     * @comment 查询 world object ids for one component type id。
     * @param desc 输入 type query descriptor。
     * @return 显式操作结果。
     */
    WorldComponentQueryResult QueryType(const WorldComponentQueryTypeDesc &desc);
    /**
     * @comment 查询 component attachments for one world object id。
     * @param desc 输入 object query descriptor。
     * @return 显式操作结果。
     */
    WorldComponentQueryResult QueryObject(const WorldComponentQueryObjectDesc &desc);
    /**
     * @comment 返回当前 query bridge 状态快照。
     * @return 快照值。
     */
    WorldComponentQuerySnapshot Snapshot() const;

private:
    WorldComponentQueryResult RecordSuccessResult(
        std::uint32_t matched_record_count,
        std::uint32_t written_record_count);
    WorldComponentQueryResult RecordFailureResult(WorldComponentQueryStatus status);
    WorldComponentQueryResult RecordOverflowResult(
        std::uint32_t matched_record_count,
        std::uint32_t written_record_count);
    WorldComponentQueryStatus ValidateTypeDesc(const WorldComponentQueryTypeDesc &desc) const;
    WorldComponentQueryStatus ValidateObjectDesc(const WorldComponentQueryObjectDesc &desc) const;

    WorldComponentQuerySnapshot snapshot_;
};
}
