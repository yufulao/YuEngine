// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeleton.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderCore/RenderGraphSkeletonConstants.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonDesc.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonRecord.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonRequest.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonResult.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonSnapshot.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Owns bounded RenderCore render graph declaration and dependency validation records.
 */
class RenderGraphSkeleton final {
public:
    /**
     * @comment Constructs a RenderGraphSkeleton instance.
     * @param desc Input descriptor.
     */
    explicit RenderGraphSkeleton(const RenderGraphSkeletonDesc &desc=RenderGraphSkeletonDesc());

    /**
     * @comment Validates graph declarations and prepares a submission batch request without executing it.
     * @param request Caller-owned graph skeleton request.
     * @return Explicit operation result.
     */
    RenderGraphSkeletonResult Prepare(const RenderGraphSkeletonRequest &request);
    /**
     * @comment Copies retained graph records into caller-owned output storage.
     * @param output Caller-owned graph record output storage.
     * @return Number of records copied.
     */
    std::size_t QueryRecords(std::span<RenderGraphSkeletonRecord> output) const;
    /**
     * @comment Releases one retained graph declaration record.
     * @param graph_id Graph identifier to release.
     * @return Explicit operation status.
     */
    RenderGraphSkeletonStatus Release(std::uint32_t graph_id);
    /**
     * @comment Returns the current render graph skeleton snapshot.
     * @return Snapshot value.
     */
    RenderGraphSkeletonSnapshot Snapshot() const;
    /**
     * @comment Resets bounded graph declaration records and counters.
     */
    void Reset();

private:
    struct Record final {
        RenderGraphSkeletonRecord record{};
    };

    RenderGraphSkeletonStatus ValidateRequest(
        const RenderGraphSkeletonRequest &request,
        RenderGraphSkeletonResult *result) const;
    bool HasGraphId(std::uint32_t graph_id) const;
    bool HasRecordCapacity() const;
    void RecordRejectedResult(const RenderGraphSkeletonResult &result);
    void RecordPreparedResult(const RenderGraphSkeletonResult &result);
    void RecordReleaseResult(std::uint32_t graph_id, RenderGraphSkeletonStatus status);

    RenderGraphSkeletonDesc desc_;
    RenderGraphSkeletonSnapshot snapshot_;
    std::array<Record, MAX_RENDER_GRAPH_SKELETON_RECORDS> records_;
};
}
