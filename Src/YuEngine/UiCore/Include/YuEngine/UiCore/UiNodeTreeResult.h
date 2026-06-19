// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiNodeTreeResult.h

#pragma once

#include "YuEngine/UiCore/UiNodeRecord.h"
#include "YuEngine/UiCore/UiNodeTreeStatus.h"

namespace yuengine::uicore {
struct UiNodeTreeResult final {
    UiNodeTreeStatus status = UiNodeTreeStatus::Success;
    UiNodeRecord record;

    /**
     * @comment 创建成功 result。
     * @param record 输入 node record。
     * @return 显式操作结果。
     */
    static UiNodeTreeResult Success(const UiNodeRecord &record) {
        return UiNodeTreeResult{UiNodeTreeStatus::Success, record};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 status。
     * @return 显式操作结果。
     */
    static UiNodeTreeResult Failure(UiNodeTreeStatus status) {
        return UiNodeTreeResult{status, UiNodeRecord{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiNodeTreeStatus::Success;
    }
};
}
