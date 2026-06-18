// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeleton.h

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
 * @comment 持有 固定容量 RenderCore render graph declaration 和 dependency validation 记录.
 */
class RenderGraphSkeleton final {
public:
    /**
     * @comment 构造 RenderGraphSkeleton 实例。
     * @param desc 输入描述。
     */
    explicit RenderGraphSkeleton(const RenderGraphSkeletonDesc &desc=RenderGraphSkeletonDesc());

    /**
     * @comment 验证 graph declarations 和 prepares 一个 submission batch 请求 且不 executing it。
     * @param request 调用方持有的 graph skeleton 请求。
     * @return 显式操作结果。
     */
    RenderGraphSkeletonResult Prepare(const RenderGraphSkeletonRequest &request);
    /**
     * @comment 复制 保留的 graph 记录 写入 调用方持有 output 存储。
     * @param output 调用方持有的 graph 记录 output 存储。
     * @return 已复制的记录数量。
     */
    std::size_t QueryRecords(std::span<RenderGraphSkeletonRecord> output) const;
    /**
     * @comment 释放 一个 保留的 graph declaration 记录.
     * @param graph_id 要释放的 Graph 标识符。
     * @return 显式操作状态。
     */
    RenderGraphSkeletonStatus Release(std::uint32_t graph_id);
    /**
     * @comment 返回当前 render graph skeleton 快照。
     * @return 快照值。
     */
    RenderGraphSkeletonSnapshot Snapshot() const;
    /**
     * @comment 重置固定容量 graph declaration 记录和计数。
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
