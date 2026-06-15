// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentQueryBridge.h

#pragma once

#include "YuEngine/World/WorldComponentQueryDesc.h"
#include "YuEngine/World/WorldComponentQueryResult.h"
#include "YuEngine/World/WorldComponentQuerySnapshot.h"
#include "YuEngine/World/WorldComponentQueryStatus.h"

namespace yuengine::world {
class WorldComponentQueryBridge final {
public:
    /**
     * @comment Constructs a world component query bridge.
     */
    WorldComponentQueryBridge();

    /**
     * @comment Queries world object ids for one component type id.
     * @param desc Input type query descriptor.
     * @return Explicit operation result.
     */
    WorldComponentQueryResult QueryType(const WorldComponentQueryTypeDesc &desc);
    /**
     * @comment Queries component attachments for one world object id.
     * @param desc Input object query descriptor.
     * @return Explicit operation result.
     */
    WorldComponentQueryResult QueryObject(const WorldComponentQueryObjectDesc &desc);
    /**
     * @comment Returns a snapshot of the current query bridge state.
     * @return Snapshot value.
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
