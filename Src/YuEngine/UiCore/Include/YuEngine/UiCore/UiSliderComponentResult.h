// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiSliderComponentResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiSliderAdjustmentSource.h"
#include "YuEngine/UiCore/UiSliderComponentStatus.h"
#include "YuEngine/UiCore/UiSliderVisualUpdate.h"

namespace yuengine::uicore {
struct UiSliderComponentResult final {
    UiSliderComponentStatus status = UiSliderComponentStatus::InvalidOutputBuffer;
    UiNodeId node_id;
    UiNodeId captured_node_id;
    UiSliderAdjustmentSource adjustment_source = UiSliderAdjustmentSource::None;
    UiSliderVisualUpdate visual_update;
    UiDirtyChangeType dirty_change_type = UiDirtyChangeType::PaintOnly;
    float normalized_value = 0.0F;
    float resolved_value = 0.0F;
    float previous_value = 0.0F;
    std::uint32_t value_changed_event_key = 0U;
    bool value_changed = false;
    bool pointer_capture_active = false;
    bool keyboard_adjustment_path = false;
    bool gamepad_adjustment_path = false;

    /**
     * @comment 检查 Slider component 输出是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiSliderComponentStatus::Success;
    }
};
}
