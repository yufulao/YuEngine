// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiLayoutPass.h

#pragma once

#include <span>

#include "YuEngine/UiCore/UiLayoutContainerDesc.h"
#include "YuEngine/UiCore/UiLayoutPassResult.h"
#include "YuEngine/UiCore/UiNodeRecord.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiRect.h"

namespace yuengine::uicore {
class UiLayoutPass final {
public:
    /**
     * @comment 应用 UI layout containers。
     * @param tree 输入并被更新的 node tree。
     * @param containers 输入 layout container descriptors。
     * @return 显式 layout pass 结果。
     */
    UiLayoutPassResult Apply(UiNodeTree *tree, std::span<const UiLayoutContainerDesc> containers);

private:
    UiLayoutPassStatus ApplyContainer(UiNodeTree *tree, const UiLayoutContainerDesc &desc);
    UiLayoutPassStatus ApplyChildRect(UiNodeTree *tree, const UiNodeRecord &container, UiNodeId child_id, UiRect rect);
    UiLayoutPassStatus ValidateContainer(const UiNodeTree &tree, const UiLayoutContainerDesc &desc) const;
    UiRect BuildChildRect(
        const UiLayoutContainerDesc &desc,
        const UiNodeRecord &container,
        std::uint32_t child_index,
        std::uint32_t child_count) const;
    UiRect BuildStackRect(
        const UiLayoutContainerDesc &desc,
        const UiRect &area,
        std::uint32_t child_index,
        std::uint32_t child_count) const;
    UiRect BuildGridRect(
        const UiLayoutContainerDesc &desc,
        const UiRect &area,
        std::uint32_t child_index,
        std::uint32_t child_count) const;
    UiRect BuildOverlayRect(const UiRect &area) const;
    UiRect BuildScrollViewportRect(const UiLayoutContainerDesc &desc, const UiRect &area) const;

    UiLayoutPassResult result_;
};
}
