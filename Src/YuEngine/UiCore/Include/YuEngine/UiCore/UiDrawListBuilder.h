// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDrawListBuilder.h

#pragma once

#include <span>

#include "YuEngine/UiCore/UiDrawElement.h"
#include "YuEngine/UiCore/UiDrawElementDesc.h"
#include "YuEngine/UiCore/UiDrawListResult.h"
#include "YuEngine/UiCore/UiDrawListStatus.h"
#include "YuEngine/UiCore/UiNodeTree.h"

namespace yuengine::uicore {
class UiDrawListBuilder final {
public:
    /**
     * @comment 构建 deterministic draw-element list。
     * @param tree 输入 node tree。
     * @param descs 输入 draw element descriptors。
     * @param out_elements 调用方持有的输出 buffer。
     * @param out_result 输出 build result。
     * @return 显式 build 状态。
     */
    UiDrawListStatus Build(
        const UiNodeTree &tree,
        std::span<const UiDrawElementDesc> descs,
        std::span<UiDrawElement> out_elements,
        UiDrawListResult *out_result) const;

private:
    UiDrawListStatus Validate(
        const UiNodeTree &tree,
        std::span<const UiDrawElementDesc> descs,
        std::span<UiDrawElement> out_elements,
        UiDrawListResult *out_result) const;
    UiDrawElement BuildElement(const UiNodeRecord &record, const UiDrawElementDesc &desc) const;
};
}
