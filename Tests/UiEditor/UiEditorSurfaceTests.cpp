// Module: Tests UiEditor
// File: Tests/UiEditor/UiEditorSurfaceTests.cpp

#include <array>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <span>
#include <string_view>

#include "YuEngine/File/MountId.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/File/VirtualPath.h"
#include "YuEngine/PreviewHost/PreviewHost.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiEditor/UiEditorSurface.h"

using yuengine::file::FileStatus;
using yuengine::file::MountId;
using yuengine::file::MountTable;
using yuengine::file::VirtualPath;
using yuengine::previewhost::PreviewHostFrameFormat;
using yuengine::previewhost::PreviewHostFrameResult;
using yuengine::previewhost::PreviewHostStatus;
using yuengine::serialize::SerializeStatus;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UiRectTransform;
using yuengine::uieditor::BuildUiEditorRuntimeDocumentSurface;
using yuengine::uieditor::BuildUiEditorDesignInspectorWorkflowSurface;
using yuengine::uieditor::BuildUiEditorRuntimePreviewStyleTemplateStateWorkflow;
using yuengine::uieditor::BuildUiEditorStyleThemeTemplateSerializationWorkflow;
using yuengine::uieditor::UiEditorDesignCommand;
using yuengine::uieditor::UiEditorDesignCommandKind;
using yuengine::uieditor::UiEditorDesignCommandLedgerRecord;
using yuengine::uieditor::UiEditorDesignInspectorWorkflowRequest;
using yuengine::uieditor::UiEditorDesignInspectorWorkflowResult;
using yuengine::uieditor::UiEditorDesignSurfaceRow;
using yuengine::uieditor::UiEditorDesignWorkflowBlockedLayer;
using yuengine::uieditor::UiEditorDesignWorkflowStatus;
using yuengine::uieditor::UiEditorComponentKind;
using yuengine::uieditor::UiEditorHierarchyRow;
using yuengine::uieditor::UiEditorInspectorFieldKind;
using yuengine::uieditor::UiEditorInspectorFieldRow;
using yuengine::uieditor::UiEditorPreviewFeedbackRecord;
using yuengine::uieditor::UiEditorRuntimeDocument;
using yuengine::uieditor::UiEditorRuntimeDocumentHeader;
using yuengine::uieditor::UiEditorRuntimeDocumentSurfaceRequest;
using yuengine::uieditor::UiEditorRuntimeDocumentSurfaceResult;
using yuengine::uieditor::UiEditorRuntimeNodeRecord;
using yuengine::uieditor::UiEditorRuntimePreviewStyleTemplateStateRow;
using yuengine::uieditor::UiEditorRuntimePreviewWorkflowBlockedLayer;
using yuengine::uieditor::UiEditorRuntimePreviewWorkflowRequest;
using yuengine::uieditor::UiEditorRuntimePreviewWorkflowResult;
using yuengine::uieditor::UiEditorRuntimePreviewWorkflowStatus;
using yuengine::uieditor::UiEditorStyleTemplateStateCommand;
using yuengine::uieditor::UiEditorStyleTemplateStateCommandKind;
using yuengine::uieditor::UiEditorStyleTemplateStateLedgerRecord;
using yuengine::uieditor::UiEditorStyleTemplateStateRecord;
using yuengine::uieditor::UiEditorStyleThemeTemplateSerializationBlockedLayer;
using yuengine::uieditor::UiEditorStyleThemeTemplateSerializationRow;
using yuengine::uieditor::UiEditorStyleThemeTemplateSerializationStatus;
using yuengine::uieditor::UiEditorStyleThemeTemplateSerializationWorkflowRequest;
using yuengine::uieditor::UiEditorStyleThemeTemplateSerializationWorkflowResult;
using yuengine::uieditor::UiEditorSurfaceBlockedLayer;
using yuengine::uieditor::UiEditorSurfaceStatus;

namespace {
constexpr const char *TEST_DOCUMENT =
    "UiEditorSurface_BuildsRuntimeDocumentHierarchyRows";
constexpr const char *TEST_PREVIEW =
    "UiEditorSurface_ConsumesPreviewHostFrameFeedback";
constexpr const char *TEST_NO_MUTATION =
    "UiEditorSurface_RejectsMissingPreviewFeedbackWithoutMutation";
constexpr const char *TEST_WORKFLOW_SELECTION =
    "UiEditorWorkflow_NodeSelectionBuildsDesignSurface";
constexpr const char *TEST_WORKFLOW_LAYOUT_PREVIEW =
    "UiEditorWorkflow_MultiComponentLayoutPreviewRecordsTextAndSlider";
constexpr const char *TEST_WORKFLOW_INSPECTOR =
    "UiEditorWorkflow_InspectorRowsExposeComponentState";
constexpr const char *TEST_WORKFLOW_EDIT =
    "UiEditorWorkflow_ComponentEditStagesDocumentAndLedger";
constexpr const char *TEST_WORKFLOW_MISSING_PREVIEW =
    "UiEditorWorkflow_MissingPreviewFeedbackDoesNotMutateOutputs";
constexpr const char *TEST_WORKFLOW_INVALID_COMPONENT =
    "UiEditorWorkflow_InvalidComponentDoesNotMutateOutputs";
constexpr const char *TEST_WORKFLOW_CAPACITY =
    "UiEditorWorkflow_OutputCapacityDoesNotMutateOutputs";
constexpr const char *TEST_RUNTIME_PREVIEW =
    "UiEditorRuntimePreviewWorkflow_BuildsStyleTemplateStateRows";
constexpr const char *TEST_RUNTIME_PREVIEW_MISSING_PREVIEW =
    "UiEditorRuntimePreviewWorkflow_MissingPreviewFeedbackDoesNotMutateOutputs";
constexpr const char *TEST_RUNTIME_PREVIEW_INVALID_STYLE =
    "UiEditorRuntimePreviewWorkflow_InvalidStyleTemplateStateDoesNotMutateOutputs";
constexpr const char *TEST_RUNTIME_PREVIEW_CAPACITY =
    "UiEditorRuntimePreviewWorkflow_OutputCapacityDoesNotMutateOutputs";
constexpr const char *TEST_SERIALIZATION =
    "UiEditorStyleThemeTemplateSerializationWorkflow_RoundTripsThroughFileVfs";
constexpr const char *TEST_SERIALIZATION_MISSING_FILE =
    "UiEditorStyleThemeTemplateSerializationWorkflow_MissingFileVfsDoesNotMutateOutputs";
constexpr const char *TEST_SERIALIZATION_CAPACITY =
    "UiEditorStyleThemeTemplateSerializationWorkflow_OutputCapacityDoesNotMutateOutputs";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t DOCUMENT_ID = 5101U;
constexpr float TOLERANCE = 0.0001F;
constexpr const char *UI_EDITOR_TEST_MOUNT = "UiEditor";
constexpr const char *UI_EDITOR_SERIALIZATION_PATH = "Runtime/StyleThemeTemplate.bin";

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool Approx(float left, float right) {
    const float delta = std::fabs(left - right);
    return delta <= TOLERANCE;
}

UiRectTransform StretchTransform() {
    UiRectTransform transform{};
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.pivot = {0.5F, 0.5F};
    return transform;
}

UiRectTransform ChildTransform() {
    UiRectTransform transform{};
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.offset_min = {10.0F, 20.0F};
    transform.offset_max = {-30.0F, -40.0F};
    transform.pivot = {0.5F, 0.5F};
    return transform;
}

UiRectTransform AbsoluteChildTransform(
    float x,
    float y,
    float width,
    float height) {
    UiRectTransform transform{};
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {0.0F, 0.0F};
    transform.offset_min = {x, y};
    transform.offset_max = {x + width, y + height};
    transform.pivot = {0.5F, 0.5F};
    return transform;
}

UiEditorRuntimeDocumentHeader Header(std::uint32_t node_count=2U) {
    UiEditorRuntimeDocumentHeader header{};
    header.document_id = DOCUMENT_ID;
    header.schema_version = 1U;
    header.node_count = node_count;
    header.viewport_width = 800.0F;
    header.viewport_height = 600.0F;
    header.is_valid = true;
    return header;
}

UiEditorRuntimeNodeRecord RootNode() {
    UiEditorRuntimeNodeRecord record{};
    record.node_id = UiNodeId{1U};
    record.parent_id = UiNodeId{};
    record.rect_transform = StretchTransform();
    record.component_kind = UiEditorComponentKind::Panel;
    record.sibling_order = 0U;
    record.layer = 1;
    return record;
}

UiEditorRuntimeNodeRecord ChildNode() {
    UiEditorRuntimeNodeRecord record{};
    record.node_id = UiNodeId{2U};
    record.parent_id = UiNodeId{1U};
    record.rect_transform = ChildTransform();
    record.component_kind = UiEditorComponentKind::Button;
    record.sibling_order = 0U;
    record.layer = 2;
    return record;
}

UiEditorRuntimeNodeRecord TextNode() {
    UiEditorRuntimeNodeRecord record{};
    record.node_id = UiNodeId{3U};
    record.parent_id = UiNodeId{1U};
    record.rect_transform = AbsoluteChildTransform(32.0F, 420.0F, 360.0F, 48.0F);
    record.component_kind = UiEditorComponentKind::Text;
    record.sibling_order = 1U;
    record.layer = 3;
    return record;
}

UiEditorRuntimeNodeRecord SliderNode() {
    UiEditorRuntimeNodeRecord record{};
    record.node_id = UiNodeId{4U};
    record.parent_id = UiNodeId{1U};
    record.rect_transform = AbsoluteChildTransform(32.0F, 320.0F, 300.0F, 24.0F);
    record.component_kind = UiEditorComponentKind::Slider;
    record.sibling_order = 2U;
    record.layer = 3;
    return record;
}

PreviewHostFrameResult PreviewFrame() {
    PreviewHostFrameResult result{};
    result.status = PreviewHostStatus::Success;
    result.frame.frame_id = 71U;
    result.frame.width = 800U;
    result.frame.height = 600U;
    result.frame.format = PreviewHostFrameFormat::Headless;
    result.diagnostic_count = 0U;
    result.capture_bytes_written = 128U;
    result.submitted_render_scene_frame = true;
    result.headless_output = true;
    return result;
}

std::filesystem::path SerializationFixtureRoot() {
    return std::filesystem::temp_directory_path() /
        "YuEngineUiEditorSerializationTests";
}

UiEditorRuntimeDocumentSurfaceRequest SurfaceRequest(
    const UiEditorRuntimeDocument &document,
    UiNodeId selected_node_id,
    const PreviewHostFrameResult *preview_frame,
    std::span<UiEditorHierarchyRow> hierarchy_output,
    std::span<UiEditorPreviewFeedbackRecord> preview_output,
    bool require_preview_feedback=true) {
    UiEditorRuntimeDocumentSurfaceRequest request{};
    request.document = &document;
    request.selected_node_id = selected_node_id;
    request.preview_frame = preview_frame;
    request.require_preview_feedback = require_preview_feedback;
    request.hierarchy_output = hierarchy_output;
    request.preview_feedback_output = preview_output;
    return request;
}

UiEditorDesignInspectorWorkflowRequest WorkflowRequest(
    const UiEditorRuntimeDocument &document,
    UiNodeId selected_node_id,
    const PreviewHostFrameResult *preview_frame,
    const UiEditorDesignCommand &command,
    std::span<UiEditorHierarchyRow> hierarchy_output,
    std::span<UiEditorDesignSurfaceRow> design_output,
    std::span<UiEditorInspectorFieldRow> inspector_output,
    std::span<UiEditorPreviewFeedbackRecord> preview_output,
    std::span<UiEditorRuntimeNodeRecord> staged_output,
    std::span<UiEditorDesignCommandLedgerRecord> ledger_output) {
    UiEditorDesignInspectorWorkflowRequest request{};
    request.document = &document;
    request.selected_node_id = selected_node_id;
    request.preview_frame = preview_frame;
    request.command = command;
    request.hierarchy_output = hierarchy_output;
    request.design_surface_output = design_output;
    request.inspector_output = inspector_output;
    request.preview_feedback_output = preview_output;
    request.staged_document_output = staged_output;
    request.command_ledger_output = ledger_output;
    return request;
}

UiEditorStyleTemplateStateRecord ButtonStyleState() {
    UiEditorStyleTemplateStateRecord record{};
    record.node_id = UiNodeId{2U};
    record.component_kind = UiEditorComponentKind::Button;
    record.style_key = 810U;
    record.theme_key = 875U;
    record.template_key = 920U;
    record.state_revision = 7U;
    record.hovered = true;
    record.focused = false;
    record.pressed = false;
    record.disabled = false;
    record.runtime_state_valid = true;
    record.style_resolved = true;
    record.template_instanced = true;
    return record;
}

UiEditorRuntimePreviewWorkflowRequest RuntimePreviewWorkflowRequest(
    const UiEditorRuntimeDocument &document,
    UiNodeId selected_node_id,
    const PreviewHostFrameResult *preview_frame,
    const UiEditorDesignCommand &design_command,
    const UiEditorStyleTemplateStateCommand &style_command,
    std::span<const UiEditorStyleTemplateStateRecord> style_records,
    std::span<UiEditorHierarchyRow> hierarchy_output,
    std::span<UiEditorDesignSurfaceRow> design_output,
    std::span<UiEditorInspectorFieldRow> inspector_output,
    std::span<UiEditorPreviewFeedbackRecord> preview_output,
    std::span<UiEditorRuntimeNodeRecord> staged_output,
    std::span<UiEditorDesignCommandLedgerRecord> ledger_output,
    std::span<UiEditorRuntimePreviewStyleTemplateStateRow> runtime_preview_output,
    std::span<UiEditorStyleTemplateStateLedgerRecord> style_ledger_output) {
    UiEditorRuntimePreviewWorkflowRequest request{};
    request.document = &document;
    request.selected_node_id = selected_node_id;
    request.preview_frame = preview_frame;
    request.design_command = design_command;
    request.style_template_state_command = style_command;
    request.style_template_state_records = style_records;
    request.hierarchy_output = hierarchy_output;
    request.design_surface_output = design_output;
    request.inspector_output = inspector_output;
    request.preview_feedback_output = preview_output;
    request.staged_document_output = staged_output;
    request.command_ledger_output = ledger_output;
    request.runtime_preview_output = runtime_preview_output;
    request.style_template_state_ledger_output = style_ledger_output;
    return request;
}

UiEditorStyleThemeTemplateSerializationWorkflowRequest SerializationWorkflowRequest(
    const UiEditorRuntimeDocument &document,
    UiNodeId selected_node_id,
    const PreviewHostFrameResult *preview_frame,
    const UiEditorDesignCommand &design_command,
    const UiEditorStyleTemplateStateCommand &style_command,
    std::span<const UiEditorStyleTemplateStateRecord> style_records,
    MountTable *mount_table,
    std::span<UiEditorHierarchyRow> hierarchy_output,
    std::span<UiEditorDesignSurfaceRow> design_output,
    std::span<UiEditorInspectorFieldRow> inspector_output,
    std::span<UiEditorPreviewFeedbackRecord> preview_output,
    std::span<UiEditorRuntimeNodeRecord> staged_output,
    std::span<UiEditorDesignCommandLedgerRecord> ledger_output,
    std::span<UiEditorRuntimePreviewStyleTemplateStateRow> runtime_preview_output,
    std::span<UiEditorStyleTemplateStateLedgerRecord> style_ledger_output,
    std::span<UiEditorStyleThemeTemplateSerializationRow> serialization_output) {
    UiEditorStyleThemeTemplateSerializationWorkflowRequest request{};
    request.document = &document;
    request.selected_node_id = selected_node_id;
    request.preview_frame = preview_frame;
    request.design_command = design_command;
    request.style_template_state_command = style_command;
    request.style_template_state_records = style_records;
    request.file_mount_table = mount_table;
    request.file_mount = MountId(UI_EDITOR_TEST_MOUNT);
    request.file_path = VirtualPath(UI_EDITOR_SERIALIZATION_PATH);
    request.hierarchy_output = hierarchy_output;
    request.design_surface_output = design_output;
    request.inspector_output = inspector_output;
    request.preview_feedback_output = preview_output;
    request.staged_document_output = staged_output;
    request.command_ledger_output = ledger_output;
    request.runtime_preview_output = runtime_preview_output;
    request.style_template_state_ledger_output = style_ledger_output;
    request.serialization_output = serialization_output;
    return request;
}

bool SentinelHierarchyUnchanged(const UiEditorHierarchyRow &row) {
    return row.document_id == 9001U && row.node_id.value == 9002U && !row.selected;
}

bool SentinelPreviewUnchanged(const UiEditorPreviewFeedbackRecord &record) {
    return record.document_id == 9003U && !record.feedback_from_preview_host;
}

bool SentinelDesignUnchanged(const UiEditorDesignSurfaceRow &row) {
    return row.document_id == 9004U && row.node_id.value == 9005U && !row.selected;
}

bool SentinelInspectorUnchanged(const UiEditorInspectorFieldRow &row) {
    return row.document_id == 9006U && row.node_id.value == 9007U && !row.editable;
}

bool SentinelStagedNodeUnchanged(const UiEditorRuntimeNodeRecord &record) {
    return record.node_id.value == 9008U &&
        record.component_kind == UiEditorComponentKind::Unknown &&
        record.layer == 9009;
}

bool SentinelDesignLedgerUnchanged(
    const UiEditorDesignCommandLedgerRecord &record) {
    return record.document_id == 9010U &&
        record.command_sequence == 9011U &&
        !record.command_applied;
}

bool SentinelRuntimePreviewUnchanged(
    const UiEditorRuntimePreviewStyleTemplateStateRow &row) {
    return row.document_id == 9012U &&
        row.node_id.value == 9013U &&
        !row.engine_runtime_preview;
}

bool SentinelStyleLedgerUnchanged(
    const UiEditorStyleTemplateStateLedgerRecord &record) {
    return record.document_id == 9014U &&
        record.command_sequence == 9015U &&
        !record.command_applied;
}

bool SentinelSerializationUnchanged(
    const UiEditorStyleThemeTemplateSerializationRow &row) {
    return row.document_id == 9016U &&
        row.node_id.value == 9017U &&
        !row.saved_through_file_vfs;
}

void SeedWorkflowSentinels(
    std::span<UiEditorHierarchyRow> hierarchy_output,
    std::span<UiEditorDesignSurfaceRow> design_output,
    std::span<UiEditorInspectorFieldRow> inspector_output,
    std::span<UiEditorPreviewFeedbackRecord> preview_output,
    std::span<UiEditorRuntimeNodeRecord> staged_output,
    std::span<UiEditorDesignCommandLedgerRecord> ledger_output) {
    hierarchy_output[0U].document_id = 9001U;
    hierarchy_output[0U].node_id = UiNodeId{9002U};
    hierarchy_output[0U].selected = false;
    design_output[0U].document_id = 9004U;
    design_output[0U].node_id = UiNodeId{9005U};
    design_output[0U].selected = false;
    inspector_output[0U].document_id = 9006U;
    inspector_output[0U].node_id = UiNodeId{9007U};
    inspector_output[0U].editable = false;
    preview_output[0U].document_id = 9003U;
    preview_output[0U].feedback_from_preview_host = false;
    staged_output[0U].node_id = UiNodeId{9008U};
    staged_output[0U].component_kind = UiEditorComponentKind::Unknown;
    staged_output[0U].layer = 9009;
    ledger_output[0U].document_id = 9010U;
    ledger_output[0U].command_sequence = 9011U;
    ledger_output[0U].command_applied = false;
}

void SeedRuntimePreviewSentinels(
    std::span<UiEditorHierarchyRow> hierarchy_output,
    std::span<UiEditorDesignSurfaceRow> design_output,
    std::span<UiEditorInspectorFieldRow> inspector_output,
    std::span<UiEditorPreviewFeedbackRecord> preview_output,
    std::span<UiEditorRuntimeNodeRecord> staged_output,
    std::span<UiEditorDesignCommandLedgerRecord> ledger_output,
    std::span<UiEditorRuntimePreviewStyleTemplateStateRow> runtime_preview_output,
    std::span<UiEditorStyleTemplateStateLedgerRecord> style_ledger_output) {
    SeedWorkflowSentinels(
        hierarchy_output,
        design_output,
        inspector_output,
        preview_output,
        staged_output,
        ledger_output);
    runtime_preview_output[0U].document_id = 9012U;
    runtime_preview_output[0U].node_id = UiNodeId{9013U};
    runtime_preview_output[0U].engine_runtime_preview = false;
    style_ledger_output[0U].document_id = 9014U;
    style_ledger_output[0U].command_sequence = 9015U;
    style_ledger_output[0U].command_applied = false;
}

void SeedSerializationSentinels(
    std::span<UiEditorHierarchyRow> hierarchy_output,
    std::span<UiEditorDesignSurfaceRow> design_output,
    std::span<UiEditorInspectorFieldRow> inspector_output,
    std::span<UiEditorPreviewFeedbackRecord> preview_output,
    std::span<UiEditorRuntimeNodeRecord> staged_output,
    std::span<UiEditorDesignCommandLedgerRecord> ledger_output,
    std::span<UiEditorRuntimePreviewStyleTemplateStateRow> runtime_preview_output,
    std::span<UiEditorStyleTemplateStateLedgerRecord> style_ledger_output,
    std::span<UiEditorStyleThemeTemplateSerializationRow> serialization_output) {
    SeedRuntimePreviewSentinels(
        hierarchy_output,
        design_output,
        inspector_output,
        preview_output,
        staged_output,
        ledger_output,
        runtime_preview_output,
        style_ledger_output);
    serialization_output[0U].document_id = 9016U;
    serialization_output[0U].node_id = UiNodeId{9017U};
    serialization_output[0U].saved_through_file_vfs = false;
}

int UiEditorSurfaceBuildsRuntimeDocumentHierarchyRows() {
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorPreviewFeedbackRecord, 0U> preview_output{};

    UiEditorRuntimeDocumentSurfaceResult result{};
    const UiEditorSurfaceStatus status =
        BuildUiEditorRuntimeDocumentSurface(
            SurfaceRequest(
                document,
                UiNodeId{2U},
                nullptr,
                hierarchy_output,
                preview_output,
                false),
            &result);
    if (status != UiEditorSurfaceStatus::Success || !result.Succeeded()) {
        return Fail("ui editor document surface failed");
    }

    if (result.hierarchy_row_count != 2U || !result.built_ui_node_tree ||
        !result.built_hierarchy_rows || !result.consumed_runtime_ui_document) {
        return Fail("ui editor document surface counters mismatch");
    }

    if (hierarchy_output[0U].node_id.value != 1U ||
        hierarchy_output[0U].component_kind != UiEditorComponentKind::Panel ||
        hierarchy_output[0U].depth != 0U) {
        return Fail("ui editor root row mismatch");
    }

    if (hierarchy_output[1U].node_id.value != 2U ||
        hierarchy_output[1U].parent_id.value != 1U ||
        hierarchy_output[1U].component_kind != UiEditorComponentKind::Button ||
        hierarchy_output[1U].depth != 1U ||
        !hierarchy_output[1U].selected) {
        return Fail("ui editor child row mismatch");
    }

    if (!Approx(hierarchy_output[1U].world_rect.x, 10.0F) ||
        !Approx(hierarchy_output[1U].world_rect.y, 20.0F) ||
        !Approx(hierarchy_output[1U].world_rect.width, 760.0F) ||
        !Approx(hierarchy_output[1U].world_rect.height, 540.0F)) {
        return Fail("ui editor child rect mismatch");
    }

    if (result.mutated_runtime_data || result.opened_native_window ||
        result.used_forbidden_preview_path) {
        return Fail("ui editor boundary flags changed");
    }

    return 0;
}

int UiEditorSurfaceConsumesPreviewHostFrameFeedback() {
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};

    UiEditorRuntimeDocumentSurfaceResult result{};
    const UiEditorSurfaceStatus status =
        BuildUiEditorRuntimeDocumentSurface(
            SurfaceRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                hierarchy_output,
                preview_output),
            &result);
    if (status != UiEditorSurfaceStatus::Success) {
        return Fail("ui editor preview feedback surface failed");
    }

    if (!result.consumed_preview_host_feedback || !result.emitted_preview_feedback ||
        result.preview_feedback_count != 1U) {
        return Fail("ui editor preview feedback counters mismatch");
    }

    if (preview_output[0U].document_id != DOCUMENT_ID ||
        preview_output[0U].selected_node_id.value != 2U ||
        preview_output[0U].frame_id != 71U ||
        preview_output[0U].viewport_width != 800U ||
        preview_output[0U].viewport_height != 600U ||
        preview_output[0U].preview_status != PreviewHostStatus::Success ||
        !preview_output[0U].preview_frame_built ||
        !preview_output[0U].headless_output ||
        !preview_output[0U].feedback_from_preview_host) {
        return Fail("ui editor preview feedback record mismatch");
    }

    return 0;
}

int UiEditorSurfaceRejectsMissingPreviewFeedbackWithoutMutation() {
    const std::array<UiEditorRuntimeNodeRecord, 1U> nodes{RootNode()};
    const UiEditorRuntimeDocument document{Header(1U), nodes};
    std::array<UiEditorHierarchyRow, 1U> hierarchy_output{};
    hierarchy_output[0U].document_id = 9001U;
    hierarchy_output[0U].node_id = UiNodeId{9002U};
    hierarchy_output[0U].selected = false;
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    preview_output[0U].document_id = 9003U;
    preview_output[0U].feedback_from_preview_host = false;

    UiEditorRuntimeDocumentSurfaceResult result{};
    const UiEditorSurfaceStatus status =
        BuildUiEditorRuntimeDocumentSurface(
            SurfaceRequest(
                document,
                UiNodeId{1U},
                nullptr,
                hierarchy_output,
                preview_output),
            &result);
    if (status != UiEditorSurfaceStatus::PreviewFeedbackMissing) {
        return Fail("ui editor missing preview feedback status mismatch");
    }

    if (result.blocked_layer != UiEditorSurfaceBlockedLayer::PreviewHostFeedback) {
        return Fail("ui editor missing preview feedback layer mismatch");
    }

    if (!SentinelHierarchyUnchanged(hierarchy_output[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U])) {
        return Fail("ui editor missing preview feedback mutated output");
    }

    return 0;
}

int UiEditorWorkflowNodeSelectionBuildsDesignSurface() {
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    UiEditorDesignCommand command{};
    command.kind = UiEditorDesignCommandKind::None;
    command.command_sequence = 3U;

    UiEditorDesignInspectorWorkflowResult result{};
    const UiEditorDesignWorkflowStatus status =
        BuildUiEditorDesignInspectorWorkflowSurface(
            WorkflowRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                command,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output),
            &result);
    if (status != UiEditorDesignWorkflowStatus::Success || !result.Succeeded()) {
        return Fail("ui editor workflow selection failed");
    }

    if (result.hierarchy_row_count != 2U ||
        result.design_surface_row_count != 2U ||
        result.inspector_field_count != 7U ||
        result.preview_feedback_count != 1U ||
        result.staged_node_count != 2U ||
        result.command_ledger_count != 1U ||
        !result.consumed_runtime_ui_document ||
        !result.consumed_preview_host_feedback ||
        !result.built_design_surface ||
        !result.emitted_hierarchy_rows ||
        !result.emitted_inspector_fields ||
        !result.emitted_command_ledger ||
        result.command_applied ||
        result.staged_document_update ||
        result.mutated_runtime_data ||
        result.opened_native_window ||
        result.used_forbidden_preview_path) {
        return Fail("ui editor workflow selection counters mismatch");
    }

    if (!hierarchy_output[1U].selected ||
        !design_output[1U].selected ||
        design_output[1U].node_id.value != 2U ||
        design_output[1U].component_kind != UiEditorComponentKind::Button ||
        design_output[1U].preview_frame_id != 71U ||
        design_output[1U].preview_status != PreviewHostStatus::Success ||
        !design_output[1U].preview_feedback_available ||
        preview_output[0U].selected_node_id.value != 2U ||
        ledger_output[0U].command_kind != UiEditorDesignCommandKind::None ||
        ledger_output[0U].command_sequence != 3U ||
        ledger_output[0U].command_applied) {
        return Fail("ui editor workflow design selection output mismatch");
    }

    return 0;
}

int UiEditorWorkflowMultiComponentLayoutPreviewRecordsTextAndSlider() {
    const std::array<UiEditorRuntimeNodeRecord, 3U> nodes{
        RootNode(),
        TextNode(),
        SliderNode()};
    const UiEditorRuntimeDocument document{Header(3U), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    std::array<UiEditorHierarchyRow, 3U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 3U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 3U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    UiEditorDesignCommand command{};

    UiEditorDesignInspectorWorkflowResult result{};
    const UiEditorDesignWorkflowStatus status =
        BuildUiEditorDesignInspectorWorkflowSurface(
            WorkflowRequest(
                document,
                UiNodeId{3U},
                &preview_frame,
                command,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output),
            &result);
    if (status != UiEditorDesignWorkflowStatus::Success ||
        !result.Succeeded()) {
        return Fail("ui editor multi component preview workflow failed");
    }

    if (result.hierarchy_row_count != 3U ||
        result.design_surface_row_count != 3U ||
        result.preview_feedback_count != 1U ||
        !result.built_design_surface ||
        !result.consumed_preview_host_feedback) {
        return Fail("ui editor multi component preview counters mismatch");
    }

    const UiEditorPreviewFeedbackRecord &preview = preview_output[0U];
    if (preview.preview_component_count != 2U ||
        preview.preview_component_kind_count != 2U ||
        preview.text_component_count != 1U ||
        preview.slider_component_count != 1U ||
        preview.image_component_count != 0U ||
        preview.button_component_count != 0U ||
        !preview.multi_component_preview_layout ||
        preview.frame_id != 71U ||
        !preview.preview_frame_built ||
        !preview.feedback_from_preview_host) {
        return Fail("ui editor multi component preview feedback mismatch");
    }

    if (design_output[1U].node_id.value != 3U ||
        design_output[1U].component_kind != UiEditorComponentKind::Text ||
        !design_output[1U].selected ||
        design_output[1U].preview_frame_id != 71U ||
        !design_output[1U].preview_feedback_available) {
        return Fail("ui editor text design preview row mismatch");
    }

    if (design_output[2U].node_id.value != 4U ||
        design_output[2U].component_kind != UiEditorComponentKind::Slider ||
        design_output[2U].selected ||
        design_output[2U].preview_frame_id != 71U ||
        !design_output[2U].preview_feedback_available) {
        return Fail("ui editor slider design preview row mismatch");
    }

    if (!Approx(design_output[1U].world_rect.x, 32.0F) ||
        !Approx(design_output[1U].world_rect.y, 420.0F) ||
        !Approx(design_output[1U].world_rect.width, 360.0F) ||
        !Approx(design_output[2U].world_rect.x, 32.0F) ||
        !Approx(design_output[2U].world_rect.y, 320.0F) ||
        !Approx(design_output[2U].world_rect.width, 300.0F)) {
        return Fail("ui editor multi component layout rect mismatch");
    }

    if (inspector_output[0U].component_kind != UiEditorComponentKind::Text ||
        staged_output[1U].component_kind != UiEditorComponentKind::Text ||
        staged_output[2U].component_kind != UiEditorComponentKind::Slider) {
        return Fail("ui editor multi component staged output mismatch");
    }

    return 0;
}

int UiEditorWorkflowInspectorRowsExposeComponentState() {
    UiEditorRuntimeNodeRecord child = ChildNode();
    child.visible = false;
    child.enabled = false;
    child.hit_testable = false;
    child.layer = 4;
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), child};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    UiEditorDesignCommand command{};

    UiEditorDesignInspectorWorkflowResult result{};
    const UiEditorDesignWorkflowStatus status =
        BuildUiEditorDesignInspectorWorkflowSurface(
            WorkflowRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                command,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output),
            &result);
    if (status != UiEditorDesignWorkflowStatus::Success ||
        result.inspector_field_count != 7U ||
        !result.emitted_inspector_fields) {
        return Fail("ui editor workflow inspector failed");
    }

    if (inspector_output[0U].field_kind != UiEditorInspectorFieldKind::ComponentKind ||
        inspector_output[0U].component_kind != UiEditorComponentKind::Button ||
        inspector_output[1U].field_kind != UiEditorInspectorFieldKind::Visible ||
        inspector_output[1U].bool_value ||
        inspector_output[2U].field_kind != UiEditorInspectorFieldKind::Enabled ||
        inspector_output[2U].bool_value ||
        inspector_output[3U].field_kind != UiEditorInspectorFieldKind::HitTestable ||
        inspector_output[3U].bool_value ||
        inspector_output[5U].field_kind != UiEditorInspectorFieldKind::Layer ||
        inspector_output[5U].int_value != 4 ||
        inspector_output[6U].field_kind != UiEditorInspectorFieldKind::RectTransform ||
        !Approx(inspector_output[6U].rect_transform.offset_min.x, 10.0F) ||
        !inspector_output[6U].editable ||
        !inspector_output[6U].selected_node) {
        return Fail("ui editor workflow inspector field rows mismatch");
    }

    return 0;
}

int UiEditorWorkflowComponentEditStagesDocumentAndLedger() {
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    UiEditorDesignCommand command{};
    command.kind = UiEditorDesignCommandKind::SetEnabled;
    command.bool_value = false;
    command.command_sequence = 9U;

    UiEditorDesignInspectorWorkflowResult result{};
    const UiEditorDesignWorkflowStatus status =
        BuildUiEditorDesignInspectorWorkflowSurface(
            WorkflowRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                command,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output),
            &result);
    if (status != UiEditorDesignWorkflowStatus::Success ||
        !result.command_applied ||
        !result.staged_document_update ||
        !result.emitted_command_ledger) {
        return Fail("ui editor workflow component edit failed");
    }

    if (!nodes[1U].enabled ||
        staged_output[1U].enabled ||
        staged_output[1U].node_id.value != 2U ||
        ledger_output[0U].command_kind != UiEditorDesignCommandKind::SetEnabled ||
        ledger_output[0U].command_sequence != 9U ||
        !ledger_output[0U].before_enabled ||
        ledger_output[0U].after_enabled ||
        !ledger_output[0U].staged_document_update ||
        !ledger_output[0U].command_applied ||
        inspector_output[2U].bool_value) {
        return Fail("ui editor workflow component edit staged output mismatch");
    }

    return 0;
}

int UiEditorWorkflowMissingPreviewFeedbackDoesNotMutateOutputs() {
    const std::array<UiEditorRuntimeNodeRecord, 1U> nodes{RootNode()};
    const UiEditorRuntimeDocument document{Header(1U), nodes};
    std::array<UiEditorHierarchyRow, 1U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 1U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 1U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    SeedWorkflowSentinels(
        hierarchy_output,
        design_output,
        inspector_output,
        preview_output,
        staged_output,
        ledger_output);
    UiEditorDesignCommand command{};
    command.kind = UiEditorDesignCommandKind::SetVisible;
    command.bool_value = false;

    UiEditorDesignInspectorWorkflowResult result{};
    const UiEditorDesignWorkflowStatus status =
        BuildUiEditorDesignInspectorWorkflowSurface(
            WorkflowRequest(
                document,
                UiNodeId{1U},
                nullptr,
                command,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output),
            &result);
    if (status != UiEditorDesignWorkflowStatus::PreviewFeedbackMissing ||
        result.blocked_layer != UiEditorDesignWorkflowBlockedLayer::PreviewHostFeedback ||
        result.emitted_hierarchy_rows ||
        result.emitted_inspector_fields ||
        result.staged_document_update ||
        result.command_applied) {
        return Fail("ui editor workflow missing preview result mismatch");
    }

    if (!SentinelHierarchyUnchanged(hierarchy_output[0U]) ||
        !SentinelDesignUnchanged(design_output[0U]) ||
        !SentinelInspectorUnchanged(inspector_output[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelStagedNodeUnchanged(staged_output[0U]) ||
        !SentinelDesignLedgerUnchanged(ledger_output[0U])) {
        return Fail("ui editor workflow missing preview mutated outputs");
    }

    return 0;
}

int UiEditorWorkflowInvalidComponentDoesNotMutateOutputs() {
    UiEditorRuntimeNodeRecord invalid = RootNode();
    invalid.component_kind = UiEditorComponentKind::Unknown;
    const std::array<UiEditorRuntimeNodeRecord, 1U> nodes{invalid};
    const UiEditorRuntimeDocument document{Header(1U), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    std::array<UiEditorHierarchyRow, 1U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 1U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 1U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    SeedWorkflowSentinels(
        hierarchy_output,
        design_output,
        inspector_output,
        preview_output,
        staged_output,
        ledger_output);
    UiEditorDesignCommand command{};

    UiEditorDesignInspectorWorkflowResult result{};
    const UiEditorDesignWorkflowStatus status =
        BuildUiEditorDesignInspectorWorkflowSurface(
            WorkflowRequest(
                document,
                UiNodeId{1U},
                &preview_frame,
                command,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output),
            &result);
    if (status != UiEditorDesignWorkflowStatus::InvalidNode ||
        result.blocked_layer != UiEditorDesignWorkflowBlockedLayer::RuntimeUiDocument) {
        return Fail("ui editor workflow invalid component result mismatch");
    }

    if (!SentinelHierarchyUnchanged(hierarchy_output[0U]) ||
        !SentinelDesignUnchanged(design_output[0U]) ||
        !SentinelInspectorUnchanged(inspector_output[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelStagedNodeUnchanged(staged_output[0U]) ||
        !SentinelDesignLedgerUnchanged(ledger_output[0U])) {
        return Fail("ui editor workflow invalid component mutated outputs");
    }

    return 0;
}

int UiEditorWorkflowOutputCapacityDoesNotMutateOutputs() {
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    std::array<UiEditorHierarchyRow, 1U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 1U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 1U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    SeedWorkflowSentinels(
        hierarchy_output,
        design_output,
        inspector_output,
        preview_output,
        staged_output,
        ledger_output);
    UiEditorDesignCommand command{};

    UiEditorDesignInspectorWorkflowResult result{};
    const UiEditorDesignWorkflowStatus status =
        BuildUiEditorDesignInspectorWorkflowSurface(
            WorkflowRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                command,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output),
            &result);
    if (status != UiEditorDesignWorkflowStatus::OutputCapacityExceeded ||
        result.blocked_layer != UiEditorDesignWorkflowBlockedLayer::Output) {
        return Fail("ui editor workflow capacity result mismatch");
    }

    if (result.surface.runtime_node_count != 2U ||
        result.surface.hierarchy_row_count != 2U ||
        result.surface.preview_feedback_count != 1U ||
        result.hierarchy_row_count != 2U ||
        result.design_surface_row_count != 2U ||
        result.inspector_field_count != 7U ||
        result.preview_feedback_count != 1U ||
        result.staged_node_count != 2U ||
        result.command_ledger_count != 1U) {
        return Fail("ui editor workflow capacity required counts mismatch");
    }

    if (!SentinelHierarchyUnchanged(hierarchy_output[0U]) ||
        !SentinelDesignUnchanged(design_output[0U]) ||
        !SentinelInspectorUnchanged(inspector_output[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelStagedNodeUnchanged(staged_output[0U]) ||
        !SentinelDesignLedgerUnchanged(ledger_output[0U])) {
        return Fail("ui editor workflow capacity mutated outputs");
    }

    return 0;
}

int UiEditorRuntimePreviewWorkflowBuildsStyleTemplateStateRows() {
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    const std::array<UiEditorStyleTemplateStateRecord, 1U> style_records{
        ButtonStyleState()};
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    std::array<UiEditorRuntimePreviewStyleTemplateStateRow, 1U>
        runtime_preview_output{};
    std::array<UiEditorStyleTemplateStateLedgerRecord, 1U>
        style_ledger_output{};
    UiEditorDesignCommand design_command{};
    design_command.kind = UiEditorDesignCommandKind::SetEnabled;
    design_command.bool_value = false;
    design_command.command_sequence = 17U;
    UiEditorStyleTemplateStateCommand style_command{};
    style_command.kind = UiEditorStyleTemplateStateCommandKind::SetInteractionState;
    style_command.command_sequence = 18U;
    style_command.state_revision = 8U;
    style_command.hovered = true;
    style_command.focused = true;
    style_command.pressed = true;
    style_command.disabled = false;

    UiEditorRuntimePreviewWorkflowResult result{};
    const UiEditorRuntimePreviewWorkflowStatus status =
        BuildUiEditorRuntimePreviewStyleTemplateStateWorkflow(
            RuntimePreviewWorkflowRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                design_command,
                style_command,
                style_records,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output,
                runtime_preview_output,
                style_ledger_output),
            &result);
    if (status != UiEditorRuntimePreviewWorkflowStatus::Success ||
        !result.Succeeded()) {
        return Fail("ui editor runtime preview workflow failed");
    }

    if (result.hierarchy_row_count != 2U ||
        result.design_surface_row_count != 2U ||
        result.inspector_field_count != 7U ||
        result.preview_feedback_count != 1U ||
        result.staged_node_count != 2U ||
        result.command_ledger_count != 1U ||
        result.runtime_preview_row_count != 1U ||
        result.style_template_state_ledger_count != 1U ||
        !result.consumed_runtime_ui_document ||
        !result.consumed_preview_host_feedback ||
        !result.consumed_style_template_state ||
        !result.built_design_surface ||
        !result.built_engine_runtime_preview ||
        !result.emitted_runtime_preview_row ||
        !result.emitted_style_template_state_ledger ||
        !result.design_command_applied ||
        !result.style_template_state_command_applied ||
        result.mutated_runtime_data ||
        result.opened_native_window ||
        result.used_forbidden_preview_path) {
        return Fail("ui editor runtime preview counters mismatch");
    }

    if (staged_output[1U].enabled ||
        ledger_output[0U].command_kind != UiEditorDesignCommandKind::SetEnabled ||
        ledger_output[0U].command_sequence != 17U ||
        !ledger_output[0U].before_enabled ||
        ledger_output[0U].after_enabled) {
        return Fail("ui editor runtime preview design command mismatch");
    }

    if (runtime_preview_output[0U].document_id != DOCUMENT_ID ||
        runtime_preview_output[0U].node_id.value != 2U ||
        runtime_preview_output[0U].component_kind != UiEditorComponentKind::Button ||
        runtime_preview_output[0U].style_key != 810U ||
        runtime_preview_output[0U].theme_key != 875U ||
        runtime_preview_output[0U].template_key != 920U ||
        runtime_preview_output[0U].state_revision != 8U ||
        runtime_preview_output[0U].preview_frame_id != 71U ||
        runtime_preview_output[0U].preview_status != PreviewHostStatus::Success ||
        !runtime_preview_output[0U].selected ||
        !runtime_preview_output[0U].hovered ||
        !runtime_preview_output[0U].focused ||
        !runtime_preview_output[0U].pressed ||
        runtime_preview_output[0U].disabled ||
        !runtime_preview_output[0U].runtime_state_valid ||
        !runtime_preview_output[0U].style_resolved ||
        !runtime_preview_output[0U].template_instanced ||
        !runtime_preview_output[0U].engine_runtime_preview ||
        !runtime_preview_output[0U].preview_feedback_available ||
        !runtime_preview_output[0U].editable) {
        return Fail("ui editor runtime preview row mismatch");
    }

    if (style_ledger_output[0U].command_kind !=
            UiEditorStyleTemplateStateCommandKind::SetInteractionState ||
        style_ledger_output[0U].command_sequence != 18U ||
        style_ledger_output[0U].before_theme_key != 875U ||
        style_ledger_output[0U].after_theme_key != 875U ||
        style_ledger_output[0U].before_state_revision != 7U ||
        style_ledger_output[0U].after_state_revision != 8U ||
        style_ledger_output[0U].before_focused ||
        !style_ledger_output[0U].after_focused ||
        style_ledger_output[0U].before_pressed ||
        !style_ledger_output[0U].after_pressed ||
        !style_ledger_output[0U].staged_style_template_state_update ||
        !style_ledger_output[0U].command_applied) {
        return Fail("ui editor runtime preview style ledger mismatch");
    }

    return 0;
}

int UiEditorRuntimePreviewWorkflowMissingPreviewFeedbackDoesNotMutateOutputs() {
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const std::array<UiEditorStyleTemplateStateRecord, 1U> style_records{
        ButtonStyleState()};
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    std::array<UiEditorRuntimePreviewStyleTemplateStateRow, 1U>
        runtime_preview_output{};
    std::array<UiEditorStyleTemplateStateLedgerRecord, 1U>
        style_ledger_output{};
    SeedRuntimePreviewSentinels(
        hierarchy_output,
        design_output,
        inspector_output,
        preview_output,
        staged_output,
        ledger_output,
        runtime_preview_output,
        style_ledger_output);
    UiEditorDesignCommand design_command{};
    UiEditorStyleTemplateStateCommand style_command{};
    style_command.kind = UiEditorStyleTemplateStateCommandKind::SetStyleKey;
    style_command.style_key = 811U;

    UiEditorRuntimePreviewWorkflowResult result{};
    const UiEditorRuntimePreviewWorkflowStatus status =
        BuildUiEditorRuntimePreviewStyleTemplateStateWorkflow(
            RuntimePreviewWorkflowRequest(
                document,
                UiNodeId{2U},
                nullptr,
                design_command,
                style_command,
                style_records,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output,
                runtime_preview_output,
                style_ledger_output),
            &result);
    if (status != UiEditorRuntimePreviewWorkflowStatus::PreviewFeedbackMissing ||
        result.blocked_layer !=
            UiEditorRuntimePreviewWorkflowBlockedLayer::PreviewHostFeedback ||
        result.emitted_runtime_preview_row ||
        result.style_template_state_command_applied) {
        return Fail("ui editor runtime preview missing feedback result mismatch");
    }

    if (!SentinelHierarchyUnchanged(hierarchy_output[0U]) ||
        !SentinelDesignUnchanged(design_output[0U]) ||
        !SentinelInspectorUnchanged(inspector_output[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelStagedNodeUnchanged(staged_output[0U]) ||
        !SentinelDesignLedgerUnchanged(ledger_output[0U]) ||
        !SentinelRuntimePreviewUnchanged(runtime_preview_output[0U]) ||
        !SentinelStyleLedgerUnchanged(style_ledger_output[0U])) {
        return Fail("ui editor runtime preview missing feedback mutated output");
    }

    return 0;
}

int UiEditorRuntimePreviewWorkflowInvalidStyleTemplateStateDoesNotMutateOutputs() {
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    UiEditorStyleTemplateStateRecord invalid_style = ButtonStyleState();
    invalid_style.style_key = 0U;
    const std::array<UiEditorStyleTemplateStateRecord, 1U> style_records{
        invalid_style};
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    std::array<UiEditorRuntimePreviewStyleTemplateStateRow, 1U>
        runtime_preview_output{};
    std::array<UiEditorStyleTemplateStateLedgerRecord, 1U>
        style_ledger_output{};
    SeedRuntimePreviewSentinels(
        hierarchy_output,
        design_output,
        inspector_output,
        preview_output,
        staged_output,
        ledger_output,
        runtime_preview_output,
        style_ledger_output);
    UiEditorDesignCommand design_command{};
    UiEditorStyleTemplateStateCommand style_command{};

    UiEditorRuntimePreviewWorkflowResult result{};
    const UiEditorRuntimePreviewWorkflowStatus status =
        BuildUiEditorRuntimePreviewStyleTemplateStateWorkflow(
            RuntimePreviewWorkflowRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                design_command,
                style_command,
                style_records,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output,
                runtime_preview_output,
                style_ledger_output),
            &result);
    if (status != UiEditorRuntimePreviewWorkflowStatus::InvalidStyleTemplateState ||
        result.blocked_layer !=
            UiEditorRuntimePreviewWorkflowBlockedLayer::StyleTemplateState) {
        return Fail("ui editor runtime preview invalid style result mismatch");
    }

    if (!SentinelHierarchyUnchanged(hierarchy_output[0U]) ||
        !SentinelDesignUnchanged(design_output[0U]) ||
        !SentinelInspectorUnchanged(inspector_output[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelStagedNodeUnchanged(staged_output[0U]) ||
        !SentinelDesignLedgerUnchanged(ledger_output[0U]) ||
        !SentinelRuntimePreviewUnchanged(runtime_preview_output[0U]) ||
        !SentinelStyleLedgerUnchanged(style_ledger_output[0U])) {
        return Fail("ui editor runtime preview invalid style mutated output");
    }

    return 0;
}

int UiEditorRuntimePreviewWorkflowOutputCapacityDoesNotMutateOutputs() {
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    const std::array<UiEditorStyleTemplateStateRecord, 1U> style_records{
        ButtonStyleState()};
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    std::array<UiEditorRuntimePreviewStyleTemplateStateRow, 1U>
        runtime_preview_output{};
    std::array<UiEditorStyleTemplateStateLedgerRecord, 1U>
        style_ledger_output{};
    std::array<UiEditorRuntimePreviewStyleTemplateStateRow, 0U>
        small_runtime_preview_output{};
    SeedRuntimePreviewSentinels(
        hierarchy_output,
        design_output,
        inspector_output,
        preview_output,
        staged_output,
        ledger_output,
        runtime_preview_output,
        style_ledger_output);
    UiEditorDesignCommand design_command{};
    UiEditorStyleTemplateStateCommand style_command{};

    UiEditorRuntimePreviewWorkflowResult result{};
    const UiEditorRuntimePreviewWorkflowStatus status =
        BuildUiEditorRuntimePreviewStyleTemplateStateWorkflow(
            RuntimePreviewWorkflowRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                design_command,
                style_command,
                style_records,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output,
                small_runtime_preview_output,
                style_ledger_output),
            &result);
    if (status != UiEditorRuntimePreviewWorkflowStatus::OutputCapacityExceeded ||
        result.blocked_layer != UiEditorRuntimePreviewWorkflowBlockedLayer::Output) {
        return Fail("ui editor runtime preview capacity result mismatch");
    }

    if (result.design_workflow.hierarchy_row_count != 2U ||
        result.design_workflow.design_surface_row_count != 2U ||
        result.design_workflow.inspector_field_count != 7U ||
        result.design_workflow.preview_feedback_count != 1U ||
        result.design_workflow.staged_node_count != 2U ||
        result.design_workflow.command_ledger_count != 1U ||
        result.hierarchy_row_count != 2U ||
        result.design_surface_row_count != 2U ||
        result.inspector_field_count != 7U ||
        result.preview_feedback_count != 1U ||
        result.staged_node_count != 2U ||
        result.command_ledger_count != 1U ||
        result.runtime_preview_row_count != 1U ||
        result.style_template_state_ledger_count != 1U ||
        result.style_template_state_record_count != 1U) {
        return Fail("ui editor runtime preview capacity required counts mismatch");
    }

    if (!SentinelHierarchyUnchanged(hierarchy_output[0U]) ||
        !SentinelDesignUnchanged(design_output[0U]) ||
        !SentinelInspectorUnchanged(inspector_output[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelStagedNodeUnchanged(staged_output[0U]) ||
        !SentinelDesignLedgerUnchanged(ledger_output[0U]) ||
        !SentinelRuntimePreviewUnchanged(runtime_preview_output[0U]) ||
        !SentinelStyleLedgerUnchanged(style_ledger_output[0U])) {
        return Fail("ui editor runtime preview capacity mutated output");
    }

    return 0;
}

int UiEditorStyleThemeTemplateSerializationWorkflowRoundTripsThroughFileVfs() {
    const std::filesystem::path root =
        SerializationFixtureRoot() / "RoundTrip";
    std::filesystem::remove_all(root);
    MountTable mount_table;
    const FileStatus mount_status =
        mount_table.RegisterLooseMount(MountId(UI_EDITOR_TEST_MOUNT), root);
    if (mount_status != FileStatus::Success) {
        return Fail("ui editor serialization mount setup failed");
    }

    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    const std::array<UiEditorStyleTemplateStateRecord, 1U> style_records{
        ButtonStyleState()};
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    std::array<UiEditorRuntimePreviewStyleTemplateStateRow, 1U>
        runtime_preview_output{};
    std::array<UiEditorStyleTemplateStateLedgerRecord, 1U>
        style_ledger_output{};
    std::array<UiEditorStyleThemeTemplateSerializationRow, 1U>
        serialization_output{};
    UiEditorDesignCommand design_command{};
    UiEditorStyleTemplateStateCommand style_command{};
    style_command.kind = UiEditorStyleTemplateStateCommandKind::SetThemeKey;
    style_command.command_sequence = 27U;
    style_command.theme_key = 876U;

    UiEditorStyleThemeTemplateSerializationWorkflowResult result{};
    const UiEditorStyleThemeTemplateSerializationStatus status =
        BuildUiEditorStyleThemeTemplateSerializationWorkflow(
            SerializationWorkflowRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                design_command,
                style_command,
                style_records,
                &mount_table,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output,
                runtime_preview_output,
                style_ledger_output,
                serialization_output),
            &result);
    if (status != UiEditorStyleThemeTemplateSerializationStatus::Success ||
        !result.Succeeded()) {
        std::filesystem::remove_all(root);
        return Fail("ui editor serialization workflow failed");
    }

    if (result.blocked_layer !=
            UiEditorStyleThemeTemplateSerializationBlockedLayer::None ||
        result.serialize_write_status != SerializeStatus::Success ||
        result.serialize_read_status != SerializeStatus::Success ||
        result.file_write_status != FileStatus::Success ||
        result.file_read_status != FileStatus::Success ||
        result.serialization_row_count != 1U ||
        result.serialized_byte_count == 0U ||
        result.saved_byte_count != result.serialized_byte_count ||
        result.loaded_byte_count != result.serialized_byte_count ||
        !result.saved_through_file_vfs ||
        !result.loaded_through_file_vfs ||
        !result.serialized_runtime_ui_data ||
        !result.deserialized_runtime_ui_data ||
        !result.runtime_asset_boundary_preserved ||
        !result.ui_runtime_boundary_preserved ||
        !result.component_template_state_provenance_preserved ||
        !result.emitted_serialization_row ||
        result.mutated_runtime_data ||
        result.opened_native_window ||
        result.used_forbidden_preview_path) {
        std::filesystem::remove_all(root);
        return Fail("ui editor serialization counters mismatch");
    }

    if (runtime_preview_output[0U].theme_key != 876U ||
        style_ledger_output[0U].command_kind !=
            UiEditorStyleTemplateStateCommandKind::SetThemeKey ||
        style_ledger_output[0U].before_theme_key != 875U ||
        style_ledger_output[0U].after_theme_key != 876U ||
        style_records[0U].theme_key != 875U) {
        std::filesystem::remove_all(root);
        return Fail("ui editor serialization theme ledger mismatch");
    }

    const UiEditorStyleThemeTemplateSerializationRow &row =
        serialization_output[0U];
    if (row.document_id != DOCUMENT_ID ||
        row.node_id.value != 2U ||
        row.component_kind != UiEditorComponentKind::Button ||
        row.source_style_key != 810U ||
        row.source_theme_key != 876U ||
        row.source_template_key != 920U ||
        row.source_state_revision != 7U ||
        row.runtime_asset_style_key != 810U ||
        row.runtime_asset_theme_key != 876U ||
        row.runtime_asset_template_key != 920U ||
        row.runtime_asset_state_revision != 7U ||
        row.ui_runtime_style_key != 810U ||
        row.ui_runtime_theme_key != 876U ||
        row.ui_runtime_template_key != 920U ||
        row.ui_runtime_state_revision != 7U ||
        row.serialized_byte_count != result.serialized_byte_count ||
        row.saved_byte_count != result.saved_byte_count ||
        row.loaded_byte_count != result.loaded_byte_count ||
        !row.saved_through_file_vfs ||
        !row.loaded_through_file_vfs ||
        !row.serialized_runtime_ui_data ||
        !row.deserialized_runtime_ui_data ||
        !row.runtime_asset_boundary_preserved ||
        !row.ui_runtime_boundary_preserved ||
        !row.component_template_state_provenance_preserved) {
        std::filesystem::remove_all(root);
        return Fail("ui editor serialization row mismatch");
    }

    std::filesystem::remove_all(root);
    return 0;
}

int UiEditorStyleThemeTemplateSerializationWorkflowMissingFileVfsDoesNotMutateOutputs() {
    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    const std::array<UiEditorStyleTemplateStateRecord, 1U> style_records{
        ButtonStyleState()};
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    std::array<UiEditorRuntimePreviewStyleTemplateStateRow, 1U>
        runtime_preview_output{};
    std::array<UiEditorStyleTemplateStateLedgerRecord, 1U>
        style_ledger_output{};
    std::array<UiEditorStyleThemeTemplateSerializationRow, 1U>
        serialization_output{};
    SeedSerializationSentinels(
        hierarchy_output,
        design_output,
        inspector_output,
        preview_output,
        staged_output,
        ledger_output,
        runtime_preview_output,
        style_ledger_output,
        serialization_output);
    UiEditorDesignCommand design_command{};
    UiEditorStyleTemplateStateCommand style_command{};

    UiEditorStyleThemeTemplateSerializationWorkflowResult result{};
    const UiEditorStyleThemeTemplateSerializationStatus status =
        BuildUiEditorStyleThemeTemplateSerializationWorkflow(
            SerializationWorkflowRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                design_command,
                style_command,
                style_records,
                nullptr,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output,
                runtime_preview_output,
                style_ledger_output,
                serialization_output),
            &result);
    if (status != UiEditorStyleThemeTemplateSerializationStatus::FileVfsUnavailable ||
        result.blocked_layer !=
            UiEditorStyleThemeTemplateSerializationBlockedLayer::FileVfs) {
        return Fail("ui editor serialization missing file result mismatch");
    }

    if (!SentinelHierarchyUnchanged(hierarchy_output[0U]) ||
        !SentinelDesignUnchanged(design_output[0U]) ||
        !SentinelInspectorUnchanged(inspector_output[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelStagedNodeUnchanged(staged_output[0U]) ||
        !SentinelDesignLedgerUnchanged(ledger_output[0U]) ||
        !SentinelRuntimePreviewUnchanged(runtime_preview_output[0U]) ||
        !SentinelStyleLedgerUnchanged(style_ledger_output[0U]) ||
        !SentinelSerializationUnchanged(serialization_output[0U])) {
        return Fail("ui editor serialization missing file mutated output");
    }

    return 0;
}

int UiEditorStyleThemeTemplateSerializationWorkflowOutputCapacityDoesNotMutateOutputs() {
    const std::filesystem::path root =
        SerializationFixtureRoot() / "Capacity";
    std::filesystem::remove_all(root);
    MountTable mount_table;
    const FileStatus mount_status =
        mount_table.RegisterLooseMount(MountId(UI_EDITOR_TEST_MOUNT), root);
    if (mount_status != FileStatus::Success) {
        return Fail("ui editor serialization capacity mount setup failed");
    }

    const std::array<UiEditorRuntimeNodeRecord, 2U> nodes{RootNode(), ChildNode()};
    const UiEditorRuntimeDocument document{Header(), nodes};
    const PreviewHostFrameResult preview_frame = PreviewFrame();
    const std::array<UiEditorStyleTemplateStateRecord, 1U> style_records{
        ButtonStyleState()};
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    std::array<UiEditorRuntimePreviewStyleTemplateStateRow, 1U>
        runtime_preview_output{};
    std::array<UiEditorStyleTemplateStateLedgerRecord, 1U>
        style_ledger_output{};
    std::array<UiEditorStyleThemeTemplateSerializationRow, 1U>
        serialization_output{};
    std::array<UiEditorStyleThemeTemplateSerializationRow, 0U>
        small_serialization_output{};
    SeedSerializationSentinels(
        hierarchy_output,
        design_output,
        inspector_output,
        preview_output,
        staged_output,
        ledger_output,
        runtime_preview_output,
        style_ledger_output,
        serialization_output);
    UiEditorDesignCommand design_command{};
    UiEditorStyleTemplateStateCommand style_command{};

    UiEditorStyleThemeTemplateSerializationWorkflowResult result{};
    const UiEditorStyleThemeTemplateSerializationStatus status =
        BuildUiEditorStyleThemeTemplateSerializationWorkflow(
            SerializationWorkflowRequest(
                document,
                UiNodeId{2U},
                &preview_frame,
                design_command,
                style_command,
                style_records,
                &mount_table,
                hierarchy_output,
                design_output,
                inspector_output,
                preview_output,
                staged_output,
                ledger_output,
                runtime_preview_output,
                style_ledger_output,
                small_serialization_output),
            &result);
    if (status !=
            UiEditorStyleThemeTemplateSerializationStatus::OutputCapacityExceeded ||
        result.blocked_layer !=
            UiEditorStyleThemeTemplateSerializationBlockedLayer::Output) {
        std::filesystem::remove_all(root);
        return Fail("ui editor serialization capacity result mismatch");
    }

    if (result.runtime_preview_workflow.hierarchy_row_count != 2U ||
        result.runtime_preview_workflow.design_surface_row_count != 2U ||
        result.runtime_preview_workflow.inspector_field_count != 7U ||
        result.runtime_preview_workflow.preview_feedback_count != 1U ||
        result.runtime_preview_workflow.staged_node_count != 2U ||
        result.runtime_preview_workflow.command_ledger_count != 1U ||
        result.runtime_preview_workflow.runtime_preview_row_count != 1U ||
        result.runtime_preview_workflow.style_template_state_ledger_count != 1U ||
        result.hierarchy_row_count != 2U ||
        result.design_surface_row_count != 2U ||
        result.inspector_field_count != 7U ||
        result.preview_feedback_count != 1U ||
        result.staged_node_count != 2U ||
        result.command_ledger_count != 1U ||
        result.runtime_preview_row_count != 1U ||
        result.style_template_state_ledger_count != 1U ||
        result.serialization_row_count != 1U) {
        std::filesystem::remove_all(root);
        return Fail("ui editor serialization capacity required counts mismatch");
    }

    if (!SentinelHierarchyUnchanged(hierarchy_output[0U]) ||
        !SentinelDesignUnchanged(design_output[0U]) ||
        !SentinelInspectorUnchanged(inspector_output[0U]) ||
        !SentinelPreviewUnchanged(preview_output[0U]) ||
        !SentinelStagedNodeUnchanged(staged_output[0U]) ||
        !SentinelDesignLedgerUnchanged(ledger_output[0U]) ||
        !SentinelRuntimePreviewUnchanged(runtime_preview_output[0U]) ||
        !SentinelStyleLedgerUnchanged(style_ledger_output[0U]) ||
        !SentinelSerializationUnchanged(serialization_output[0U])) {
        std::filesystem::remove_all(root);
        return Fail("ui editor serialization capacity mutated output");
    }

    std::filesystem::remove_all(root);
    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_DOCUMENT) {
        return UiEditorSurfaceBuildsRuntimeDocumentHierarchyRows();
    }

    if (name == TEST_PREVIEW) {
        return UiEditorSurfaceConsumesPreviewHostFrameFeedback();
    }

    if (name == TEST_NO_MUTATION) {
        return UiEditorSurfaceRejectsMissingPreviewFeedbackWithoutMutation();
    }

    if (name == TEST_WORKFLOW_SELECTION) {
        return UiEditorWorkflowNodeSelectionBuildsDesignSurface();
    }

    if (name == TEST_WORKFLOW_LAYOUT_PREVIEW) {
        return UiEditorWorkflowMultiComponentLayoutPreviewRecordsTextAndSlider();
    }

    if (name == TEST_WORKFLOW_INSPECTOR) {
        return UiEditorWorkflowInspectorRowsExposeComponentState();
    }

    if (name == TEST_WORKFLOW_EDIT) {
        return UiEditorWorkflowComponentEditStagesDocumentAndLedger();
    }

    if (name == TEST_WORKFLOW_MISSING_PREVIEW) {
        return UiEditorWorkflowMissingPreviewFeedbackDoesNotMutateOutputs();
    }

    if (name == TEST_WORKFLOW_INVALID_COMPONENT) {
        return UiEditorWorkflowInvalidComponentDoesNotMutateOutputs();
    }

    if (name == TEST_WORKFLOW_CAPACITY) {
        return UiEditorWorkflowOutputCapacityDoesNotMutateOutputs();
    }

    if (name == TEST_RUNTIME_PREVIEW) {
        return UiEditorRuntimePreviewWorkflowBuildsStyleTemplateStateRows();
    }

    if (name == TEST_RUNTIME_PREVIEW_MISSING_PREVIEW) {
        return UiEditorRuntimePreviewWorkflowMissingPreviewFeedbackDoesNotMutateOutputs();
    }

    if (name == TEST_RUNTIME_PREVIEW_INVALID_STYLE) {
        return UiEditorRuntimePreviewWorkflowInvalidStyleTemplateStateDoesNotMutateOutputs();
    }

    if (name == TEST_RUNTIME_PREVIEW_CAPACITY) {
        return UiEditorRuntimePreviewWorkflowOutputCapacityDoesNotMutateOutputs();
    }

    if (name == TEST_SERIALIZATION) {
        return UiEditorStyleThemeTemplateSerializationWorkflowRoundTripsThroughFileVfs();
    }

    if (name == TEST_SERIALIZATION_MISSING_FILE) {
        return UiEditorStyleThemeTemplateSerializationWorkflowMissingFileVfsDoesNotMutateOutputs();
    }

    if (name == TEST_SERIALIZATION_CAPACITY) {
        return UiEditorStyleThemeTemplateSerializationWorkflowOutputCapacityDoesNotMutateOutputs();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
