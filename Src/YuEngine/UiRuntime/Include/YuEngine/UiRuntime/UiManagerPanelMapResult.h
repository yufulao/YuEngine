// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPanelMapResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerPanelMapRecord.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"

namespace yuengine::uiruntime {
struct UiManagerPanelMapResult final {
    UiManagerPanelMapStatus status = UiManagerPanelMapStatus::InvalidOutputBuffer;
    UiManagerPanelMapRecord record;
    std::uint32_t loaded_panel_count = 0U;
    std::uint32_t active_panel_count = 0U;
    bool reused_loaded = false;
    bool already_active = false;
    bool already_inactive = false;

    /**
     * @comment 检查 panel map 操作是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiManagerPanelMapStatus::Success;
    }
};
}
