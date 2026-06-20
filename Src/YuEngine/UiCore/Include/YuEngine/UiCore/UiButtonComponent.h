// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiButtonComponent.h

#pragma once

#include "YuEngine/UiCore/UiButtonComponentDesc.h"
#include "YuEngine/UiCore/UiButtonComponentResult.h"
#include "YuEngine/UiCore/UiButtonComponentStatus.h"
#include "YuEngine/UiCore/UiButtonVisualStateDesc.h"
#include "YuEngine/UiCore/UiButtonVisualUpdate.h"
#include "YuEngine/UiCore/UiNodeRecord.h"
#include "YuEngine/UiCore/UiNodeTree.h"

namespace yuengine::uicore {
class UiButtonComponent final {
public:
    /**
     * @comment 解析 Button component 状态、visual update 和 activation hooks。
     * @param tree UI node tree。
     * @param desc Button component 描述。
     * @param out_result 输出 component result。
     * @return 显式 component 状态。
     */
    UiButtonComponentStatus Build(
        const UiNodeTree &tree,
        const UiButtonComponentDesc &desc,
        UiButtonComponentResult *out_result) const;

private:
    UiButtonComponentStatus ValidateDesc(const UiButtonComponentDesc &desc) const;
    UiButtonState ResolveState(const UiButtonComponentDesc &desc, const UiNodeRecord &record) const;
    UiButtonActivationSource ResolveActivationSource(
        const UiButtonComponentDesc &desc,
        UiButtonState state) const;
    const UiButtonVisualStateDesc &SelectVisual(
        const UiButtonComponentDesc &desc,
        UiButtonState state) const;
    UiButtonVisualUpdate BuildVisualUpdate(
        const UiButtonComponentDesc &desc,
        const UiButtonVisualStateDesc &visual) const;
};
}
