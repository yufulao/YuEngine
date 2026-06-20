// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiProjectRuntimeSmokeSampleResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerFullscreenStackSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackSnapshot.h"
#include "YuEngine/UiRuntime/UiProjectRuntimeSmokeSampleStatus.h"

namespace yuengine::uiruntime {
struct UiProjectRuntimeSmokeSampleResult final {
    UiProjectRuntimeSmokeSampleStatus status = UiProjectRuntimeSmokeSampleStatus::InvalidOutputBuffer;
    bool passed = false;
    bool popup_opened = false;
    bool popup_displayed = false;
    bool popup_closed = false;
    bool popup_released = false;
    bool fullscreen_opened = false;
    bool fullscreen_top_displayed = false;
    bool fullscreen_back_restored = false;
    bool fullscreen_released = false;
    bool grid_opened = false;
    bool grid_displayed = false;
    bool grid_visible_data_read = false;
    bool grid_released = false;
    bool cleanup_passed = false;
    std::uint32_t smoke_step_count = 0U;
    std::uint32_t popup_open_display_count = 0U;
    std::uint32_t fullscreen_open_display_count = 0U;
    std::uint32_t grid_open_display_count = 0U;
    std::uint32_t grid_visible_item_count = 0U;
    std::uint32_t released_panel_count = 0U;
    UiManagerPanelMapSnapshot panel_map_snapshot;
    UiManagerPopupStackSnapshot popup_stack_snapshot;
    UiManagerFullscreenStackSnapshot fullscreen_stack_snapshot;

    /**
     * @comment 检查 Project UI smoke sample 是否通过。
     * @return 通过时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiProjectRuntimeSmokeSampleStatus::Success && passed;
    }
};
}
