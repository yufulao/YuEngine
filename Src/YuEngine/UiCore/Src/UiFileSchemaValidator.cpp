// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiFileSchemaValidator.cpp

#include "YuEngine/UiCore/UiFileSchemaValidator.h"

#include <cstddef>

namespace yuengine::uicore {
namespace {
bool IsStorageValid(const void *data, std::size_t count) {
    if (count == 0U) {
        return true;
    }

    return data != nullptr;
}

bool IsIssueStorageValid(
    std::span<UiFileSchemaIssueRecord> out_issues,
    std::uint32_t issue_count) {
    if (issue_count == 0U) {
        return true;
    }

    if (out_issues.size() < static_cast<std::size_t>(issue_count)) {
        return false;
    }

    return out_issues.data() != nullptr;
}
}

UiFileSchemaStatus UiFileSchemaValidator::Validate(
    const UiFileSchemaDesc &desc,
    std::span<UiFileSchemaIssueRecord> out_issues,
    UiFileSchemaValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiFileSchemaStatus::InvalidInput;
    }

    *out_result = UiFileSchemaValidationResult{};
    out_result->checked_node_count = static_cast<std::uint32_t>(desc.nodes.size());
    out_result->checked_layout_count = static_cast<std::uint32_t>(desc.layouts.size());
    out_result->checked_style_ref_count = static_cast<std::uint32_t>(desc.style_refs.size());
    out_result->checked_resource_ref_count = static_cast<std::uint32_t>(desc.resource_refs.size());
    out_result->checked_event_binding_count = static_cast<std::uint32_t>(desc.event_bindings.size());

    const UiFileSchemaStatus header_status = ValidateHeader(desc.header, out_result);
    if (header_status != UiFileSchemaStatus::Success) {
        out_result->status = header_status;
        return header_status;
    }

    if (!IsStorageValid(desc.nodes.data(), desc.nodes.size()) ||
        !IsStorageValid(desc.layouts.data(), desc.layouts.size()) ||
        !IsStorageValid(desc.style_refs.data(), desc.style_refs.size()) ||
        !IsStorageValid(desc.resource_refs.data(), desc.resource_refs.size()) ||
        !IsStorageValid(desc.event_bindings.data(), desc.event_bindings.size())) {
        out_result->status = UiFileSchemaStatus::InvalidInput;
        return UiFileSchemaStatus::InvalidInput;
    }

    for (std::size_t index = 0U; index < desc.nodes.size(); ++index) {
        const UiFileNodeRecord &record = desc.nodes[index];
        const UiFileSchemaStatus record_status =
            ValidateNodeRecord(record, static_cast<std::uint32_t>(index), out_result);
        if (record_status != UiFileSchemaStatus::Success) {
            out_result->status = record_status;
            return record_status;
        }
    }

    for (std::size_t index = 0U; index < desc.layouts.size(); ++index) {
        const UiFileLayoutRecord &record = desc.layouts[index];
        const UiFileSchemaStatus record_status =
            ValidateLayoutRecord(record, static_cast<std::uint32_t>(index), out_result);
        if (record_status != UiFileSchemaStatus::Success) {
            out_result->status = record_status;
            return record_status;
        }
    }

    for (std::size_t index = 0U; index < desc.style_refs.size(); ++index) {
        const UiFileStyleRef &record = desc.style_refs[index];
        const UiFileSchemaStatus record_status =
            ValidateStyleRef(record, static_cast<std::uint32_t>(index), out_result);
        if (record_status != UiFileSchemaStatus::Success) {
            out_result->status = record_status;
            return record_status;
        }
    }

    for (std::size_t index = 0U; index < desc.resource_refs.size(); ++index) {
        const UiFileResourceRef &record = desc.resource_refs[index];
        const UiFileSchemaStatus record_status =
            ValidateResourceRef(record, static_cast<std::uint32_t>(index), out_result);
        if (record_status != UiFileSchemaStatus::Success) {
            out_result->status = record_status;
            return record_status;
        }
    }

    for (std::size_t index = 0U; index < desc.event_bindings.size(); ++index) {
        const UiFileEventBinding &record = desc.event_bindings[index];
        const UiFileSchemaStatus record_status =
            ValidateEventBinding(record, static_cast<std::uint32_t>(index), out_result);
        if (record_status != UiFileSchemaStatus::Success) {
            out_result->status = record_status;
            return record_status;
        }
    }

    std::uint32_t issue_count = 0U;
    if (!ContainsNodeId(desc.nodes, desc.header.root_node_id)) {
        ++issue_count;
    }

    for (std::size_t index = 0U; index < desc.nodes.size(); ++index) {
        const std::uint32_t node_index = static_cast<std::uint32_t>(index);
        if (!IsFirstNodeIdOccurrence(desc.nodes, node_index)) {
            continue;
        }

        const std::uint32_t node_count = CountNodeId(desc.nodes, desc.nodes[index].node_id);
        if (node_count > 1U) {
            ++issue_count;
        }
    }

    for (const UiFileNodeRecord &record : desc.nodes) {
        if (!record.parent_id.IsValid()) {
            continue;
        }

        if (!ContainsNodeId(desc.nodes, record.parent_id)) {
            ++issue_count;
        }
    }

    for (const UiFileLayoutRecord &record : desc.layouts) {
        if (!ContainsNodeId(desc.nodes, record.container.container_id)) {
            ++issue_count;
        }
    }

    for (const UiFileStyleRef &record : desc.style_refs) {
        if (!ContainsNodeId(desc.nodes, record.node_id)) {
            ++issue_count;
        }
    }

    for (const UiFileResourceRef &record : desc.resource_refs) {
        if (!ContainsNodeId(desc.nodes, record.node_id)) {
            ++issue_count;
        }
    }

    for (const UiFileEventBinding &record : desc.event_bindings) {
        if (!ContainsNodeId(desc.nodes, record.node_id)) {
            ++issue_count;
        }

        if (record.binding_key == 0U || record.event_key == 0U) {
            ++issue_count;
        }
    }

    out_result->issue_count = issue_count;
    if (!IsIssueStorageValid(out_issues, issue_count)) {
        out_result->status = UiFileSchemaStatus::OutputCapacityExceeded;
        return UiFileSchemaStatus::OutputCapacityExceeded;
    }

    std::uint32_t issue_index = 0U;
    if (!ContainsNodeId(desc.nodes, desc.header.root_node_id)) {
        out_issues[issue_index].issue_kind = UiFileSchemaIssueKind::MissingRootNode;
        out_issues[issue_index].node_id = desc.header.root_node_id;
        out_issues[issue_index].context_key = desc.header.layout_id;
        ++issue_index;
    }

    for (std::size_t index = 0U; index < desc.nodes.size(); ++index) {
        const std::uint32_t node_index = static_cast<std::uint32_t>(index);
        if (!IsFirstNodeIdOccurrence(desc.nodes, node_index)) {
            continue;
        }

        const UiNodeId node_id = desc.nodes[index].node_id;
        const std::uint32_t node_count = CountNodeId(desc.nodes, node_id);
        if (node_count <= 1U) {
            continue;
        }

        out_issues[issue_index].issue_kind = UiFileSchemaIssueKind::DuplicateNodeId;
        out_issues[issue_index].node_id = node_id;
        out_issues[issue_index].context_key = node_id.value;
        out_issues[issue_index].record_index = node_index;
        out_issues[issue_index].duplicate_count = node_count;
        ++issue_index;
    }

    for (std::size_t index = 0U; index < desc.nodes.size(); ++index) {
        const UiFileNodeRecord &record = desc.nodes[index];
        if (!record.parent_id.IsValid()) {
            continue;
        }

        if (ContainsNodeId(desc.nodes, record.parent_id)) {
            continue;
        }

        out_issues[issue_index].issue_kind = UiFileSchemaIssueKind::MissingParentNode;
        out_issues[issue_index].node_id = record.node_id;
        out_issues[issue_index].context_key = record.parent_id.value;
        out_issues[issue_index].record_index = static_cast<std::uint32_t>(index);
        ++issue_index;
    }

    for (std::size_t index = 0U; index < desc.layouts.size(); ++index) {
        const UiFileLayoutRecord &record = desc.layouts[index];
        if (ContainsNodeId(desc.nodes, record.container.container_id)) {
            continue;
        }

        out_issues[issue_index].issue_kind = UiFileSchemaIssueKind::MissingLayoutContainerNode;
        out_issues[issue_index].node_id = record.container.container_id;
        out_issues[issue_index].context_key = record.container.container_id.value;
        out_issues[issue_index].record_index = static_cast<std::uint32_t>(index);
        ++issue_index;
    }

    for (std::size_t index = 0U; index < desc.style_refs.size(); ++index) {
        const UiFileStyleRef &record = desc.style_refs[index];
        if (ContainsNodeId(desc.nodes, record.node_id)) {
            continue;
        }

        out_issues[issue_index].issue_kind = UiFileSchemaIssueKind::MissingStyleRefNode;
        out_issues[issue_index].node_id = record.node_id;
        out_issues[issue_index].context_key = record.style_key;
        out_issues[issue_index].record_index = static_cast<std::uint32_t>(index);
        ++issue_index;
    }

    for (std::size_t index = 0U; index < desc.resource_refs.size(); ++index) {
        const UiFileResourceRef &record = desc.resource_refs[index];
        if (ContainsNodeId(desc.nodes, record.node_id)) {
            continue;
        }

        out_issues[issue_index].issue_kind = UiFileSchemaIssueKind::MissingResourceRefNode;
        out_issues[issue_index].node_id = record.node_id;
        out_issues[issue_index].context_key = record.resource_key;
        out_issues[issue_index].record_index = static_cast<std::uint32_t>(index);
        ++issue_index;
    }

    for (std::size_t index = 0U; index < desc.event_bindings.size(); ++index) {
        const UiFileEventBinding &record = desc.event_bindings[index];
        if (!ContainsNodeId(desc.nodes, record.node_id)) {
            out_issues[issue_index].issue_kind = UiFileSchemaIssueKind::MissingEventBindingNode;
            out_issues[issue_index].node_id = record.node_id;
            out_issues[issue_index].context_key = record.binding_key;
            out_issues[issue_index].record_index = static_cast<std::uint32_t>(index);
            ++issue_index;
        }

        if (record.binding_key != 0U && record.event_key != 0U) {
            continue;
        }

        out_issues[issue_index].issue_kind = UiFileSchemaIssueKind::MissingEventBindingKey;
        out_issues[issue_index].node_id = record.node_id;
        out_issues[issue_index].context_key = record.binding_key;
        out_issues[issue_index].record_index = static_cast<std::uint32_t>(index);
        ++issue_index;
    }

    if (issue_count > 0U) {
        out_result->status = UiFileSchemaStatus::IssuesFound;
        return UiFileSchemaStatus::IssuesFound;
    }

    out_result->status = UiFileSchemaStatus::Success;
    return UiFileSchemaStatus::Success;
}

UiFileSchemaStatus UiFileSchemaValidator::ValidateHeader(
    const UiFileSchemaHeader &header,
    UiFileSchemaValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiFileSchemaStatus::InvalidInput;
    }

    if (header.schema_id != UI_FILE_SCHEMA_ID) {
        return UiFileSchemaStatus::InvalidHeader;
    }

    if (header.schema_version != UI_FILE_SCHEMA_VERSION) {
        return UiFileSchemaStatus::InvalidHeader;
    }

    if (header.layout_id == 0U) {
        return UiFileSchemaStatus::InvalidHeader;
    }

    if (!header.root_node_id.IsValid()) {
        out_result->failed_node_id = header.root_node_id;
        return UiFileSchemaStatus::InvalidHeader;
    }

    return UiFileSchemaStatus::Success;
}

UiFileSchemaStatus UiFileSchemaValidator::ValidateNodeRecord(
    const UiFileNodeRecord &record,
    std::uint32_t record_index,
    UiFileSchemaValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiFileSchemaStatus::InvalidInput;
    }

    out_result->failed_record_index = record_index;
    out_result->failed_node_id = record.node_id;
    if (!record.node_id.IsValid()) {
        return UiFileSchemaStatus::InvalidNodeRecord;
    }

    if (record.rect_transform.dpi_scale <= 0.0F) {
        return UiFileSchemaStatus::InvalidNodeRecord;
    }

    return UiFileSchemaStatus::Success;
}

UiFileSchemaStatus UiFileSchemaValidator::ValidateLayoutRecord(
    const UiFileLayoutRecord &record,
    std::uint32_t record_index,
    UiFileSchemaValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiFileSchemaStatus::InvalidInput;
    }

    out_result->failed_record_index = record_index;
    out_result->failed_node_id = record.container.container_id;
    if (!record.container.container_id.IsValid()) {
        return UiFileSchemaStatus::InvalidLayoutRecord;
    }

    if (record.container.grid_column_count == 0U) {
        return UiFileSchemaStatus::InvalidLayoutRecord;
    }

    return UiFileSchemaStatus::Success;
}

UiFileSchemaStatus UiFileSchemaValidator::ValidateStyleRef(
    const UiFileStyleRef &record,
    std::uint32_t record_index,
    UiFileSchemaValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiFileSchemaStatus::InvalidInput;
    }

    out_result->failed_record_index = record_index;
    out_result->failed_node_id = record.node_id;
    if (!record.node_id.IsValid()) {
        return UiFileSchemaStatus::InvalidStyleRef;
    }

    if (record.style_key == 0U) {
        return UiFileSchemaStatus::InvalidStyleRef;
    }

    return UiFileSchemaStatus::Success;
}

UiFileSchemaStatus UiFileSchemaValidator::ValidateResourceRef(
    const UiFileResourceRef &record,
    std::uint32_t record_index,
    UiFileSchemaValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiFileSchemaStatus::InvalidInput;
    }

    out_result->failed_record_index = record_index;
    out_result->failed_node_id = record.node_id;
    if (!record.node_id.IsValid()) {
        return UiFileSchemaStatus::InvalidResourceRef;
    }

    if (!IsKnownResourceKind(record.kind)) {
        return UiFileSchemaStatus::InvalidResourceRef;
    }

    if (record.resource_key == 0U) {
        return UiFileSchemaStatus::InvalidResourceRef;
    }

    return UiFileSchemaStatus::Success;
}

UiFileSchemaStatus UiFileSchemaValidator::ValidateEventBinding(
    const UiFileEventBinding &record,
    std::uint32_t record_index,
    UiFileSchemaValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiFileSchemaStatus::InvalidInput;
    }

    out_result->failed_record_index = record_index;
    out_result->failed_node_id = record.node_id;
    if (!record.node_id.IsValid()) {
        return UiFileSchemaStatus::InvalidEventBinding;
    }

    return UiFileSchemaStatus::Success;
}

bool UiFileSchemaValidator::ContainsNodeId(
    std::span<const UiFileNodeRecord> nodes,
    UiNodeId node_id) const {
    if (!node_id.IsValid()) {
        return false;
    }

    for (const UiFileNodeRecord &record : nodes) {
        if (record.node_id.value == node_id.value) {
            return true;
        }
    }

    return false;
}

std::uint32_t UiFileSchemaValidator::CountNodeId(
    std::span<const UiFileNodeRecord> nodes,
    UiNodeId node_id) const {
    std::uint32_t count = 0U;
    if (!node_id.IsValid()) {
        return count;
    }

    for (const UiFileNodeRecord &record : nodes) {
        if (record.node_id.value == node_id.value) {
            ++count;
        }
    }

    return count;
}

bool UiFileSchemaValidator::IsFirstNodeIdOccurrence(
    std::span<const UiFileNodeRecord> nodes,
    std::uint32_t node_index) const {
    if (static_cast<std::size_t>(node_index) >= nodes.size()) {
        return false;
    }

    const UiNodeId node_id = nodes[static_cast<std::size_t>(node_index)].node_id;
    for (std::uint32_t index = 0U; index < node_index; ++index) {
        if (nodes[static_cast<std::size_t>(index)].node_id.value == node_id.value) {
            return false;
        }
    }

    return true;
}

bool UiFileSchemaValidator::IsKnownResourceKind(UiFileResourceKind kind) const {
    if (kind == UiFileResourceKind::Sprite) {
        return true;
    }

    if (kind == UiFileResourceKind::Font) {
        return true;
    }

    if (kind == UiFileResourceKind::Localization) {
        return true;
    }

    if (kind == UiFileResourceKind::Audio) {
        return true;
    }

    return kind == UiFileResourceKind::Custom;
}
}
