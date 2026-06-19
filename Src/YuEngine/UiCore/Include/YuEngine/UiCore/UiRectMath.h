// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiRectMath.h

#pragma once

#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiRectMathResult.h"
#include "YuEngine/UiCore/UiRectTransform.h"

namespace yuengine::uicore {
class UiRectMath final {
public:
    /**
     * @comment 根据 parent rect 和 rect transform 计算 world rect。
     * @param parent_rect 输入 parent rect。
     * @param transform 输入 rect transform。
     * @return 显式计算结果。
     */
    static UiRectMathResult Resolve(const UiRect &parent_rect, const UiRectTransform &transform);
};
}
