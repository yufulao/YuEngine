// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Include/YuEngine/UiEditor/UiEditorIdEventValidator.h

#pragma once

#include <cstdint>
#include <span>
#include <string_view>

namespace yuengine::uieditor {
enum class UiEditorIdEventIssueKind {
    None = 0,
    DuplicateNodeId,
    MissingEventName
};

enum class UiEditorIdEventValidationStatus {
    Success = 0,
    IssuesFound,
    InvalidInput,
    InvalidNode,
    InvalidEvent,
    OutputCapacityExceeded
};

struct UiEditorIdValidationNode final {
    std::uint32_t node_id = 0U;
};

struct UiEditorEventBinding final {
    std::uint32_t node_id = 0U;
    std::uint32_t event_id = 0U;
    std::string_view event_name{};
};

struct UiEditorIdEventValidationRecord final {
    UiEditorIdEventIssueKind issue_kind = UiEditorIdEventIssueKind::None;
    std::uint32_t node_id = 0U;
    std::uint32_t context_id = 0U;
    std::uint32_t duplicate_count = 0U;
};

struct UiEditorIdEventValidationResult final {
    UiEditorIdEventValidationStatus status = UiEditorIdEventValidationStatus::Success;
    std::uint32_t checked_node_count = 0U;
    std::uint32_t checked_event_count = 0U;
    std::uint32_t duplicate_node_id_count = 0U;
    std::uint32_t missing_event_name_count = 0U;
    std::uint32_t report_count = 0U;
    std::uint32_t failed_node_index = 0U;
    std::uint32_t failed_event_index = 0U;
    std::uint32_t failed_node_id = 0U;

    /**
     * @comment 检查 ID/event validation 是否没有发现问题。
     * @return 没有 duplicate id 和 missing event name 时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiEditorIdEventValidationStatus::Success;
    }
};

class UiEditorIdEventValidator final {
public:
    /**
     * @comment 校验 editor layout node id 和 event binding name，并输出可报告问题。
     * @param nodes layout 中提取的 node id 列表。
     * @param events layout 中提取的 event binding 列表。
     * @param out_reports 调用方持有的 report buffer。
     * @param out_result 输出 validation result。
     * @return 显式校验状态。
     */
    UiEditorIdEventValidationStatus Validate(
        std::span<const UiEditorIdValidationNode> nodes,
        std::span<const UiEditorEventBinding> events,
        std::span<UiEditorIdEventValidationRecord> out_reports,
        UiEditorIdEventValidationResult *out_result) const;

private:
    UiEditorIdEventValidationStatus ValidateNode(
        const UiEditorIdValidationNode &node,
        std::uint32_t node_index,
        UiEditorIdEventValidationResult *out_result) const;
    UiEditorIdEventValidationStatus ValidateEvent(
        const UiEditorEventBinding &event_binding,
        std::uint32_t event_index,
        UiEditorIdEventValidationResult *out_result) const;
    std::uint32_t CountNodeId(
        std::span<const UiEditorIdValidationNode> nodes,
        std::uint32_t node_id) const;
    bool IsFirstNodeIdOccurrence(
        std::span<const UiEditorIdValidationNode> nodes,
        std::size_t node_index) const;
};
}
