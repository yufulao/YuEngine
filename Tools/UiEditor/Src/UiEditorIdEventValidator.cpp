// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Src/UiEditorIdEventValidator.cpp

#include "YuEngine/UiEditor/UiEditorIdEventValidator.h"

#include <cstddef>

namespace yuengine::uieditor {
namespace {
bool IsNodeStorageValid(std::span<const UiEditorIdValidationNode> nodes) {
    if (nodes.empty()) {
        return true;
    }

    return nodes.data() != nullptr;
}

bool IsEventStorageValid(std::span<const UiEditorEventBinding> events) {
    if (events.empty()) {
        return true;
    }

    return events.data() != nullptr;
}

bool IsReportStorageValid(
    std::span<UiEditorIdEventValidationRecord> out_reports,
    std::uint32_t report_count) {
    if (report_count == 0U) {
        return true;
    }

    if (out_reports.size() < static_cast<std::size_t>(report_count)) {
        return false;
    }

    return out_reports.data() != nullptr;
}
}

UiEditorIdEventValidationStatus UiEditorIdEventValidator::Validate(
    std::span<const UiEditorIdValidationNode> nodes,
    std::span<const UiEditorEventBinding> events,
    std::span<UiEditorIdEventValidationRecord> out_reports,
    UiEditorIdEventValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorIdEventValidationStatus::InvalidInput;
    }

    *out_result = UiEditorIdEventValidationResult{};
    out_result->checked_node_count = static_cast<std::uint32_t>(nodes.size());
    out_result->checked_event_count = static_cast<std::uint32_t>(events.size());

    if (!IsNodeStorageValid(nodes) || !IsEventStorageValid(events)) {
        out_result->status = UiEditorIdEventValidationStatus::InvalidInput;
        return UiEditorIdEventValidationStatus::InvalidInput;
    }

    for (std::size_t index = 0U; index < nodes.size(); ++index) {
        const UiEditorIdValidationNode &node = nodes[index];
        const UiEditorIdEventValidationStatus node_status =
            ValidateNode(node, static_cast<std::uint32_t>(index), out_result);
        if (node_status != UiEditorIdEventValidationStatus::Success) {
            out_result->status = node_status;
            return node_status;
        }
    }

    for (std::size_t index = 0U; index < events.size(); ++index) {
        const UiEditorEventBinding &event_binding = events[index];
        const UiEditorIdEventValidationStatus event_status =
            ValidateEvent(event_binding, static_cast<std::uint32_t>(index), out_result);
        if (event_status != UiEditorIdEventValidationStatus::Success) {
            out_result->status = event_status;
            return event_status;
        }
    }

    std::uint32_t duplicate_node_id_count = 0U;
    for (std::size_t index = 0U; index < nodes.size(); ++index) {
        if (!IsFirstNodeIdOccurrence(nodes, index)) {
            continue;
        }

        const std::uint32_t node_count = CountNodeId(nodes, nodes[index].node_id);
        if (node_count > 1U) {
            ++duplicate_node_id_count;
        }
    }

    std::uint32_t missing_event_name_count = 0U;
    for (const UiEditorEventBinding &event_binding : events) {
        if (event_binding.event_name.empty()) {
            ++missing_event_name_count;
        }
    }

    out_result->duplicate_node_id_count = duplicate_node_id_count;
    out_result->missing_event_name_count = missing_event_name_count;
    out_result->report_count = duplicate_node_id_count + missing_event_name_count;
    if (!IsReportStorageValid(out_reports, out_result->report_count)) {
        out_result->status = UiEditorIdEventValidationStatus::OutputCapacityExceeded;
        return UiEditorIdEventValidationStatus::OutputCapacityExceeded;
    }

    std::uint32_t report_index = 0U;
    for (std::size_t index = 0U; index < nodes.size(); ++index) {
        if (!IsFirstNodeIdOccurrence(nodes, index)) {
            continue;
        }

        const std::uint32_t node_id = nodes[index].node_id;
        const std::uint32_t node_count = CountNodeId(nodes, node_id);
        if (node_count <= 1U) {
            continue;
        }

        out_reports[report_index].issue_kind = UiEditorIdEventIssueKind::DuplicateNodeId;
        out_reports[report_index].node_id = node_id;
        out_reports[report_index].context_id = node_id;
        out_reports[report_index].duplicate_count = node_count;
        ++report_index;
    }

    for (const UiEditorEventBinding &event_binding : events) {
        if (!event_binding.event_name.empty()) {
            continue;
        }

        out_reports[report_index].issue_kind = UiEditorIdEventIssueKind::MissingEventName;
        out_reports[report_index].node_id = event_binding.node_id;
        out_reports[report_index].context_id = event_binding.event_id;
        ++report_index;
    }

    if (out_result->report_count > 0U) {
        out_result->status = UiEditorIdEventValidationStatus::IssuesFound;
        return UiEditorIdEventValidationStatus::IssuesFound;
    }

    out_result->status = UiEditorIdEventValidationStatus::Success;
    return UiEditorIdEventValidationStatus::Success;
}

UiEditorIdEventValidationStatus UiEditorIdEventValidator::ValidateNode(
    const UiEditorIdValidationNode &node,
    std::uint32_t node_index,
    UiEditorIdEventValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorIdEventValidationStatus::InvalidInput;
    }

    out_result->failed_node_index = node_index;
    out_result->failed_node_id = node.node_id;
    if (node.node_id == 0U) {
        return UiEditorIdEventValidationStatus::InvalidNode;
    }

    return UiEditorIdEventValidationStatus::Success;
}

UiEditorIdEventValidationStatus UiEditorIdEventValidator::ValidateEvent(
    const UiEditorEventBinding &event_binding,
    std::uint32_t event_index,
    UiEditorIdEventValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorIdEventValidationStatus::InvalidInput;
    }

    out_result->failed_event_index = event_index;
    out_result->failed_node_id = event_binding.node_id;
    if (event_binding.node_id == 0U) {
        return UiEditorIdEventValidationStatus::InvalidEvent;
    }

    if (!event_binding.event_name.empty() && event_binding.event_name.data() == nullptr) {
        return UiEditorIdEventValidationStatus::InvalidEvent;
    }

    return UiEditorIdEventValidationStatus::Success;
}

std::uint32_t UiEditorIdEventValidator::CountNodeId(
    std::span<const UiEditorIdValidationNode> nodes,
    std::uint32_t node_id) const {
    std::uint32_t count = 0U;
    for (const UiEditorIdValidationNode &node : nodes) {
        if (node.node_id == node_id) {
            ++count;
        }
    }

    return count;
}

bool UiEditorIdEventValidator::IsFirstNodeIdOccurrence(
    std::span<const UiEditorIdValidationNode> nodes,
    std::size_t node_index) const {
    if (node_index >= nodes.size()) {
        return false;
    }

    const std::uint32_t node_id = nodes[node_index].node_id;
    for (std::size_t index = 0U; index < node_index; ++index) {
        if (nodes[index].node_id == node_id) {
            return false;
        }
    }

    return true;
}
}
