// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPopupStackResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerPanelMapRecord.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"

namespace yuengine::uiruntime {
struct UiManagerPopupStackResult final {
    UiManagerPopupStackStatus status = UiManagerPopupStackStatus::InvalidOutputBuffer;
    UiManagerPanelMapStatus panel_map_status = UiManagerPanelMapStatus::Success;
    UiManagerPanelMapRecord record;
    UiPanelId panel_id;
    UiPanelId top_panel_id;
    std::uint32_t popup_count = 0U;
    std::uint32_t required_popup_order_count = 0U;
    bool pushed = false;
    bool brought_to_top = false;
    bool already_top = false;
    bool already_in_stack = false;
    bool already_inactive = false;
    bool removed_from_stack = false;

    /**
     * @comment 检查 popup stack 操作是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiManagerPopupStackStatus::Success;
    }
};
}
