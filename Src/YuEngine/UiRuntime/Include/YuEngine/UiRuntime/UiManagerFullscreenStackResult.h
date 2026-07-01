// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerFullscreenStackResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerFullscreenStackConstants.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackStatus.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapRecord.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"

namespace yuengine::uiruntime {
enum class UiManagerFullscreenStackOperation {
    None,
    Open
};

struct UiManagerFullscreenStackResult final {
    UiManagerFullscreenStackStatus status = UiManagerFullscreenStackStatus::InvalidOutputBuffer;
    UiManagerPanelMapStatus panel_map_status = UiManagerPanelMapStatus::Success;
    UiManagerPanelMapRecord record;
    UiPanelId panel_id;
    UiPanelId closed_panel_id;
    UiPanelId restored_panel_id;
    UiPanelId top_panel_id;
    UiPanelId failed_panel_id;
    UiPanelId failed_previous_top_panel_id;
    std::uint32_t fullscreen_count = 0U;
    std::uint32_t required_fullscreen_order_count = 0U;
    std::uint32_t failed_fullscreen_order_index = MAX_UI_MANAGER_FULLSCREEN_STACK_COUNT;
    UiManagerFullscreenStackOperation failed_operation = UiManagerFullscreenStackOperation::None;
    bool pushed = false;
    bool moved_to_top = false;
    bool navigated_back = false;
    bool restored_previous = false;
    bool already_top = false;
    bool already_in_stack = false;
    bool already_inactive = false;
    bool removed_from_stack = false;
    bool closed_current = false;
    bool closed_middle = false;

    /**
     * @comment 检查 fullscreen stack 操作是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiManagerFullscreenStackStatus::Success;
    }
};
}
