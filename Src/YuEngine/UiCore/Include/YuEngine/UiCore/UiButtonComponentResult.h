// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiButtonComponentResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiButtonActivationSource.h"
#include "YuEngine/UiCore/UiButtonComponentStatus.h"
#include "YuEngine/UiCore/UiButtonState.h"
#include "YuEngine/UiCore/UiButtonVisualUpdate.h"
#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiButtonComponentResult final {
    UiButtonComponentStatus status = UiButtonComponentStatus::InvalidOutputBuffer;
    UiNodeId node_id;
    UiButtonState state = UiButtonState::Normal;
    UiButtonActivationSource activation_source = UiButtonActivationSource::None;
    UiButtonVisualUpdate visual_update;
    UiDirtyChangeType dirty_change_type = UiDirtyChangeType::HoverState;
    std::uint32_t activation_event_key = 0U;
    std::uint32_t activation_sound_key = 0U;
    bool activated = false;

    /**
     * @comment 检查 Button component 输出是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiButtonComponentStatus::Success;
    }
};
}
