// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiFileSchemaValidator.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiLayoutContainerDesc.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRectTransform.h"

namespace yuengine::uicore {
constexpr std::uint32_t UI_FILE_SCHEMA_ID = 0x59554653U;
constexpr std::uint32_t UI_FILE_SCHEMA_VERSION = 1U;

enum class UiFileResourceKind {
    Invalid = 0,
    Sprite,
    Font,
    Localization,
    Audio,
    Custom
};

enum class UiFileSchemaStatus {
    Success = 0,
    IssuesFound,
    InvalidInput,
    InvalidHeader,
    InvalidNodeRecord,
    InvalidLayoutRecord,
    InvalidStyleRef,
    InvalidResourceRef,
    InvalidEventBinding,
    OutputCapacityExceeded
};

enum class UiFileSchemaIssueKind {
    None = 0,
    MissingRootNode,
    DuplicateNodeId,
    MissingParentNode,
    MissingLayoutContainerNode,
    MissingStyleRefNode,
    MissingResourceRefNode,
    MissingEventBindingNode,
    MissingEventBindingKey
};

struct UiFileSchemaHeader final {
    std::uint32_t schema_id = UI_FILE_SCHEMA_ID;
    std::uint32_t schema_version = UI_FILE_SCHEMA_VERSION;
    std::uint32_t layout_id = 0U;
    UiNodeId root_node_id;
};

struct UiFileNodeRecord final {
    UiNodeId node_id;
    UiNodeId parent_id;
    UiRectTransform rect_transform;
    std::uint32_t sibling_order = 0U;
    std::int32_t layer = 0;
    bool is_visible = true;
    bool is_enabled = true;
    bool is_hit_testable = true;
};

struct UiFileLayoutRecord final {
    UiLayoutContainerDesc container;
};

struct UiFileStyleRef final {
    UiNodeId node_id;
    std::uint32_t style_key = 0U;
};

struct UiFileResourceRef final {
    UiNodeId node_id;
    UiFileResourceKind kind = UiFileResourceKind::Invalid;
    std::uint32_t resource_key = 0U;
};

struct UiFileEventBinding final {
    UiNodeId node_id;
    std::uint32_t binding_key = 0U;
    std::uint32_t event_key = 0U;
};

struct UiFileSchemaDesc final {
    UiFileSchemaHeader header;
    std::span<const UiFileNodeRecord> nodes{};
    std::span<const UiFileLayoutRecord> layouts{};
    std::span<const UiFileStyleRef> style_refs{};
    std::span<const UiFileResourceRef> resource_refs{};
    std::span<const UiFileEventBinding> event_bindings{};
};

struct UiFileSchemaIssueRecord final {
    UiFileSchemaIssueKind issue_kind = UiFileSchemaIssueKind::None;
    UiNodeId node_id;
    std::uint32_t context_key = 0U;
    std::uint32_t record_index = 0U;
    std::uint32_t duplicate_count = 0U;
};

struct UiFileSchemaValidationResult final {
    UiFileSchemaStatus status = UiFileSchemaStatus::Success;
    std::uint32_t checked_node_count = 0U;
    std::uint32_t checked_layout_count = 0U;
    std::uint32_t checked_style_ref_count = 0U;
    std::uint32_t checked_resource_ref_count = 0U;
    std::uint32_t checked_event_binding_count = 0U;
    std::uint32_t issue_count = 0U;
    std::uint32_t required_issue_count = 0U;
    std::uint32_t capacity_entry_issue_capacity = 0U;
    std::uint32_t capacity_entry_current_issue_count = 0U;
    std::uint32_t capacity_entry_required_issue_count = 0U;
    UiFileSchemaIssueKind failed_issue_kind = UiFileSchemaIssueKind::None;
    std::uint32_t failed_record_index = 0U;
    UiNodeId failed_node_id;
    std::uint32_t failed_context_key = 0U;
    std::uint32_t failed_duplicate_count = 0U;

    /**
     * @comment 检查 UI file schema validation 是否没有发现问题。
     * @return 校验成功且没有 issue 时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiFileSchemaStatus::Success;
    }
};

class UiFileSchemaValidator final {
public:
    /**
     * @comment 校验通用 UI file schema value records。
     * @param desc 输入 UI file schema 描述。
     * @param out_issues 调用方持有的 issue report buffer。
     * @param out_result 输出 validation result。
     * @return 显式校验状态。
     */
    UiFileSchemaStatus Validate(
        const UiFileSchemaDesc &desc,
        std::span<UiFileSchemaIssueRecord> out_issues,
        UiFileSchemaValidationResult *out_result) const;

private:
    UiFileSchemaStatus ValidateHeader(
        const UiFileSchemaHeader &header,
        UiFileSchemaValidationResult *out_result) const;
    UiFileSchemaStatus ValidateNodeRecord(
        const UiFileNodeRecord &record,
        std::uint32_t record_index,
        UiFileSchemaValidationResult *out_result) const;
    UiFileSchemaStatus ValidateLayoutRecord(
        const UiFileLayoutRecord &record,
        std::uint32_t record_index,
        UiFileSchemaValidationResult *out_result) const;
    UiFileSchemaStatus ValidateStyleRef(
        const UiFileStyleRef &record,
        std::uint32_t record_index,
        UiFileSchemaValidationResult *out_result) const;
    UiFileSchemaStatus ValidateResourceRef(
        const UiFileResourceRef &record,
        std::uint32_t record_index,
        UiFileSchemaValidationResult *out_result) const;
    UiFileSchemaStatus ValidateEventBinding(
        const UiFileEventBinding &record,
        std::uint32_t record_index,
        UiFileSchemaValidationResult *out_result) const;
    bool ContainsNodeId(std::span<const UiFileNodeRecord> nodes, UiNodeId node_id) const;
    std::uint32_t CountNodeId(std::span<const UiFileNodeRecord> nodes, UiNodeId node_id) const;
    bool IsFirstNodeIdOccurrence(std::span<const UiFileNodeRecord> nodes, std::uint32_t node_index) const;
    bool IsKnownResourceKind(UiFileResourceKind kind) const;
    void CaptureFirstUnreportedIssue(
        const UiFileSchemaDesc &desc,
        std::uint32_t failed_issue_index,
        std::uint32_t required_issue_count,
        UiFileSchemaValidationResult *out_result) const;
    bool TryCaptureCapacityIssue(
        const UiFileSchemaIssueRecord &issue,
        std::uint32_t failed_issue_index,
        std::uint32_t required_issue_count,
        std::uint32_t *issue_index,
        UiFileSchemaValidationResult *out_result) const;
    void RecordCapacityIssue(
        const UiFileSchemaIssueRecord &issue,
        std::uint32_t issue_capacity,
        std::uint32_t current_issue_count,
        std::uint32_t required_issue_count,
        UiFileSchemaValidationResult *out_result) const;
};
}
