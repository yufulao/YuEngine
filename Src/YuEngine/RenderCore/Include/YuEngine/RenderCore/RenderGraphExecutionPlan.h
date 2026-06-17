// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlan.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderCore/RenderGraphExecutionPlanConstants.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanDesc.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanRecord.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanRequest.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanResult.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanSnapshot.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Owns bounded RenderCore render graph execution-plan metadata records.
 */
class RenderGraphExecutionPlan final {
public:
    /**
     * @comment Constructs a RenderGraphExecutionPlan instance.
     * @param desc Input descriptor.
     */
    explicit RenderGraphExecutionPlan(
        const RenderGraphExecutionPlanDesc &desc=RenderGraphExecutionPlanDesc());

    /**
     * @comment Validates a prepared graph result and executes one frame packet handoff.
     * @param request Caller-owned execution-plan request.
     * @return Explicit operation result.
     */
    RenderGraphExecutionPlanResult Execute(const RenderGraphExecutionPlanRequest &request);
    /**
     * @comment Copies retained execution-plan records into caller-owned output storage.
     * @param output Caller-owned plan record output storage.
     * @return Number of records copied.
     */
    std::size_t QueryRecords(std::span<RenderGraphExecutionPlanRecord> output);
    /**
     * @comment Releases one retained execution-plan metadata record.
     * @param plan_id Plan identifier to release.
     * @return Explicit operation status.
     */
    RenderGraphExecutionPlanStatus Release(std::uint32_t plan_id);
    /**
     * @comment Returns the current execution-plan snapshot.
     * @return Snapshot value.
     */
    RenderGraphExecutionPlanSnapshot Snapshot() const;
    /**
     * @comment Resets bounded execution-plan records and counters.
     */
    void Reset();

private:
    struct Record final {
        RenderGraphExecutionPlanRecord record{};
    };

    RenderGraphExecutionPlanStatus ValidateRequest(
        const RenderGraphExecutionPlanRequest &request,
        RenderGraphExecutionPlanResult *result) const;
    bool HasPlanId(std::uint32_t plan_id) const;
    bool HasGraphId(std::uint32_t graph_id) const;
    bool HasRecordCapacity() const;
    void RecordRejectedResult(const RenderGraphExecutionPlanResult &result);
    void RecordCompletedResult(const RenderGraphExecutionPlanResult &result);
    void RecordFrameFailedResult(const RenderGraphExecutionPlanResult &result);
    void RecordReleaseResult(std::uint32_t plan_id, RenderGraphExecutionPlanStatus status);
    void StoreRecord(const RenderGraphExecutionPlanResult &result);
    void StoreLastResult(const RenderGraphExecutionPlanResult &result);

    RenderGraphExecutionPlanDesc desc_;
    RenderGraphExecutionPlanSnapshot snapshot_;
    std::array<Record, MAX_RENDER_GRAPH_EXECUTION_PLAN_RECORDS> records_;
};
}
