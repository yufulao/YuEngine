// 模块: Tools UiWebEditorStatePreview
// 文件: Tools/UiWebEditorStatePreview/Src/UiWebEditorStatePreviewData.cpp

#include "YuEngine/UiWebEditorStatePreview/UiWebEditorStatePreviewData.h"

#include <cstddef>
#include <string_view>

namespace yuengine::ui_web_editor_state_preview {
namespace {
constexpr std::uint32_t INPUT_KEY_VISIBLE = 5101U;
constexpr std::uint32_t INPUT_KEY_ENABLED = 5102U;
constexpr std::uint32_t INPUT_KEY_RESOURCE = 5103U;
constexpr std::uint32_t DEFAULT_NODE_ID = 1U;
constexpr std::uint32_t DEFAULT_RESOURCE_KEY = 6101U;

void CopyString(std::string_view value, char *out_text, std::size_t capacity) {
    if (out_text == nullptr) {
        return;
    }

    if (capacity == 0U) {
        return;
    }

    std::size_t copy_count = value.size();
    if (copy_count >= capacity) {
        copy_count = capacity - 1U;
    }

    for (std::size_t index = 0U; index < copy_count; ++index) {
        out_text[index] = value[index];
    }

    out_text[copy_count] = '\0';
}

yuengine::uicore::UiNodeId NodeId(std::uint32_t value) {
    return yuengine::uicore::UiNodeId{value};
}

UiWebEditorStatePreviewInputRecord CreateInput(
    std::uint32_t input_key,
    std::string_view name,
    UiWebEditorStatePreviewValueKind value_kind,
    std::uint32_t value0) {
    UiWebEditorStatePreviewInputRecord record{};
    record.input_key = input_key;
    record.node_id = NodeId(DEFAULT_NODE_ID);
    record.value_kind = value_kind;
    record.value0 = value0;
    CopyString(name, record.name, UI_WEB_EDITOR_STATE_PREVIEW_NAME_CAPACITY);
    return record;
}
}

UiWebEditorStatePreviewStatus UiWebEditorStatePreviewData::WriteDefaultInputs(
    std::span<UiWebEditorStatePreviewInputRecord> out_inputs,
    UiWebEditorStatePreviewCatalogResult *out_result) const {
    if (out_result == nullptr) {
        return UiWebEditorStatePreviewStatus::InvalidInput;
    }

    *out_result = UiWebEditorStatePreviewCatalogResult{};
    out_result->input_count = UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT;
    out_result->output_capacity = static_cast<std::uint32_t>(out_inputs.size());
    if (!HasStorage(
        out_inputs.data(),
        out_inputs.size(),
        UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT)) {
        out_result->status = UiWebEditorStatePreviewStatus::OutputCapacityExceeded;
        return UiWebEditorStatePreviewStatus::OutputCapacityExceeded;
    }

    out_inputs[0U] = CreateInput(
        INPUT_KEY_VISIBLE,
        "Visible",
        UiWebEditorStatePreviewValueKind::Bool,
        1U);
    out_inputs[0U].affects_visibility = true;
    out_inputs[1U] = CreateInput(
        INPUT_KEY_ENABLED,
        "Enabled",
        UiWebEditorStatePreviewValueKind::Bool,
        1U);
    out_inputs[1U].affects_enabled = true;
    out_inputs[2U] = CreateInput(
        INPUT_KEY_RESOURCE,
        "ResourceKey",
        UiWebEditorStatePreviewValueKind::ResourceKey,
        DEFAULT_RESOURCE_KEY);
    out_inputs[2U].affects_resource = true;
    out_result->status = UiWebEditorStatePreviewStatus::Success;
    return UiWebEditorStatePreviewStatus::Success;
}

UiWebEditorStatePreviewStatus UiWebEditorStatePreviewData::BuildPreview(
    const UiWebEditorStatePreviewRequest &request,
    UiWebEditorStatePreviewOutput output) const {
    if (output.result == nullptr) {
        return UiWebEditorStatePreviewStatus::InvalidInput;
    }

    *output.result = UiWebEditorStatePreviewResult{};
    output.result->document_id = request.document_id;
    output.result->layout_id = request.schema.header.layout_id;
    output.result->state_input_count = static_cast<std::uint32_t>(request.state_inputs.size());
    if (request.document_id == 0U ||
        !IsStorageValid(request.state_inputs.data(), request.state_inputs.size())) {
        output.result->status = UiWebEditorStatePreviewStatus::InvalidInput;
        return UiWebEditorStatePreviewStatus::InvalidInput;
    }

    if (output.inspector == nullptr) {
        output.result->status = UiWebEditorStatePreviewStatus::InvalidOutput;
        return UiWebEditorStatePreviewStatus::InvalidOutput;
    }

    if (!HasOutputCapacity(request, output)) {
        output.result->status = UiWebEditorStatePreviewStatus::OutputCapacityExceeded;
        return UiWebEditorStatePreviewStatus::OutputCapacityExceeded;
    }

    for (const UiWebEditorStatePreviewInputRecord &record : request.state_inputs) {
        if (IsStateInputValid(request.schema, record)) {
            continue;
        }

        output.result->status = UiWebEditorStatePreviewStatus::InvalidStateInput;
        return UiWebEditorStatePreviewStatus::InvalidStateInput;
    }

    yuengine::ui_web_editor_validator::UiWebEditorValidationReport validation_report{};
    yuengine::ui_web_editor_validator::UiWebEditorValidationOutput validation_output{};
    validation_output.schema_issues = output.schema_issues;
    validation_output.validation_issues = output.validation_issues;
    validation_output.hierarchy = output.hierarchy;
    validation_output.inspector = output.inspector;
    validation_output.canvas = output.canvas;
    validation_output.resources = output.resources;
    validation_output.preview_diagnostics = output.preview_diagnostics;
    validation_output.report = &validation_report;

    yuengine::ui_web_editor_validator::UiWebEditorValidationRequest validation_request{};
    validation_request.document_id = request.document_id;
    validation_request.schema = request.schema;
    validation_request.selected_node_id = request.selected_node_id;
    validation_request.has_selection = request.has_selection;
    validation_request.preview_message_kind = request.preview_message_kind;
    validation_request.preview_request_id = request.preview_request_id;

    const yuengine::ui_web_editor_validator::UiWebEditorValidatorIntegration validator{};
    const yuengine::ui_web_editor_validator::UiWebEditorValidationStatus validation_status =
        validator.Validate(validation_request, validation_output);
    CopyValidationReport(validation_report, output.result);
    if (validation_status != yuengine::ui_web_editor_validator::UiWebEditorValidationStatus::Success) {
        output.result->status = MapValidationStatus(validation_status);
        return output.result->status;
    }

    WriteStateOutputs(request, output);
    output.result->status = UiWebEditorStatePreviewStatus::Success;
    output.result->state_output_count = static_cast<std::uint32_t>(request.state_inputs.size());
    output.result->schema_ready = true;
    output.result->validation_checked = true;
    return UiWebEditorStatePreviewStatus::Success;
}

bool UiWebEditorStatePreviewData::IsKnownValueKind(
    UiWebEditorStatePreviewValueKind value_kind) const {
    if (value_kind == UiWebEditorStatePreviewValueKind::Bool) {
        return true;
    }

    if (value_kind == UiWebEditorStatePreviewValueKind::Number1000) {
        return true;
    }

    return value_kind == UiWebEditorStatePreviewValueKind::ResourceKey;
}

bool UiWebEditorStatePreviewData::IsStorageValid(const void *data, std::size_t count) const {
    if (count == 0U) {
        return true;
    }

    return data != nullptr;
}

bool UiWebEditorStatePreviewData::HasStorage(
    const void *data,
    std::size_t capacity,
    std::uint32_t required_count) const {
    if (required_count == 0U) {
        return true;
    }

    if (capacity < static_cast<std::size_t>(required_count)) {
        return false;
    }

    return data != nullptr;
}

bool UiWebEditorStatePreviewData::ContainsNodeId(
    std::span<const yuengine::uicore::UiFileNodeRecord> nodes,
    yuengine::uicore::UiNodeId node_id) const {
    if (!node_id.IsValid()) {
        return false;
    }

    for (const yuengine::uicore::UiFileNodeRecord &record : nodes) {
        if (record.node_id.value == node_id.value) {
            return true;
        }
    }

    return false;
}

bool UiWebEditorStatePreviewData::IsStateInputValid(
    const yuengine::uicore::UiFileSchemaDesc &schema,
    const UiWebEditorStatePreviewInputRecord &record) const {
    if (record.input_key == 0U) {
        return false;
    }

    if (!IsKnownValueKind(record.value_kind)) {
        return false;
    }

    if (record.name[0U] == '\0') {
        return false;
    }

    if (!ContainsNodeId(schema.nodes, record.node_id)) {
        return false;
    }

    if (record.value_kind == UiWebEditorStatePreviewValueKind::Bool &&
        record.value0 > 1U) {
        return false;
    }

    return true;
}

bool UiWebEditorStatePreviewData::HasOutputCapacity(
    const UiWebEditorStatePreviewRequest &request,
    const UiWebEditorStatePreviewOutput &output) const {
    const std::uint32_t required_count = static_cast<std::uint32_t>(request.state_inputs.size());
    return HasStorage(output.state_outputs.data(), output.state_outputs.size(), required_count);
}

UiWebEditorStatePreviewStatus UiWebEditorStatePreviewData::MapValidationStatus(
    yuengine::ui_web_editor_validator::UiWebEditorValidationStatus status) const {
    if (status == yuengine::ui_web_editor_validator::UiWebEditorValidationStatus::IssuesFound) {
        return UiWebEditorStatePreviewStatus::IssuesFound;
    }

    if (status == yuengine::ui_web_editor_validator::UiWebEditorValidationStatus::OutputCapacityExceeded) {
        return UiWebEditorStatePreviewStatus::OutputCapacityExceeded;
    }

    if (status == yuengine::ui_web_editor_validator::UiWebEditorValidationStatus::InvalidInput ||
        status == yuengine::ui_web_editor_validator::UiWebEditorValidationStatus::InvalidDocument) {
        return UiWebEditorStatePreviewStatus::InvalidInput;
    }

    return UiWebEditorStatePreviewStatus::ValidatorRejected;
}

void UiWebEditorStatePreviewData::CopyValidationReport(
    const yuengine::ui_web_editor_validator::UiWebEditorValidationReport &report,
    UiWebEditorStatePreviewResult *out_result) const {
    if (out_result == nullptr) {
        return;
    }

    out_result->validation_status = report.status;
    out_result->preview_status = report.preview_status;
    out_result->document_id = report.document_id;
    out_result->layout_id = report.layout_id;
    out_result->checked_node_count = report.checked_node_count;
    out_result->schema_issue_count = report.schema_issue_count;
    out_result->validation_issue_count = report.validation_issue_count;
    out_result->preview_diagnostic_count = report.preview_diagnostic_count;
}

void UiWebEditorStatePreviewData::WriteStateOutputs(
    const UiWebEditorStatePreviewRequest &request,
    UiWebEditorStatePreviewOutput output) const {
    for (std::size_t index = 0U; index < request.state_inputs.size(); ++index) {
        const UiWebEditorStatePreviewInputRecord &input = request.state_inputs[index];
        UiWebEditorStatePreviewOutputRecord record{};
        record.input_key = input.input_key;
        record.node_id = input.node_id;
        record.value_kind = input.value_kind;
        record.value0 = input.value0;
        record.value1 = input.value1;
        record.value2 = input.value2;
        record.value3 = input.value3;
        record.accepted = true;
        record.schema_checked = true;
        record.validation_checked = true;
        output.state_outputs[index] = record;
    }
}
}
