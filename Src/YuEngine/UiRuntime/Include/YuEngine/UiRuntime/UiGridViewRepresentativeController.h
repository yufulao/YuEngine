// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiGridViewRepresentativeController.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/BaseUiController.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleStatus.h"
#include "YuEngine/UiRuntime/UiGridViewRepresentativeSnapshot.h"
#include "YuEngine/UiRuntime/UiGridViewRepresentativeStatus.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgs.h"

namespace yuengine::uiruntime {
class UiGridViewRepresentativeController final : public BaseUiController {
public:
    /**
     * @comment 获取 inventory/store GridView 代表窗口快照。
     * @return 当前代表窗口显示状态。
     */
    const UiGridViewRepresentativeSnapshot &GetRepresentativeSnapshot() const;

    /**
     * @comment 滚动到指定 item，并重建可见窗口。
     * @param item_index 目标 item 索引。
     * @return 成功时返回 Success，否则返回明确状态。
     */
    UiGridViewRepresentativeStatus ScrollToItem(std::uint32_t item_index);

    /**
     * @comment 滚动到指定 group，并重建可见窗口。
     * @param group_index 目标 group 索引。
     * @return 成功时返回 Success，否则返回明确状态。
     */
    UiGridViewRepresentativeStatus ScrollToGroup(std::uint32_t group_index);

    /**
     * @comment 选择指定 item，并刷新可见 cell 状态。
     * @param item_index 目标 item 索引。
     * @return 成功时返回 Success，否则返回明确状态。
     */
    UiGridViewRepresentativeStatus SelectItem(std::uint32_t item_index);

    /**
     * @comment 刷新指定 item，并标记可见窗口中的受影响 cell。
     * @param item_index 目标 item 索引。
     * @return 成功时返回 Success，否则返回明确状态。
     */
    UiGridViewRepresentativeStatus RefreshItem(std::uint32_t item_index);

    /**
     * @comment 清空代表 GridView 数据，并保留 controller 生命周期。
     * @return 成功时返回 Success。
     */
    UiGridViewRepresentativeStatus ClearGrid();

protected:
    BaseUiLifecycleStatus OnInitEvent() override;
    BaseUiLifecycleStatus OnBindEvent() override;
    BaseUiLifecycleStatus OnOpenEvent() override;
    BaseUiLifecycleStatus OnOpenWithArgsEvent(const UiPanelOpenArgs &open_args) override;
    BaseUiLifecycleStatus OnCloseEvent() override;
    BaseUiLifecycleStatus OnClearEvent() override;

private:
    UiGridViewRepresentativeStatus RebuildGrid(std::uint32_t scroll_to_index);
    UiGridViewRepresentativeStatus SetLastStatus(UiGridViewRepresentativeStatus status);

    UiGridViewRepresentativeSnapshot representative_snapshot_;
};
}
