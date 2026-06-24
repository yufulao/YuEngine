// 模块: Tests SceneEditor
// 文件: Tests/SceneEditor/SceneEditorSurfaceTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/PreviewHost/PreviewHost.h"
#include "YuEngine/ResourceBrowser/ResourceBrowserSurface.h"
#include "YuEngine/SceneEditor/SceneEditorSurface.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldSceneAuthoringDocument.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldTransformState.h"

namespace {
using yuengine::asset::AssetHandle;
using yuengine::asset::AssetTypeId;
using yuengine::object::ObjectHandle;
using yuengine::sceneeditor::ApplySceneEditorTransformCommand;
using yuengine::previewhost::PreviewHostEditorViewportInteractionResult;
using yuengine::previewhost::PreviewHostStatus;
using yuengine::previewhost::PreviewHostViewportSessionResult;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceTypeId;
using yuengine::resourcebrowser::ResourceBrowserDependencyState;
using yuengine::resourcebrowser::ResourceBrowserImportSettings;
using yuengine::resourcebrowser::ResourceBrowserSurfaceDocumentKind;
using yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState;
using yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionState;
using yuengine::sceneeditor::BuildSceneEditorNativeSurface;
using yuengine::sceneeditor::BuildSceneEditorUsableWorkflowSurface;
using yuengine::sceneeditor::SceneEditorHierarchyRow;
using yuengine::sceneeditor::SceneEditorInspectorRow;
using yuengine::sceneeditor::SceneEditorSurfaceRequest;
using yuengine::sceneeditor::SceneEditorSurfaceResult;
using yuengine::sceneeditor::SceneEditorSurfaceStatus;
using yuengine::sceneeditor::SceneEditorTransformCommandBlockedLayer;
using yuengine::sceneeditor::SceneEditorTransformCommandMode;
using yuengine::sceneeditor::SceneEditorTransformCommandRequest;
using yuengine::sceneeditor::SceneEditorTransformCommandResult;
using yuengine::sceneeditor::SceneEditorTransformCommandStatus;
using yuengine::sceneeditor::SceneEditorTransformLedgerRecord;
using yuengine::sceneeditor::SceneEditorWorkflowBlockedLayer;
using yuengine::sceneeditor::SceneEditorWorkflowLedgerRecord;
using yuengine::sceneeditor::SceneEditorWorkflowRequest;
using yuengine::sceneeditor::SceneEditorWorkflowResult;
using yuengine::sceneeditor::SceneEditorWorkflowStatus;
using yuengine::runtimeasset::RuntimeAssetDataStatus;
using yuengine::runtimeasset::RuntimeAssetFileKind;
using yuengine::world::WorldComponentAttachmentSnapshotRecord;
using yuengine::world::WorldComponentResourceBindingSnapshotRecord;
using yuengine::world::WorldComponentSlotId;
using yuengine::world::WorldComponentTypeId;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldSceneAuthoringDependencyRecord;
using yuengine::world::WorldSceneAuthoringDocument;
using yuengine::world::WorldSceneAuthoringDocumentStatus;
using yuengine::world::WorldSceneEditorSidecarExportPolicy;
using yuengine::world::WorldSceneEditorSidecarKind;
using yuengine::world::WorldSceneEditorSidecarRecord;
using yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;
using yuengine::world::WorldTransformState;

constexpr const char *TEST_BUILDS_SURFACE =
    "SceneEditorSurface_BuildsHierarchyAndInspectorFromAuthoringDocument";
constexpr const char *TEST_REJECTS_SMALL_HIERARCHY =
    "SceneEditorSurface_RejectsSmallHierarchyOutputWithoutMutation";
constexpr const char *TEST_REJECTS_INVALID_DOCUMENT =
    "SceneEditorSurface_RejectsInvalidAuthoringDocumentWithoutMutation";
constexpr const char *TEST_SELECTION_REQUIRED =
    "SceneEditorSurface_ReportsSelectionRequiredWithoutMutation";
constexpr const char *TEST_FIELD_GROUPING =
    "SceneEditorSurface_GroupsRuntimeAndEditorOnlyInspectorFields";
constexpr const char *TEST_BOUNDARY_FLAGS =
    "SceneEditorSurface_RemainsEditorDataOnlyWithoutRuntimeMutation";
constexpr const char *TEST_TRANSFORM_APPLY =
    "SceneEditorTransformCommand_AppliesSelectedTransformAndWritesUndoLedger";
constexpr const char *TEST_TRANSFORM_UNDO_REDO =
    "SceneEditorTransformCommand_UndoRedoReplaysLedgerDeterministically";
constexpr const char *TEST_TRANSFORM_INVALID_SELECTION =
    "SceneEditorTransformCommand_RejectsInvalidSelectionWithoutMutation";
constexpr const char *TEST_WORKFLOW_SELECTION_VIEWPORT =
    "SceneEditorWorkflow_SelectionFeedsViewportAndTransformCommand";
constexpr const char *TEST_WORKFLOW_INSPECTOR_TRANSFORM =
    "SceneEditorWorkflow_InspectorSelectionAppliesTransformLedger";
constexpr const char *TEST_WORKFLOW_UNDO_REDO =
    "SceneEditorWorkflow_UndoRedoLedgerReplay";
constexpr const char *TEST_WORKFLOW_BLOCKED_DEPENDENCY =
    "SceneEditorWorkflow_BlockedDependencyDoesNotMutateOutputs";

int Fail(const char *message) {
    std::fprintf(stderr, "%s\n", message);
    return 1;
}

WorldObjectId ObjectId(std::uint32_t value) {
    return WorldObjectId{value};
}

ObjectHandle MakeObjectHandle(std::uint32_t slot, std::uint32_t generation) {
    ObjectHandle handle{};
    handle.slot = slot;
    handle.generation = generation;
    return handle;
}

ResourceHandle MakeResourceHandle(std::uint32_t slot, std::uint32_t generation) {
    ResourceHandle handle{};
    handle.slot = slot;
    handle.generation = generation;
    return handle;
}

AssetHandle MakeAssetHandle(std::uint32_t slot, std::uint32_t generation) {
    AssetHandle handle{};
    handle.slot = slot;
    handle.generation = generation;
    return handle;
}

WorldTransformState Transform(float seed) {
    WorldTransformState transform{};
    transform.translation_x = seed;
    transform.translation_y = seed + 1.0F;
    transform.translation_z = seed + 2.0F;
    transform.rotation_w = 1.0F;
    transform.scale_x = 1.0F + (seed * 0.001F);
    transform.scale_y = 1.0F + (seed * 0.001F);
    transform.scale_z = 1.0F + (seed * 0.001F);
    return transform;
}

WorldSceneAuthoringDocument MakeDocument(
    const WorldSceneObjectTransformRestoreIdentityRecord *identities,
    std::uint32_t identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *transforms,
    std::uint32_t transform_count,
    const WorldComponentAttachmentSnapshotRecord *attachments,
    std::uint32_t attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *bindings,
    std::uint32_t binding_count,
    const WorldSceneAuthoringDependencyRecord *dependencies,
    std::uint32_t dependency_count,
    const WorldSceneEditorSidecarRecord *sidecars,
    std::uint32_t sidecar_count) {
    WorldSceneAuthoringDocument document{};
    document.header.scene_document_id = 0x5001U;
    document.header.deterministic_document_hash = 0xABCDU;
    document.header.identity_record_count = identity_count;
    document.header.transform_record_count = transform_count;
    document.header.attachment_record_count = attachment_count;
    document.header.binding_record_count = binding_count;
    document.header.dependency_record_count = dependency_count;
    document.header.sidecar_record_count = sidecar_count;
    document.identity_records = identities;
    document.transform_records = transforms;
    document.attachment_records = attachments;
    document.binding_records = bindings;
    document.dependency_records = dependencies;
    document.sidecar_records = sidecars;
    return document;
}

SceneEditorSurfaceRequest MakeRequest(
    const WorldSceneAuthoringDocument *document,
    SceneEditorHierarchyRow *hierarchy_rows,
    std::uint32_t hierarchy_capacity,
    SceneEditorInspectorRow *inspector_rows,
    std::uint32_t inspector_capacity,
    bool require_selection) {
    SceneEditorSurfaceRequest request{};
    request.document = document;
    request.hierarchy_rows =
        std::span<SceneEditorHierarchyRow>(hierarchy_rows, hierarchy_capacity);
    request.inspector_rows =
        std::span<SceneEditorInspectorRow>(inspector_rows, inspector_capacity);
    request.require_selection = require_selection;
    return request;
}

WorldSceneEditorSidecarRecord SelectionSidecar(WorldObjectId world_object_id) {
    return WorldSceneEditorSidecarRecord{
        WorldSceneEditorSidecarKind::Selection,
        WorldSceneEditorSidecarExportPolicy::EditorOnly,
        world_object_id,
        0U,
        1U};
}

WorldSceneEditorSidecarRecord FoldoutSidecar(
    WorldObjectId world_object_id,
    std::uint64_t value) {
    return WorldSceneEditorSidecarRecord{
        WorldSceneEditorSidecarKind::Foldout,
        WorldSceneEditorSidecarExportPolicy::EditorOnly,
        world_object_id,
        0U,
        value};
}

bool SentinelHierarchyUnchanged(const SceneEditorHierarchyRow &row) {
    return row.world_object_id.value == 900U &&
        row.object_handle.slot == 90U &&
        row.row_index == 77U;
}

bool SentinelInspectorUnchanged(const SceneEditorInspectorRow &row) {
    return row.world_object_id.value == 901U &&
        row.object_handle.slot == 91U &&
        row.component_count == 77U;
}

bool TransformMatches(
    const WorldTransformState &left,
    const WorldTransformState &right) {
    return left.translation_x == right.translation_x &&
        left.translation_y == right.translation_y &&
        left.translation_z == right.translation_z &&
        left.rotation_x == right.rotation_x &&
        left.rotation_y == right.rotation_y &&
        left.rotation_z == right.rotation_z &&
        left.rotation_w == right.rotation_w &&
        left.scale_x == right.scale_x &&
        left.scale_y == right.scale_y &&
        left.scale_z == right.scale_z;
}

bool SentinelTransformUnchanged(
    const WorldSceneObjectTransformRestoreTransformRecord &record) {
    return record.world_object_id.value == 990U &&
        record.transform_state.translation_x == 991.0F;
}

bool SentinelLedgerUnchanged(const SceneEditorTransformLedgerRecord &record) {
    return record.world_object_id.value == 992U &&
        record.command_sequence == 993U &&
        !record.applied &&
        !record.undone &&
        !record.redone;
}

bool SentinelWorkflowLedgerUnchanged(const SceneEditorWorkflowLedgerRecord &record) {
    return record.selected_world_object_id.value == 994U &&
        record.transform_command_sequence == 995U &&
        !record.committed_workflow;
}

ResourceBrowserSurfaceSelectionState ReadyResourceSelection() {
    ResourceBrowserSurfaceSelectionState state{};
    state.selected_index = 0U;
    state.import_settings.source_path = "/Game/Scene/ready.scene";
    state.import_settings.target_kind = RuntimeAssetFileKind::Scene;
    state.import_settings.resource_type = ResourceTypeId{51U};
    state.import_settings.asset_type = AssetTypeId{61U};
    state.import_settings.stable_id = 0xAABBCCDDU;
    state.import_settings.expected_source_hash = 0x1010U;
    state.setting_validation =
        yuengine::resourcebrowser::ResourceBrowserSurfaceSettingValidationCode::None;
    state.preview_state = ResourceBrowserSurfacePreviewState::Eligible;
    state.preview_document_kind = ResourceBrowserSurfaceDocumentKind::Scene;
    state.validation_status = RuntimeAssetDataStatus::Success;
    state.dependency_state = ResourceBrowserDependencyState::Ready;
    state.resource = MakeResourceHandle(21U, 31U);
    state.asset = MakeAssetHandle(22U, 32U);
    state.stable_id = state.import_settings.stable_id;
    state.source_hash = state.import_settings.expected_source_hash;
    state.selected = true;
    state.import_settings_valid = true;
    state.preview_eligible = true;
    state.resource_asset_mapping_preserved = true;
    return state;
}

PreviewHostViewportSessionResult ReadyViewportSession(std::uint32_t selected_entity_index) {
    PreviewHostViewportSessionResult result{};
    result.status = PreviewHostStatus::Success;
    result.frame.status = PreviewHostStatus::Success;
    result.frame.frame.frame_id = 80U;
    result.frame.submitted_entity_count = 2U;
    result.camera_state.camera_id = 1U;
    result.camera_state.orbit_angle_radians = 0.25F;
    result.camera_state.orbit_radius = 5.0F;
    result.camera_state.orbit_height = 1.0F;
    result.resource_browser_preview_state = ResourceBrowserSurfacePreviewState::Eligible;
    result.resource_browser_document_kind = ResourceBrowserSurfaceDocumentKind::Scene;
    result.selected_entity_index = selected_entity_index;
    result.viewport_width = 1280U;
    result.viewport_height = 720U;
    result.consumed_viewport_controls = true;
    result.consumed_resource_browser_selection = true;
    result.resource_browser_preview_eligible = true;
    result.resource_asset_mapping_preserved = true;
    result.selected_entity_available = true;
    result.built_frame = true;
    result.emitted_hit_feedback = true;
    result.emitted_selection_feedback = true;
    result.emitted_transform_feedback = true;
    return result;
}

PreviewHostEditorViewportInteractionResult ReadyViewportInteraction(
    WorldObjectId world_object_id,
    std::uint32_t selected_entity_index) {
    PreviewHostEditorViewportInteractionResult result{};
    result.status = PreviewHostStatus::Success;
    result.selected_entity_index = selected_entity_index;
    result.selected_world_object_id = world_object_id;
    result.hit_record_count = 1U;
    result.selection_record_count = 1U;
    result.transform_feedback_count = 1U;
    result.ledger_record_count = 1U;
    result.consumed_viewport_session = true;
    result.consumed_engine_viewport_frame = true;
    result.processed_selection_command = true;
    result.emitted_hit_feedback = true;
    result.emitted_selection_feedback = true;
    result.emitted_transform_feedback = true;
    result.emitted_interaction_ledger = true;
    return result;
}

SceneEditorWorkflowRequest MakeWorkflowRequest(
    const WorldSceneAuthoringDocument *document,
    const ResourceBrowserSurfaceSelectionState *selection,
    const PreviewHostViewportSessionResult *session,
    const PreviewHostEditorViewportInteractionResult *interaction,
    WorldTransformState requested_transform,
    const SceneEditorTransformLedgerRecord *history_record,
    SceneEditorTransformCommandMode mode,
    std::span<SceneEditorHierarchyRow> hierarchy_rows,
    std::span<SceneEditorInspectorRow> inspector_rows,
    std::span<WorldSceneObjectTransformRestoreTransformRecord> transform_output,
    std::span<SceneEditorTransformLedgerRecord> transform_ledger_output,
    std::span<SceneEditorWorkflowLedgerRecord> workflow_ledger_output) {
    SceneEditorWorkflowRequest request{};
    request.document = document;
    request.resource_browser_selection = selection;
    request.viewport_session = session;
    request.viewport_interaction = interaction;
    request.requested_transform = requested_transform;
    request.history_record = history_record;
    request.transform_mode = mode;
    request.hierarchy_rows = hierarchy_rows;
    request.inspector_rows = inspector_rows;
    request.transform_output = transform_output;
    request.transform_ledger_output = transform_ledger_output;
    request.workflow_ledger_output = workflow_ledger_output;
    return request;
}

int SceneEditorSurfaceBuildsHierarchyAndInspectorFromAuthoringDocument() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)},
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(2U), MakeObjectHandle(2U, 10U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 2U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)},
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(2U), Transform(20.0F)}};
    std::array<WorldComponentAttachmentSnapshotRecord, 2U> attachments{
        WorldComponentAttachmentSnapshotRecord{ObjectId(1U), WorldComponentTypeId{7U}, WorldComponentSlotId{1U}},
        WorldComponentAttachmentSnapshotRecord{ObjectId(1U), WorldComponentTypeId{8U}, WorldComponentSlotId{2U}}};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> bindings{
        WorldComponentResourceBindingSnapshotRecord{
            ObjectId(1U),
            WorldComponentTypeId{7U},
            WorldComponentSlotId{1U},
            MakeResourceHandle(3U, 11U),
            ResourceTypeId{12U}}};
    std::array<WorldSceneAuthoringDependencyRecord, 1U> dependencies{
        WorldSceneAuthoringDependencyRecord{7001U, MakeResourceHandle(3U, 11U), ResourceTypeId{12U}}};
    std::array<WorldSceneEditorSidecarRecord, 2U> sidecars{
        SelectionSidecar(ObjectId(1U)),
        FoldoutSidecar(ObjectId(2U), 0U)};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        static_cast<std::uint32_t>(identities.size()),
        transforms.data(),
        static_cast<std::uint32_t>(transforms.size()),
        attachments.data(),
        static_cast<std::uint32_t>(attachments.size()),
        bindings.data(),
        static_cast<std::uint32_t>(bindings.size()),
        dependencies.data(),
        static_cast<std::uint32_t>(dependencies.size()),
        sidecars.data(),
        static_cast<std::uint32_t>(sidecars.size()));

    std::array<SceneEditorHierarchyRow, 2U> hierarchy_rows{};
    std::array<SceneEditorInspectorRow, 1U> inspector_rows{};
    SceneEditorSurfaceResult result{};
    const SceneEditorSurfaceStatus status = BuildSceneEditorNativeSurface(
        MakeRequest(
            &document,
            hierarchy_rows.data(),
            static_cast<std::uint32_t>(hierarchy_rows.size()),
            inspector_rows.data(),
            static_cast<std::uint32_t>(inspector_rows.size()),
            true),
        &result);

    if (status != SceneEditorSurfaceStatus::Success ||
        !result.Succeeded() ||
        result.hierarchy_row_count != 2U ||
        result.inspector_row_count != 1U ||
        result.selected_object_count != 1U ||
        result.folded_object_count != 1U ||
        result.component_attachment_count != 2U ||
        result.resource_binding_count != 1U ||
        !result.consumed_authoring_document ||
        !result.consumed_editor_sidecar ||
        result.exported_runtime_data ||
        result.opened_native_window ||
        result.used_preview_feedback) {
        return Fail("scene editor surface result did not expose expected data surface");
    }

    if (hierarchy_rows[0U].world_object_id.value != 1U ||
        hierarchy_rows[0U].component_count != 2U ||
        hierarchy_rows[0U].resource_binding_count != 1U ||
        !hierarchy_rows[0U].has_transform ||
        !hierarchy_rows[0U].visible ||
        !hierarchy_rows[0U].active ||
        !hierarchy_rows[0U].selected ||
        !hierarchy_rows[0U].expanded) {
        return Fail("first hierarchy row did not expose selected object summary");
    }

    if (hierarchy_rows[1U].world_object_id.value != 2U ||
        hierarchy_rows[1U].component_count != 0U ||
        hierarchy_rows[1U].resource_binding_count != 0U ||
        !hierarchy_rows[1U].has_transform ||
        !hierarchy_rows[1U].visible ||
        !hierarchy_rows[1U].active ||
        hierarchy_rows[1U].selected ||
        hierarchy_rows[1U].expanded) {
        return Fail("second hierarchy row did not preserve foldout sidecar state");
    }

    if (inspector_rows[0U].world_object_id.value != 1U ||
        !inspector_rows[0U].selected ||
        !inspector_rows[0U].has_transform ||
        !inspector_rows[0U].has_component_attachments ||
        !inspector_rows[0U].has_resource_bindings ||
        inspector_rows[0U].component_count != 2U ||
        inspector_rows[0U].resource_binding_count != 1U ||
        inspector_rows[0U].runtime_export_field_count != 5U ||
        inspector_rows[0U].editor_only_sidecar_field_count != 1U ||
        !inspector_rows[0U].separated_runtime_and_editor_fields ||
        inspector_rows[0U].transform.translation_x != 10.0F) {
        return Fail("inspector row did not expose selected object details");
    }

    return 0;
}

int SceneEditorSurfaceRejectsSmallHierarchyOutputWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)},
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(2U), MakeObjectHandle(2U, 10U)}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        2U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U);
    std::array<SceneEditorHierarchyRow, 1U> hierarchy_rows{
        SceneEditorHierarchyRow{ObjectId(900U), MakeObjectHandle(90U, 9U), 77U}};
    std::array<SceneEditorInspectorRow, 1U> inspector_rows{
        SceneEditorInspectorRow{ObjectId(901U), MakeObjectHandle(91U, 9U), {}, 77U}};

    SceneEditorSurfaceResult result{};
    const SceneEditorSurfaceStatus status = BuildSceneEditorNativeSurface(
        MakeRequest(&document, hierarchy_rows.data(), 1U, inspector_rows.data(), 1U, false),
        &result);
    if (status != SceneEditorSurfaceStatus::OutputCapacityExceeded ||
        result.hierarchy_row_count != 0U ||
        !SentinelHierarchyUnchanged(hierarchy_rows[0U]) ||
        !SentinelInspectorUnchanged(inspector_rows[0U])) {
        return Fail("small hierarchy output was not rejected atomically");
    }

    return 0;
}

int SceneEditorSurfaceRejectsInvalidAuthoringDocumentWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(0U), MakeObjectHandle(1U, 10U)}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U);
    std::array<SceneEditorHierarchyRow, 1U> hierarchy_rows{
        SceneEditorHierarchyRow{ObjectId(900U), MakeObjectHandle(90U, 9U), 77U}};
    std::array<SceneEditorInspectorRow, 1U> inspector_rows{
        SceneEditorInspectorRow{ObjectId(901U), MakeObjectHandle(91U, 9U), {}, 77U}};

    SceneEditorSurfaceResult result{};
    const SceneEditorSurfaceStatus status = BuildSceneEditorNativeSurface(
        MakeRequest(&document, hierarchy_rows.data(), 1U, inspector_rows.data(), 1U, false),
        &result);
    if (status != SceneEditorSurfaceStatus::InvalidAuthoringDocument ||
        result.authoring_status != WorldSceneAuthoringDocumentStatus::InvalidWorldObjectId ||
        !SentinelHierarchyUnchanged(hierarchy_rows[0U]) ||
        !SentinelInspectorUnchanged(inspector_rows[0U])) {
        return Fail("invalid authoring document mutated scene editor output");
    }

    return 0;
}

int SceneEditorSurfaceReportsSelectionRequiredWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U);
    std::array<SceneEditorHierarchyRow, 1U> hierarchy_rows{
        SceneEditorHierarchyRow{ObjectId(900U), MakeObjectHandle(90U, 9U), 77U}};
    std::array<SceneEditorInspectorRow, 1U> inspector_rows{
        SceneEditorInspectorRow{ObjectId(901U), MakeObjectHandle(91U, 9U), {}, 77U}};

    SceneEditorSurfaceResult result{};
    const SceneEditorSurfaceStatus status = BuildSceneEditorNativeSurface(
        MakeRequest(&document, hierarchy_rows.data(), 1U, inspector_rows.data(), 1U, true),
        &result);
    if (status != SceneEditorSurfaceStatus::SelectionRequired ||
        result.selected_object_count != 0U ||
        !SentinelHierarchyUnchanged(hierarchy_rows[0U]) ||
        !SentinelInspectorUnchanged(inspector_rows[0U])) {
        return Fail("required selection failure mutated scene editor rows");
    }

    return 0;
}

int SceneEditorSurfaceGroupsRuntimeAndEditorOnlyInspectorFields() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)}};
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> attachments{
        WorldComponentAttachmentSnapshotRecord{ObjectId(1U), WorldComponentTypeId{7U}, WorldComponentSlotId{1U}}};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> bindings{
        WorldComponentResourceBindingSnapshotRecord{
            ObjectId(1U),
            WorldComponentTypeId{7U},
            WorldComponentSlotId{1U},
            MakeResourceHandle(3U, 11U),
            ResourceTypeId{12U}}};
    std::array<WorldSceneAuthoringDependencyRecord, 1U> dependencies{
        WorldSceneAuthoringDependencyRecord{7001U, MakeResourceHandle(3U, 11U), ResourceTypeId{12U}}};
    std::array<WorldSceneEditorSidecarRecord, 2U> sidecars{
        SelectionSidecar(ObjectId(1U)),
        FoldoutSidecar(ObjectId(1U), 1U)};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        transforms.data(),
        1U,
        attachments.data(),
        1U,
        bindings.data(),
        1U,
        dependencies.data(),
        1U,
        sidecars.data(),
        2U);
    std::array<SceneEditorHierarchyRow, 1U> hierarchy_rows{};
    std::array<SceneEditorInspectorRow, 1U> inspector_rows{};

    SceneEditorSurfaceResult result{};
    const SceneEditorSurfaceStatus status = BuildSceneEditorNativeSurface(
        MakeRequest(&document, hierarchy_rows.data(), 1U, inspector_rows.data(), 1U, true),
        &result);
    if (status != SceneEditorSurfaceStatus::Success ||
        inspector_rows[0U].runtime_export_field_count != 4U ||
        inspector_rows[0U].editor_only_sidecar_field_count != 2U ||
        !inspector_rows[0U].separated_runtime_and_editor_fields ||
        !hierarchy_rows[0U].visible ||
        !hierarchy_rows[0U].active) {
        return Fail("inspector field grouping did not separate runtime and sidecar data");
    }

    return 0;
}

int SceneEditorSurfaceRemainsEditorDataOnlyWithoutRuntimeMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)}};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{
        SelectionSidecar(ObjectId(1U))};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        sidecars.data(),
        1U);
    std::array<SceneEditorHierarchyRow, 1U> hierarchy_rows{};
    std::array<SceneEditorInspectorRow, 1U> inspector_rows{};

    SceneEditorSurfaceResult result{};
    const SceneEditorSurfaceStatus status = BuildSceneEditorNativeSurface(
        MakeRequest(&document, hierarchy_rows.data(), 1U, inspector_rows.data(), 1U, true),
        &result);
    if (status != SceneEditorSurfaceStatus::Success ||
        result.exported_runtime_data ||
        result.mutated_runtime_data ||
        result.opened_native_window ||
        result.used_preview_feedback ||
        !result.consumed_editor_sidecar ||
        !hierarchy_rows[0U].selected ||
        !inspector_rows[0U].selected) {
        return Fail("scene editor surface crossed runtime or native window boundary");
    }

    return 0;
}

int SceneEditorTransformCommandAppliesSelectedTransformAndWritesUndoLedger() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)},
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(2U), MakeObjectHandle(2U, 10U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 2U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)},
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(2U), Transform(20.0F)}};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{
        SelectionSidecar(ObjectId(2U))};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        2U,
        transforms.data(),
        2U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        sidecars.data(),
        1U);

    std::array<WorldSceneObjectTransformRestoreTransformRecord, 2U> transform_output{};
    std::array<SceneEditorTransformLedgerRecord, 1U> ledger_output{};
    const WorldTransformState requested = Transform(200.0F);
    SceneEditorTransformCommandRequest request{};
    request.document = &document;
    request.selected_world_object_id = ObjectId(2U);
    request.requested_transform = requested;
    request.mode = SceneEditorTransformCommandMode::Apply;
    request.transform_output =
        std::span<WorldSceneObjectTransformRestoreTransformRecord>(
            transform_output.data(),
            transform_output.size());
    request.ledger_output =
        std::span<SceneEditorTransformLedgerRecord>(
            ledger_output.data(),
            ledger_output.size());

    SceneEditorTransformCommandResult result{};
    const SceneEditorTransformCommandStatus status =
        ApplySceneEditorTransformCommand(request, &result);
    if (status != SceneEditorTransformCommandStatus::Success ||
        !result.Succeeded() ||
        result.blocked_layer != SceneEditorTransformCommandBlockedLayer::None ||
        result.selected_world_object_id.value != 2U ||
        result.transform_record_count != 2U ||
        result.ledger_record_count != 1U ||
        !result.consumed_authoring_document ||
        !result.consumed_selection ||
        !result.wrote_transform_output ||
        !result.emitted_undo_redo_ledger ||
        !result.undo_available ||
        result.redo_available ||
        result.mutated_runtime_data ||
        result.opened_native_window ||
        result.used_preview_feedback) {
        return Fail("scene editor transform apply result ledger changed");
    }

    if (!TransformMatches(transform_output[0U].transform_state, transforms[0U].transform_state) ||
        !TransformMatches(transform_output[1U].transform_state, requested) ||
        !TransformMatches(ledger_output[0U].before_transform, transforms[1U].transform_state) ||
        !TransformMatches(ledger_output[0U].after_transform, requested) ||
        ledger_output[0U].world_object_id.value != 2U ||
        ledger_output[0U].command_sequence != 1U ||
        !ledger_output[0U].applied ||
        !ledger_output[0U].undo_available ||
        ledger_output[0U].redo_available) {
        return Fail("scene editor transform apply did not write selected transform and ledger");
    }

    return 0;
}

int SceneEditorTransformCommandUndoRedoReplaysLedgerDeterministically() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)}};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{
        SelectionSidecar(ObjectId(1U))};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        transforms.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        sidecars.data(),
        1U);

    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> apply_transforms{};
    std::array<SceneEditorTransformLedgerRecord, 1U> apply_ledger{};
    SceneEditorTransformCommandRequest apply_request{};
    apply_request.document = &document;
    apply_request.selected_world_object_id = ObjectId(1U);
    apply_request.requested_transform = Transform(50.0F);
    apply_request.mode = SceneEditorTransformCommandMode::Apply;
    apply_request.transform_output =
        std::span<WorldSceneObjectTransformRestoreTransformRecord>(
            apply_transforms.data(),
            apply_transforms.size());
    apply_request.ledger_output =
        std::span<SceneEditorTransformLedgerRecord>(
            apply_ledger.data(),
            apply_ledger.size());
    SceneEditorTransformCommandResult apply_result{};
    if (ApplySceneEditorTransformCommand(apply_request, &apply_result) !=
        SceneEditorTransformCommandStatus::Success) {
        return Fail("scene editor transform undo redo setup apply failed");
    }

    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> undo_transforms{};
    std::array<SceneEditorTransformLedgerRecord, 1U> undo_ledger{};
    SceneEditorTransformCommandRequest undo_request{};
    undo_request.document = &document;
    undo_request.history_record = &apply_ledger[0U];
    undo_request.mode = SceneEditorTransformCommandMode::Undo;
    undo_request.transform_output =
        std::span<WorldSceneObjectTransformRestoreTransformRecord>(
            undo_transforms.data(),
            undo_transforms.size());
    undo_request.ledger_output =
        std::span<SceneEditorTransformLedgerRecord>(
            undo_ledger.data(),
            undo_ledger.size());
    SceneEditorTransformCommandResult undo_result{};
    if (ApplySceneEditorTransformCommand(undo_request, &undo_result) !=
        SceneEditorTransformCommandStatus::Success ||
        !undo_ledger[0U].undone ||
        undo_ledger[0U].undo_available ||
        !undo_ledger[0U].redo_available ||
        undo_ledger[0U].command_sequence != 2U ||
        !TransformMatches(undo_transforms[0U].transform_state, transforms[0U].transform_state)) {
        return Fail("scene editor transform undo did not replay previous transform");
    }

    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> redo_transforms{};
    std::array<SceneEditorTransformLedgerRecord, 1U> redo_ledger{};
    SceneEditorTransformCommandRequest redo_request{};
    redo_request.document = &document;
    redo_request.history_record = &apply_ledger[0U];
    redo_request.mode = SceneEditorTransformCommandMode::Redo;
    redo_request.transform_output =
        std::span<WorldSceneObjectTransformRestoreTransformRecord>(
            redo_transforms.data(),
            redo_transforms.size());
    redo_request.ledger_output =
        std::span<SceneEditorTransformLedgerRecord>(
            redo_ledger.data(),
            redo_ledger.size());
    SceneEditorTransformCommandResult redo_result{};
    if (ApplySceneEditorTransformCommand(redo_request, &redo_result) !=
        SceneEditorTransformCommandStatus::Success ||
        !redo_ledger[0U].redone ||
        !redo_ledger[0U].undo_available ||
        redo_ledger[0U].redo_available ||
        redo_ledger[0U].command_sequence != 2U ||
        !TransformMatches(redo_transforms[0U].transform_state, apply_transforms[0U].transform_state)) {
        return Fail("scene editor transform redo did not replay applied transform");
    }

    return 0;
}

int SceneEditorTransformCommandRejectsInvalidSelectionWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        transforms.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U);
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transform_output{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(990U), Transform(991.0F)}};
    std::array<SceneEditorTransformLedgerRecord, 1U> ledger_output{};
    ledger_output[0U].world_object_id = ObjectId(992U);
    ledger_output[0U].command_sequence = 993U;

    SceneEditorTransformCommandRequest request{};
    request.document = &document;
    request.selected_world_object_id = ObjectId(99U);
    request.requested_transform = Transform(200.0F);
    request.mode = SceneEditorTransformCommandMode::Apply;
    request.transform_output =
        std::span<WorldSceneObjectTransformRestoreTransformRecord>(
            transform_output.data(),
            transform_output.size());
    request.ledger_output =
        std::span<SceneEditorTransformLedgerRecord>(
            ledger_output.data(),
            ledger_output.size());

    SceneEditorTransformCommandResult result{};
    const SceneEditorTransformCommandStatus status =
        ApplySceneEditorTransformCommand(request, &result);
    if (status != SceneEditorTransformCommandStatus::ObjectNotFound ||
        result.blocked_layer != SceneEditorTransformCommandBlockedLayer::Selection ||
        result.wrote_transform_output ||
        result.emitted_undo_redo_ledger ||
        !SentinelTransformUnchanged(transform_output[0U]) ||
        !SentinelLedgerUnchanged(ledger_output[0U])) {
        return Fail("scene editor transform invalid selection mutated outputs");
    }

    return 0;
}

int SceneEditorWorkflowSelectionFeedsViewportAndTransformCommand() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)},
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(2U), MakeObjectHandle(2U, 10U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 2U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)},
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(2U), Transform(20.0F)}};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{
        SelectionSidecar(ObjectId(2U))};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        2U,
        transforms.data(),
        2U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        sidecars.data(),
        1U);

    ResourceBrowserSurfaceSelectionState selection = ReadyResourceSelection();
    PreviewHostViewportSessionResult session = ReadyViewportSession(1U);
    PreviewHostEditorViewportInteractionResult interaction =
        ReadyViewportInteraction(ObjectId(2U), 1U);
    std::array<SceneEditorHierarchyRow, 2U> hierarchy_rows{};
    std::array<SceneEditorInspectorRow, 1U> inspector_rows{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 2U> transform_output{};
    std::array<SceneEditorTransformLedgerRecord, 1U> transform_ledger{};
    std::array<SceneEditorWorkflowLedgerRecord, 1U> workflow_ledger{};

    SceneEditorWorkflowResult result{};
    const WorldTransformState requested = Transform(200.0F);
    const SceneEditorWorkflowStatus status = BuildSceneEditorUsableWorkflowSurface(
        MakeWorkflowRequest(
            &document,
            &selection,
            &session,
            &interaction,
            requested,
            nullptr,
            SceneEditorTransformCommandMode::Apply,
            std::span<SceneEditorHierarchyRow>(hierarchy_rows.data(), hierarchy_rows.size()),
            std::span<SceneEditorInspectorRow>(inspector_rows.data(), inspector_rows.size()),
            std::span<WorldSceneObjectTransformRestoreTransformRecord>(
                transform_output.data(),
                transform_output.size()),
            std::span<SceneEditorTransformLedgerRecord>(
                transform_ledger.data(),
                transform_ledger.size()),
            std::span<SceneEditorWorkflowLedgerRecord>(
                workflow_ledger.data(),
                workflow_ledger.size())),
        &result);

    if (status != SceneEditorWorkflowStatus::Success ||
        !result.Succeeded() ||
        result.blocked_layer != SceneEditorWorkflowBlockedLayer::None ||
        result.surface_status != SceneEditorSurfaceStatus::Success ||
        result.transform_status != SceneEditorTransformCommandStatus::Success ||
        !result.consumed_authoring_document ||
        !result.consumed_resource_browser_selection ||
        !result.consumed_viewport_session ||
        !result.consumed_viewport_interaction ||
        !result.hierarchy_selection_matched_viewport ||
        !result.emitted_hierarchy_rows ||
        !result.emitted_inspector_rows ||
        !result.applied_transform_command ||
        !result.emitted_transform_ledger ||
        !result.committed_workflow ||
        result.mutated_runtime_data ||
        result.opened_native_window) {
        return Fail("scene editor workflow result did not commit visible interaction loop");
    }

    if (result.selected_world_object_id.value != 2U ||
        result.selected_hierarchy_index != 1U ||
        result.viewport_selected_entity_index != 1U ||
        result.workflow_ledger_count != 1U ||
        result.resource_preview_state != ResourceBrowserSurfacePreviewState::Eligible ||
        result.viewport_status != PreviewHostStatus::Success ||
        result.viewport_interaction_status != PreviewHostStatus::Success) {
        return Fail("scene editor workflow selection metadata mismatch");
    }

    if (!hierarchy_rows[1U].selected ||
        hierarchy_rows[1U].world_object_id.value != 2U ||
        !inspector_rows[0U].selected ||
        inspector_rows[0U].world_object_id.value != 2U ||
        !TransformMatches(inspector_rows[0U].transform, transforms[1U].transform_state)) {
        return Fail("scene editor workflow did not bridge hierarchy selection into inspector");
    }

    if (!TransformMatches(transform_output[0U].transform_state, transforms[0U].transform_state) ||
        !TransformMatches(transform_output[1U].transform_state, requested) ||
        transform_ledger[0U].world_object_id.value != 2U ||
        transform_ledger[0U].command_sequence != 1U ||
        !transform_ledger[0U].applied ||
        !transform_ledger[0U].undo_available ||
        workflow_ledger[0U].selected_world_object_id.value != 2U ||
        workflow_ledger[0U].selected_hierarchy_index != 1U ||
        workflow_ledger[0U].viewport_selected_entity_index != 1U ||
        workflow_ledger[0U].transform_command_sequence != 1U ||
        !workflow_ledger[0U].consumed_resource_browser_selection ||
        !workflow_ledger[0U].consumed_viewport_session ||
        !workflow_ledger[0U].consumed_viewport_interaction ||
        !workflow_ledger[0U].matched_hierarchy_to_viewport_selection ||
        !workflow_ledger[0U].applied_transform_command ||
        !workflow_ledger[0U].committed_workflow) {
        return Fail("scene editor workflow transform or workflow ledger mismatch");
    }

    return 0;
}

int SceneEditorWorkflowInspectorSelectionAppliesTransformLedger() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)}};
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> attachments{
        WorldComponentAttachmentSnapshotRecord{ObjectId(1U), WorldComponentTypeId{7U}, WorldComponentSlotId{1U}}};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> bindings{
        WorldComponentResourceBindingSnapshotRecord{
            ObjectId(1U),
            WorldComponentTypeId{7U},
            WorldComponentSlotId{1U},
            MakeResourceHandle(3U, 11U),
            ResourceTypeId{12U}}};
    std::array<WorldSceneAuthoringDependencyRecord, 1U> dependencies{
        WorldSceneAuthoringDependencyRecord{7001U, MakeResourceHandle(3U, 11U), ResourceTypeId{12U}}};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{
        SelectionSidecar(ObjectId(1U))};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        transforms.data(),
        1U,
        attachments.data(),
        1U,
        bindings.data(),
        1U,
        dependencies.data(),
        1U,
        sidecars.data(),
        1U);

    ResourceBrowserSurfaceSelectionState selection = ReadyResourceSelection();
    PreviewHostViewportSessionResult session = ReadyViewportSession(0U);
    PreviewHostEditorViewportInteractionResult interaction =
        ReadyViewportInteraction(ObjectId(1U), 0U);
    std::array<SceneEditorHierarchyRow, 1U> hierarchy_rows{};
    std::array<SceneEditorInspectorRow, 1U> inspector_rows{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transform_output{};
    std::array<SceneEditorTransformLedgerRecord, 1U> transform_ledger{};
    std::array<SceneEditorWorkflowLedgerRecord, 1U> workflow_ledger{};
    const WorldTransformState requested = Transform(70.0F);

    SceneEditorWorkflowResult result{};
    const SceneEditorWorkflowStatus status = BuildSceneEditorUsableWorkflowSurface(
        MakeWorkflowRequest(
            &document,
            &selection,
            &session,
            &interaction,
            requested,
            nullptr,
            SceneEditorTransformCommandMode::Apply,
            std::span<SceneEditorHierarchyRow>(hierarchy_rows.data(), hierarchy_rows.size()),
            std::span<SceneEditorInspectorRow>(inspector_rows.data(), inspector_rows.size()),
            std::span<WorldSceneObjectTransformRestoreTransformRecord>(
                transform_output.data(),
                transform_output.size()),
            std::span<SceneEditorTransformLedgerRecord>(
                transform_ledger.data(),
                transform_ledger.size()),
            std::span<SceneEditorWorkflowLedgerRecord>(
                workflow_ledger.data(),
                workflow_ledger.size())),
        &result);

    if (status != SceneEditorWorkflowStatus::Success ||
        !result.Succeeded() ||
        result.surface.inspector_row_count != 1U ||
        result.transform.ledger_record_count != 1U ||
        !result.emitted_inspector_rows ||
        !result.applied_transform_command) {
        return Fail("scene editor workflow inspector transform loop failed");
    }

    if (!inspector_rows[0U].has_transform ||
        !inspector_rows[0U].has_component_attachments ||
        !inspector_rows[0U].has_resource_bindings ||
        !inspector_rows[0U].separated_runtime_and_editor_fields ||
        inspector_rows[0U].component_count != 1U ||
        inspector_rows[0U].resource_binding_count != 1U ||
        !TransformMatches(inspector_rows[0U].transform, transforms[0U].transform_state)) {
        return Fail("scene editor workflow inspector did not expose selected transform context");
    }

    if (!TransformMatches(transform_output[0U].transform_state, requested) ||
        !TransformMatches(transform_ledger[0U].before_transform, transforms[0U].transform_state) ||
        !TransformMatches(transform_ledger[0U].after_transform, requested) ||
        transform_ledger[0U].world_object_id.value != 1U ||
        !workflow_ledger[0U].emitted_inspector_rows ||
        !workflow_ledger[0U].applied_transform_command) {
        return Fail("scene editor workflow inspector transform ledger mismatch");
    }

    return 0;
}

int SceneEditorWorkflowUndoRedoLedgerReplay() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)}};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{
        SelectionSidecar(ObjectId(1U))};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        transforms.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        sidecars.data(),
        1U);

    SceneEditorTransformLedgerRecord history{};
    history.world_object_id = ObjectId(1U);
    history.before_transform = Transform(10.0F);
    history.after_transform = Transform(50.0F);
    history.command_sequence = 7U;
    history.applied = true;
    history.undo_available = true;

    ResourceBrowserSurfaceSelectionState selection = ReadyResourceSelection();
    PreviewHostViewportSessionResult session = ReadyViewportSession(0U);
    PreviewHostEditorViewportInteractionResult interaction =
        ReadyViewportInteraction(ObjectId(1U), 0U);
    std::array<SceneEditorHierarchyRow, 1U> undo_hierarchy{};
    std::array<SceneEditorInspectorRow, 1U> undo_inspector{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> undo_transform_output{};
    std::array<SceneEditorTransformLedgerRecord, 1U> undo_transform_ledger{};
    std::array<SceneEditorWorkflowLedgerRecord, 1U> undo_workflow_ledger{};

    SceneEditorWorkflowResult undo_result{};
    const SceneEditorWorkflowStatus undo_status = BuildSceneEditorUsableWorkflowSurface(
        MakeWorkflowRequest(
            &document,
            &selection,
            &session,
            &interaction,
            Transform(90.0F),
            &history,
            SceneEditorTransformCommandMode::Undo,
            std::span<SceneEditorHierarchyRow>(undo_hierarchy.data(), undo_hierarchy.size()),
            std::span<SceneEditorInspectorRow>(undo_inspector.data(), undo_inspector.size()),
            std::span<WorldSceneObjectTransformRestoreTransformRecord>(
                undo_transform_output.data(),
                undo_transform_output.size()),
            std::span<SceneEditorTransformLedgerRecord>(
                undo_transform_ledger.data(),
                undo_transform_ledger.size()),
            std::span<SceneEditorWorkflowLedgerRecord>(
                undo_workflow_ledger.data(),
                undo_workflow_ledger.size())),
        &undo_result);

    if (undo_status != SceneEditorWorkflowStatus::Success ||
        !TransformMatches(undo_transform_output[0U].transform_state, history.before_transform) ||
        !undo_transform_ledger[0U].undone ||
        undo_transform_ledger[0U].command_sequence != 8U ||
        !undo_workflow_ledger[0U].replayed_undo ||
        undo_workflow_ledger[0U].replayed_redo ||
        undo_workflow_ledger[0U].transform_command_sequence != 8U) {
        return Fail("scene editor workflow undo replay mismatch");
    }

    std::array<SceneEditorHierarchyRow, 1U> redo_hierarchy{};
    std::array<SceneEditorInspectorRow, 1U> redo_inspector{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> redo_transform_output{};
    std::array<SceneEditorTransformLedgerRecord, 1U> redo_transform_ledger{};
    std::array<SceneEditorWorkflowLedgerRecord, 1U> redo_workflow_ledger{};

    SceneEditorWorkflowResult redo_result{};
    const SceneEditorWorkflowStatus redo_status = BuildSceneEditorUsableWorkflowSurface(
        MakeWorkflowRequest(
            &document,
            &selection,
            &session,
            &interaction,
            Transform(90.0F),
            &history,
            SceneEditorTransformCommandMode::Redo,
            std::span<SceneEditorHierarchyRow>(redo_hierarchy.data(), redo_hierarchy.size()),
            std::span<SceneEditorInspectorRow>(redo_inspector.data(), redo_inspector.size()),
            std::span<WorldSceneObjectTransformRestoreTransformRecord>(
                redo_transform_output.data(),
                redo_transform_output.size()),
            std::span<SceneEditorTransformLedgerRecord>(
                redo_transform_ledger.data(),
                redo_transform_ledger.size()),
            std::span<SceneEditorWorkflowLedgerRecord>(
                redo_workflow_ledger.data(),
                redo_workflow_ledger.size())),
        &redo_result);

    if (redo_status != SceneEditorWorkflowStatus::Success ||
        !TransformMatches(redo_transform_output[0U].transform_state, history.after_transform) ||
        !redo_transform_ledger[0U].redone ||
        redo_transform_ledger[0U].command_sequence != 8U ||
        redo_workflow_ledger[0U].replayed_undo ||
        !redo_workflow_ledger[0U].replayed_redo ||
        redo_workflow_ledger[0U].transform_command_sequence != 8U) {
        return Fail("scene editor workflow redo replay mismatch");
    }

    return 0;
}

int SceneEditorWorkflowBlockedDependencyDoesNotMutateOutputs() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 10U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)}};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{
        SelectionSidecar(ObjectId(1U))};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        transforms.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        sidecars.data(),
        1U);

    ResourceBrowserSurfaceSelectionState selection = ReadyResourceSelection();
    selection.preview_state = ResourceBrowserSurfacePreviewState::BlockedByDiagnostic;
    selection.preview_eligible = false;
    selection.diagnostic_blocks_preview = true;
    PreviewHostViewportSessionResult session = ReadyViewportSession(0U);
    PreviewHostEditorViewportInteractionResult interaction =
        ReadyViewportInteraction(ObjectId(1U), 0U);
    std::array<SceneEditorHierarchyRow, 1U> hierarchy_rows{};
    hierarchy_rows[0U].world_object_id = ObjectId(900U);
    hierarchy_rows[0U].object_handle = MakeObjectHandle(90U, 1U);
    hierarchy_rows[0U].row_index = 77U;
    std::array<SceneEditorInspectorRow, 1U> inspector_rows{};
    inspector_rows[0U].world_object_id = ObjectId(901U);
    inspector_rows[0U].object_handle = MakeObjectHandle(91U, 1U);
    inspector_rows[0U].component_count = 77U;
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transform_output{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(990U), Transform(991.0F)}};
    std::array<SceneEditorTransformLedgerRecord, 1U> transform_ledger{};
    transform_ledger[0U].world_object_id = ObjectId(992U);
    transform_ledger[0U].command_sequence = 993U;
    std::array<SceneEditorWorkflowLedgerRecord, 1U> workflow_ledger{};
    workflow_ledger[0U].selected_world_object_id = ObjectId(994U);
    workflow_ledger[0U].transform_command_sequence = 995U;

    SceneEditorWorkflowResult result{};
    const SceneEditorWorkflowStatus status = BuildSceneEditorUsableWorkflowSurface(
        MakeWorkflowRequest(
            &document,
            &selection,
            &session,
            &interaction,
            Transform(70.0F),
            nullptr,
            SceneEditorTransformCommandMode::Apply,
            std::span<SceneEditorHierarchyRow>(hierarchy_rows.data(), hierarchy_rows.size()),
            std::span<SceneEditorInspectorRow>(inspector_rows.data(), inspector_rows.size()),
            std::span<WorldSceneObjectTransformRestoreTransformRecord>(
                transform_output.data(),
                transform_output.size()),
            std::span<SceneEditorTransformLedgerRecord>(
                transform_ledger.data(),
                transform_ledger.size()),
            std::span<SceneEditorWorkflowLedgerRecord>(
                workflow_ledger.data(),
                workflow_ledger.size())),
        &result);

    if (status != SceneEditorWorkflowStatus::BlockedResourceBrowserSelection ||
        result.blocked_layer != SceneEditorWorkflowBlockedLayer::ResourceBrowserSelection ||
        result.resource_preview_state != ResourceBrowserSurfacePreviewState::BlockedByDiagnostic ||
        !result.consumed_authoring_document ||
        result.consumed_resource_browser_selection ||
        result.consumed_viewport_session ||
        result.consumed_viewport_interaction ||
        result.emitted_hierarchy_rows ||
        result.emitted_inspector_rows ||
        result.applied_transform_command ||
        result.committed_workflow) {
        return Fail("scene editor workflow blocked dependency result mismatch");
    }

    if (!SentinelHierarchyUnchanged(hierarchy_rows[0U]) ||
        !SentinelInspectorUnchanged(inspector_rows[0U]) ||
        !SentinelTransformUnchanged(transform_output[0U]) ||
        !SentinelLedgerUnchanged(transform_ledger[0U]) ||
        !SentinelWorkflowLedgerUnchanged(workflow_ledger[0U])) {
        return Fail("scene editor workflow blocked dependency mutated outputs");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail("expected one test name");
    }

    const std::string_view test_name(argv[1]);
    if (test_name == TEST_BUILDS_SURFACE) {
        return SceneEditorSurfaceBuildsHierarchyAndInspectorFromAuthoringDocument();
    }

    if (test_name == TEST_REJECTS_SMALL_HIERARCHY) {
        return SceneEditorSurfaceRejectsSmallHierarchyOutputWithoutMutation();
    }

    if (test_name == TEST_REJECTS_INVALID_DOCUMENT) {
        return SceneEditorSurfaceRejectsInvalidAuthoringDocumentWithoutMutation();
    }

    if (test_name == TEST_SELECTION_REQUIRED) {
        return SceneEditorSurfaceReportsSelectionRequiredWithoutMutation();
    }

    if (test_name == TEST_FIELD_GROUPING) {
        return SceneEditorSurfaceGroupsRuntimeAndEditorOnlyInspectorFields();
    }

    if (test_name == TEST_BOUNDARY_FLAGS) {
        return SceneEditorSurfaceRemainsEditorDataOnlyWithoutRuntimeMutation();
    }

    if (test_name == TEST_TRANSFORM_APPLY) {
        return SceneEditorTransformCommandAppliesSelectedTransformAndWritesUndoLedger();
    }

    if (test_name == TEST_TRANSFORM_UNDO_REDO) {
        return SceneEditorTransformCommandUndoRedoReplaysLedgerDeterministically();
    }

    if (test_name == TEST_TRANSFORM_INVALID_SELECTION) {
        return SceneEditorTransformCommandRejectsInvalidSelectionWithoutMutation();
    }

    if (test_name == TEST_WORKFLOW_SELECTION_VIEWPORT) {
        return SceneEditorWorkflowSelectionFeedsViewportAndTransformCommand();
    }

    if (test_name == TEST_WORKFLOW_INSPECTOR_TRANSFORM) {
        return SceneEditorWorkflowInspectorSelectionAppliesTransformLedger();
    }

    if (test_name == TEST_WORKFLOW_UNDO_REDO) {
        return SceneEditorWorkflowUndoRedoLedgerReplay();
    }

    if (test_name == TEST_WORKFLOW_BLOCKED_DEPENDENCY) {
        return SceneEditorWorkflowBlockedDependencyDoesNotMutateOutputs();
    }

    return Fail("unknown test name");
}
