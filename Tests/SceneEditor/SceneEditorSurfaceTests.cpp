// 模块: Tests SceneEditor
// 文件: Tests/SceneEditor/SceneEditorSurfaceTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/SceneEditor/SceneEditorSurface.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldSceneAuthoringDocument.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldTransformState.h"

namespace {
using yuengine::object::ObjectHandle;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceTypeId;
using yuengine::sceneeditor::BuildSceneEditorNativeSurface;
using yuengine::sceneeditor::SceneEditorHierarchyRow;
using yuengine::sceneeditor::SceneEditorInspectorRow;
using yuengine::sceneeditor::SceneEditorSurfaceRequest;
using yuengine::sceneeditor::SceneEditorSurfaceResult;
using yuengine::sceneeditor::SceneEditorSurfaceStatus;
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
constexpr const char *TEST_BOUNDARY_FLAGS =
    "SceneEditorSurface_RemainsEditorDataOnlyWithoutRuntimeMutation";

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
        !hierarchy_rows[0U].selected ||
        !hierarchy_rows[0U].expanded) {
        return Fail("first hierarchy row did not expose selected object summary");
    }

    if (hierarchy_rows[1U].world_object_id.value != 2U ||
        hierarchy_rows[1U].component_count != 0U ||
        hierarchy_rows[1U].resource_binding_count != 0U ||
        !hierarchy_rows[1U].has_transform ||
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

    if (test_name == TEST_BOUNDARY_FLAGS) {
        return SceneEditorSurfaceRemainsEditorDataOnlyWithoutRuntimeMutation();
    }

    return Fail("unknown test name");
}
