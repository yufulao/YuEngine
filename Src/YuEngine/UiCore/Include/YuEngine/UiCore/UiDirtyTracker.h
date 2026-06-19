// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDirtyTracker.h

#pragma once

#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiDirtyState.h"

namespace yuengine::uicore {
class UiDirtyTracker final {
public:
    /**
     * @comment 应用 UI dirty change。
     * @param change_type 输入 change type。
     * @return 当前 dirty state。
     */
    UiDirtyState ApplyChange(UiDirtyChangeType change_type);
    /**
     * @comment 清空 dirty state。
     */
    void Clear();
    /**
     * @comment 返回当前 dirty state。
     * @return dirty state 值。
     */
    UiDirtyState Snapshot() const;

private:
    void Mark(std::uint32_t domains);

    UiDirtyState state_;
};
}
