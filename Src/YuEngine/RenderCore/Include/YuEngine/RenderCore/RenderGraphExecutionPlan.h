// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphExecutionPlan.h

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
 * @comment 持有固定容量 RenderCore render graph execution-plan 元数据记录。
 */
class RenderGraphExecutionPlan final {
public:
    /**
     * @comment 构造 RenderGraphExecutionPlan 实例。
     * @param desc 输入描述。
     */
    explicit RenderGraphExecutionPlan(
        const RenderGraphExecutionPlanDesc &desc=RenderGraphExecutionPlanDesc());

    /**
     * @comment 验证准备好的 graph 结果并执行一次 frame packet 交接。
     * @param request 调用方持有的 execution-plan 请求。
     * @return 显式操作结果。
     */
    RenderGraphExecutionPlanResult Execute(const RenderGraphExecutionPlanRequest &request);
    /**
     * @comment 将保留的 execution-plan 记录复制到调用方持有输出存储。
     * @param output 调用方持有的 plan 记录输出存储。
     * @return 已复制的记录数量。
     */
    std::size_t QueryRecords(std::span<RenderGraphExecutionPlanRecord> output);
    /**
     * @comment 释放一条保留的 execution-plan 元数据记录。
     * @param plan_id 要释放的 Plan 标识符。
     * @return 显式操作状态。
     */
    RenderGraphExecutionPlanStatus Release(std::uint32_t plan_id);
    /**
     * @comment 返回当前 execution-plan 快照。
     * @return 快照值。
     */
    RenderGraphExecutionPlanSnapshot Snapshot() const;
    /**
     * @comment 重置固定容量 execution-plan 记录和计数。
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
