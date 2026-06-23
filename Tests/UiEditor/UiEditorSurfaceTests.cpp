// Module: Tests UiEditor
// File: Tests/UiEditor/UiEditorSurfaceTests.cpp

#include <array>
#include <cmath>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/PreviewHost/PreviewHost.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiEditor/UiEditorSurface.h"

using yuengine::previewhost::PreviewHostFrameFormat;
using yuengine::previewhost::PreviewHostFrameResult;
using yuengine::previewhost::PreviewHostStatus;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UiRectTransform;
using yuengine::uieditor::BuildUiEditorRuntimeDocumentSurface;
using yuengine::uieditor::UiEditorComponentKind;
using yuengine::uieditor::UiEditorHierarchyRow;
using yuengine::uieditor::UiEditorPreviewFeedbackRecord;
using yuengine::uieditor::UiEditorRuntimeDocument;
using yuengine::uieditor::UiEditorRuntimeDocumentHeader;
using yuengine::uieditor::UiEditorRuntimeDocumentSurfaceRequest;
using yuengine::uieditor::UiEditorRuntimeDocumentSurfaceResult;
using yuengine::uieditor::UiEditorRuntimeNodeRecord;
using yuengine::uieditor::UiEditorSurfaceBlockedLayer;
using yuengine::uieditor::UiEditorSurfaceStatus;

namespace {
constexpr const char *TEST_DOCUMENT =
    "UiEditorSurface_BuildsRuntimeDocumentHierarchyRows";
constexpr const char *TEST_PREVIEW =
    "UiEditorSurface_ConsumesPreviewHostFrameFeedback";
constexpr const char *TEST_NO_MUTATION =
    "UiEditorSurface_RejectsMissingPreviewFeedbackWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t DOCUMENT_ID = 5101U;
constexpr float TOLERANCE = 0.0001F;

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

bool SentinelHierarchyUnchanged(const UiEditorHierarchyRow &row) {
    return row.document_id == 9001U && row.node_id.value == 9002U && !row.selected;
}

bool SentinelPreviewUnchanged(const UiEditorPreviewFeedbackRecord &record) {
    return record.document_id == 9003U && !record.feedback_from_preview_host;
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

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
