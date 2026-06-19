// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiRectMathResult.h

#pragma once

#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiRectMathStatus.h"
#include "YuEngine/UiCore/UiVector2.h"

namespace yuengine::uicore {
struct UiRectMathResult final {
    UiRectMathStatus status = UiRectMathStatus::Success;
    UiRect rect;
    UiRect content_rect;
    UiVector2 pivot_point;

    /**
     * @comment 检查 rect math 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiRectMathStatus::Success;
    }
};
}
