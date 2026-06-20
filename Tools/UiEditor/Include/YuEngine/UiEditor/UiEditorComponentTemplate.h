// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Include/YuEngine/UiEditor/UiEditorComponentTemplate.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiEditor/UiEditorIdEventValidator.h"
#include "YuEngine/UiEditor/UiEditorResourceReferenceValidator.h"
#include "YuEngine/UiEditor/UiEditorShellState.h"

namespace yuengine::uieditor {
enum class UiEditorComponentTemplateKind {
    Invalid = 0,
    Text,
    Image,
    Button,
    Slider,
    GridView
};

enum class UiEditorComponentTemplateDefaultState {
    Invalid = 0,
    TextReady,
    ImageReady,
    ButtonNormal,
    SliderIdle,
    GridViewReady
};

enum class UiEditorComponentTemplateStatus {
    Success = 0,
    InvalidInput,
    InvalidTemplateKind,
    InvalidLayoutNode,
    InvalidOutput,
    OutputCapacityExceeded
};

struct UiEditorComponentTemplateDesc final {
    UiEditorComponentTemplateKind kind = UiEditorComponentTemplateKind::Invalid;
    std::uint32_t node_id = 0U;
    std::uint32_t parent_node_id = 0U;
    std::uint32_t order = 0U;
    std::uint32_t primary_resource_key = 0U;
    std::uint32_t secondary_resource_key = 0U;
    std::uint32_t event_id = 0U;
};

struct UiEditorComponentTemplateRecord final {
    UiEditorComponentTemplateKind kind = UiEditorComponentTemplateKind::Invalid;
    UiEditorComponentTemplateDefaultState default_state = UiEditorComponentTemplateDefaultState::Invalid;
    UiEditorLayoutNodeRecord layout_node{};
};

struct UiEditorComponentTemplateResult final {
    UiEditorComponentTemplateStatus status = UiEditorComponentTemplateStatus::Success;
    UiEditorComponentTemplateKind kind = UiEditorComponentTemplateKind::Invalid;
    UiEditorComponentTemplateDefaultState default_state = UiEditorComponentTemplateDefaultState::Invalid;
    std::uint32_t required_resource_count = 0U;
    std::uint32_t written_resource_count = 0U;
    std::uint32_t required_event_count = 0U;
    std::uint32_t written_event_count = 0U;
    std::uint32_t failed_node_id = 0U;

    /**
     * @comment 检查 component template 是否成功创建完整输出。
     * @return 成功创建 layout node、resource references 和 event bindings 时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiEditorComponentTemplateStatus::Success;
    }
};

class UiEditorComponentTemplateFactory final {
public:
    /**
     * @comment 创建 editor-only component template 输出。
     * @param desc 输入 template 描述。
     * @param out_record 输出 layout node 与 default state。
     * @param out_resources 调用方持有的 resource reference buffer。
     * @param out_events 调用方持有的 event binding buffer。
     * @param out_result 输出 template 创建结果。
     * @return 显式创建状态。
     */
    UiEditorComponentTemplateStatus Create(
        const UiEditorComponentTemplateDesc &desc,
        UiEditorComponentTemplateRecord *out_record,
        std::span<UiEditorResourceReference> out_resources,
        std::span<UiEditorEventBinding> out_events,
        UiEditorComponentTemplateResult *out_result) const;

private:
    UiEditorComponentTemplateStatus ValidateDesc(
        const UiEditorComponentTemplateDesc &desc,
        UiEditorComponentTemplateResult *out_result) const;
    UiEditorComponentTemplateStatus ValidateOutput(
        std::span<UiEditorResourceReference> out_resources,
        std::span<UiEditorEventBinding> out_events,
        const UiEditorComponentTemplateResult &result) const;
    bool WriteRecord(
        const UiEditorComponentTemplateDesc &desc,
        UiEditorComponentTemplateRecord *out_record) const;
    void WriteResources(
        const UiEditorComponentTemplateDesc &desc,
        std::span<UiEditorResourceReference> out_resources,
        UiEditorComponentTemplateResult *out_result) const;
    void WriteEvents(
        const UiEditorComponentTemplateDesc &desc,
        std::span<UiEditorEventBinding> out_events,
        UiEditorComponentTemplateResult *out_result) const;
    UiEditorComponentTemplateResult MakeResult(const UiEditorComponentTemplateDesc &desc) const;
};
}
