// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDrawBatcher.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiDrawBatch.h"
#include "YuEngine/UiCore/UiDrawBatchResult.h"
#include "YuEngine/UiCore/UiDrawBatchStatus.h"
#include "YuEngine/UiCore/UiDrawElement.h"

namespace yuengine::uicore {
struct UiDrawBatcherSnapshot final {
    std::uint32_t draw_element_count = 0U;
    std::uint32_t batch_count = 0U;
    std::uint32_t accepted_operation_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    UiDrawBatchStatus last_status = UiDrawBatchStatus::Success;
    std::uint32_t last_capacity_entry_output_index = 0U;
    UiNodeId last_capacity_entry_node_id;
    UiDrawBatchKey last_capacity_entry_key;
    std::uint32_t last_capacity_entry_output_capacity = 0U;
    std::uint32_t last_capacity_entry_written_batch_count = 0U;
    std::uint32_t last_required_output_batch_count = 0U;
};

class UiDrawBatcher final {
public:
    /**
     * @comment 将 ordered draw elements 合并为 deterministic draw batches。
     * @param elements 已按绘制顺序排序的 draw elements。
     * @param out_batches 调用方持有的输出 batch buffer。
     * @param out_result 输出 batch result。
     * @return 显式 batch 状态。
     */
    UiDrawBatchStatus Build(
        std::span<const UiDrawElement> elements,
        std::span<UiDrawBatch> out_batches,
        UiDrawBatchResult *out_result) const;
    /**
     * @comment 返回当前 batcher 状态快照。
     * @return 快照值。
     */
    UiDrawBatcherSnapshot Snapshot() const;

private:
    UiDrawBatchStatus RecordFailure(UiDrawBatchStatus status) const;
    UiDrawBatchStatus RecordSuccess() const;
    void ClearOutputCapacityEntry() const;
    UiDrawBatchStatus ValidateElements(
        std::span<const UiDrawElement> elements,
        UiDrawBatchResult *out_result) const;
    UiDrawBatchStatus ValidateElement(
        const UiDrawElement &element,
        std::uint32_t index,
        UiDrawBatchResult *out_result) const;
    std::uint32_t CountBatches(std::span<const UiDrawElement> elements) const;
    void RecordOutputCapacityFailure(
        std::span<const UiDrawElement> elements,
        std::size_t output_batch_capacity,
        std::uint32_t required_batch_count,
        UiDrawBatchResult *out_result) const;
    void WriteBatches(std::span<const UiDrawElement> elements, std::span<UiDrawBatch> out_batches) const;
    UiDrawBatchKey BuildKey(const UiDrawElement &element) const;
    bool KeyMatches(const UiDrawBatchKey &left, const UiDrawBatchKey &right) const;
    bool RectMatches(const UiRect &left, const UiRect &right) const;

    mutable UiDrawBatcherSnapshot snapshot_;
};
}
