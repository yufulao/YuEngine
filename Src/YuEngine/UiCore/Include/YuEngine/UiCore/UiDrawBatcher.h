// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDrawBatcher.h

#pragma once

#include <span>

#include "YuEngine/UiCore/UiDrawBatch.h"
#include "YuEngine/UiCore/UiDrawBatchResult.h"
#include "YuEngine/UiCore/UiDrawBatchStatus.h"
#include "YuEngine/UiCore/UiDrawElement.h"

namespace yuengine::uicore {
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

private:
    UiDrawBatchStatus ValidateElements(
        std::span<const UiDrawElement> elements,
        UiDrawBatchResult *out_result) const;
    UiDrawBatchStatus ValidateElement(
        const UiDrawElement &element,
        std::uint32_t index,
        UiDrawBatchResult *out_result) const;
    std::uint32_t CountBatches(std::span<const UiDrawElement> elements) const;
    void WriteBatches(std::span<const UiDrawElement> elements, std::span<UiDrawBatch> out_batches) const;
    UiDrawBatchKey BuildKey(const UiDrawElement &element) const;
    bool KeyMatches(const UiDrawBatchKey &left, const UiDrawBatchKey &right) const;
    bool RectMatches(const UiRect &left, const UiRect &right) const;
};
}
