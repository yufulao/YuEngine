// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Include/YuEngine/UiEditor/UiEditorStatePreview.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiButtonState.h"
#include "YuEngine/UiEditor/UiEditorComponentTemplate.h"
#include "YuEngine/UiEditor/UiEditorShellState.h"

namespace yuengine::uieditor {
constexpr std::uint32_t UI_EDITOR_STATE_PREVIEW_NAME_CAPACITY = 32U;

enum class UiEditorStatePreviewStatus {
    Success = 0,
    InvalidInput,
    InvalidTemplate,
    InvalidState,
    InvalidOutput
};

struct UiEditorStatePreviewDesc final {
    UiEditorComponentTemplateRecord component_template;
    yuengine::uicore::UiButtonState button_state = yuengine::uicore::UiButtonState::Normal;
};

struct UiEditorStatePreviewRecord final {
    UiEditorComponentTemplateKind component_kind = UiEditorComponentTemplateKind::Invalid;
    yuengine::uicore::UiButtonState button_state = yuengine::uicore::UiButtonState::Normal;
    std::uint32_t node_id = 0U;
    char state_name[UI_EDITOR_STATE_PREVIEW_NAME_CAPACITY] = {};
    bool state_badge_visible = false;
    bool disabled_overlay_visible = false;
    bool selected_outline_visible = false;
    bool interaction_enabled = false;
};

struct UiEditorStatePreviewResult final {
    UiEditorStatePreviewStatus status = UiEditorStatePreviewStatus::Success;
    yuengine::uicore::UiButtonState button_state = yuengine::uicore::UiButtonState::Normal;
    std::uint32_t node_id = 0U;

    /**
     * @comment 检查 state preview 是否成功生成可见状态记录。
     * @return 成功生成记录时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiEditorStatePreviewStatus::Success;
    }
};

class UiEditorStatePreviewFactory final {
public:
    /**
     * @comment 创建 editor-only Button state preview 记录。
     * @param desc 输入 state preview 描述。
     * @param out_record 输出可见状态记录。
     * @param out_result 输出显式状态。
     * @return 显式创建状态。
     */
    UiEditorStatePreviewStatus CreateButtonStatePreview(
        const UiEditorStatePreviewDesc &desc,
        UiEditorStatePreviewRecord *out_record,
        UiEditorStatePreviewResult *out_result) const;

private:
    UiEditorStatePreviewStatus ValidateDesc(
        const UiEditorStatePreviewDesc &desc,
        UiEditorStatePreviewResult *out_result) const;
    bool WriteRecord(
        const UiEditorStatePreviewDesc &desc,
        UiEditorStatePreviewRecord *out_record) const;
};
}
