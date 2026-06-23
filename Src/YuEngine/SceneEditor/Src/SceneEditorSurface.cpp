// 模块: YuEngine SceneEditor
// 文件: Src/YuEngine/SceneEditor/Src/SceneEditorSurface.cpp

#include "YuEngine/SceneEditor/SceneEditorSurface.h"

#include <array>
#include <cstdint>

namespace yuengine::sceneeditor {
namespace {
using WorldComponentAttachmentSnapshotRecord =
    yuengine::world::WorldComponentAttachmentSnapshotRecord;
using WorldComponentResourceBindingSnapshotRecord =
    yuengine::world::WorldComponentResourceBindingSnapshotRecord;
using WorldObjectId = yuengine::world::WorldObjectId;
using WorldSceneAuthoringDependencyRecord =
    yuengine::world::WorldSceneAuthoringDependencyRecord;
using WorldSceneAuthoringDocument =
    yuengine::world::WorldSceneAuthoringDocument;
using WorldSceneAuthoringDocumentStatus =
    yuengine::world::WorldSceneAuthoringDocumentStatus;
using WorldSceneAuthoringDocumentValidator =
    yuengine::world::WorldSceneAuthoringDocumentValidator;
using WorldSceneAuthoringRuntimeExport =
    yuengine::world::WorldSceneAuthoringRuntimeExport;
using WorldSceneEditorSidecarKind =
    yuengine::world::WorldSceneEditorSidecarKind;
using WorldSceneEditorSidecarRecord =
    yuengine::world::WorldSceneEditorSidecarRecord;
using WorldSceneObjectTransformRestoreIdentityRecord =
    yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using WorldSceneObjectTransformRestoreTransformRecord =
    yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;

bool IsSpanStorageValid(std::span<SceneEditorHierarchyRow> rows) {
    if (rows.empty()) {
        return true;
    }

    return rows.data() != nullptr;
}

bool IsSpanStorageValid(std::span<SceneEditorInspectorRow> rows) {
    if (rows.empty()) {
        return true;
    }

    return rows.data() != nullptr;
}

bool IsObjectEqual(WorldObjectId left, WorldObjectId right) {
    return left.value == right.value;
}

bool IsObjectHandleActive(yuengine::object::ObjectHandle handle) {
    return handle.IsValid();
}

SceneEditorSurfaceStatus MapAuthoringStatus(WorldSceneAuthoringDocumentStatus status) {
    if (status == WorldSceneAuthoringDocumentStatus::Success) {
        return SceneEditorSurfaceStatus::Success;
    }

    if (status == WorldSceneAuthoringDocumentStatus::OutputCapacityExceeded) {
        return SceneEditorSurfaceStatus::OutputCapacityExceeded;
    }

    if (status == WorldSceneAuthoringDocumentStatus::InputCountExceeded) {
        return SceneEditorSurfaceStatus::OutputCapacityExceeded;
    }

    return SceneEditorSurfaceStatus::InvalidAuthoringDocument;
}

WorldSceneAuthoringDocumentStatus ValidateAuthoringDocument(
    const WorldSceneAuthoringDocument &document) {
    std::array<
        WorldSceneObjectTransformRestoreIdentityRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> identities{};
    std::array<
        WorldSceneObjectTransformRestoreTransformRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> transforms{};
    std::array<
        WorldComponentAttachmentSnapshotRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> attachments{};
    std::array<
        WorldComponentResourceBindingSnapshotRecord,
        yuengine::world::MAX_WORLD_OBJECT_COUNT> bindings{};
    std::array<
        WorldSceneAuthoringDependencyRecord,
        yuengine::world::MAX_WORLD_SCENE_AUTHORING_DEPENDENCY_COUNT> dependencies{};
    std::uint32_t identity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t attachment_count = 0U;
    std::uint32_t binding_count = 0U;
    std::uint32_t dependency_count = 0U;

    WorldSceneAuthoringRuntimeExport runtime_export{};
    runtime_export.identity_records = identities.data();
    runtime_export.identity_capacity = static_cast<std::uint32_t>(identities.size());
    runtime_export.identity_count = &identity_count;
    runtime_export.transform_records = transforms.data();
    runtime_export.transform_capacity = static_cast<std::uint32_t>(transforms.size());
    runtime_export.transform_count = &transform_count;
    runtime_export.attachment_records = attachments.data();
    runtime_export.attachment_capacity = static_cast<std::uint32_t>(attachments.size());
    runtime_export.attachment_count = &attachment_count;
    runtime_export.binding_records = bindings.data();
    runtime_export.binding_capacity = static_cast<std::uint32_t>(bindings.size());
    runtime_export.binding_count = &binding_count;
    runtime_export.dependency_records = dependencies.data();
    runtime_export.dependency_capacity = static_cast<std::uint32_t>(dependencies.size());
    runtime_export.dependency_count = &dependency_count;

    WorldSceneAuthoringDocumentValidator validator;
    return validator.ValidateAndExport(document, &runtime_export).status;
}

std::uint32_t CountSidecarKind(
    const WorldSceneAuthoringDocument &document,
    WorldSceneEditorSidecarKind kind) {
    std::uint32_t count = 0U;
    std::uint32_t index = 0U;
    while (index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = document.sidecar_records[index];
        if (record.kind == kind) {
            ++count;
        }

        ++index;
    }

    return count;
}

std::uint32_t CountAttachmentsForObject(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t count = 0U;
    std::uint32_t index = 0U;
    while (index < document.header.attachment_record_count) {
        const WorldComponentAttachmentSnapshotRecord &record =
            document.attachment_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            ++count;
        }

        ++index;
    }

    return count;
}

std::uint32_t CountBindingsForObject(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t count = 0U;
    std::uint32_t index = 0U;
    while (index < document.header.binding_record_count) {
        const WorldComponentResourceBindingSnapshotRecord &record =
            document.binding_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            ++count;
        }

        ++index;
    }

    return count;
}

const WorldSceneObjectTransformRestoreTransformRecord *FindTransform(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.transform_record_count) {
        const WorldSceneObjectTransformRestoreTransformRecord &record =
            document.transform_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            return &record;
        }

        ++index;
    }

    return nullptr;
}

bool HasSidecarKindForObject(
    const WorldSceneAuthoringDocument &document,
    WorldSceneEditorSidecarKind kind,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = document.sidecar_records[index];
        if (record.kind != kind) {
            ++index;
            continue;
        }

        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            return true;
        }

        ++index;
    }

    return false;
}

std::uint32_t CountSidecarsForObject(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t count = 0U;
    std::uint32_t index = 0U;
    while (index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = document.sidecar_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            ++count;
        }

        ++index;
    }

    return count;
}

bool IsExpanded(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &record = document.sidecar_records[index];
        if (record.kind != WorldSceneEditorSidecarKind::Foldout) {
            ++index;
            continue;
        }

        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            return record.value != 0U;
        }

        ++index;
    }

    return true;
}

const WorldSceneObjectTransformRestoreIdentityRecord *FindIdentity(
    const WorldSceneAuthoringDocument &document,
    WorldObjectId world_object_id) {
    std::uint32_t index = 0U;
    while (index < document.header.identity_record_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &record =
            document.identity_records[index];
        if (IsObjectEqual(record.world_object_id, world_object_id)) {
            return &record;
        }

        ++index;
    }

    return nullptr;
}

SceneEditorHierarchyRow BuildHierarchyRow(
    const WorldSceneAuthoringDocument &document,
    const WorldSceneObjectTransformRestoreIdentityRecord &identity,
    std::uint32_t row_index) {
    SceneEditorHierarchyRow row{};
    row.world_object_id = identity.world_object_id;
    row.object_handle = identity.object_handle;
    row.row_index = row_index;
    row.depth = 0U;
    row.component_count = CountAttachmentsForObject(document, identity.world_object_id);
    row.resource_binding_count = CountBindingsForObject(document, identity.world_object_id);
    row.has_transform = FindTransform(document, identity.world_object_id) != nullptr;
    row.visible = identity.world_object_id.IsValid();
    row.active = IsObjectHandleActive(identity.object_handle);
    row.selected = HasSidecarKindForObject(
        document,
        WorldSceneEditorSidecarKind::Selection,
        identity.world_object_id);
    row.expanded = IsExpanded(document, identity.world_object_id);
    return row;
}

SceneEditorInspectorRow BuildInspectorRow(
    const WorldSceneAuthoringDocument &document,
    const WorldSceneObjectTransformRestoreIdentityRecord &identity) {
    SceneEditorInspectorRow row{};
    row.world_object_id = identity.world_object_id;
    row.object_handle = identity.object_handle;
    const WorldSceneObjectTransformRestoreTransformRecord *transform =
        FindTransform(document, identity.world_object_id);
    row.has_transform = transform != nullptr;
    if (transform != nullptr) {
        row.transform = transform->transform_state;
    }

    row.component_count = CountAttachmentsForObject(document, identity.world_object_id);
    row.resource_binding_count = CountBindingsForObject(document, identity.world_object_id);
    row.runtime_export_field_count = 1U;
    if (row.has_transform) {
        ++row.runtime_export_field_count;
    }

    row.runtime_export_field_count += row.component_count;
    row.runtime_export_field_count += row.resource_binding_count;
    row.editor_only_sidecar_field_count =
        CountSidecarsForObject(document, identity.world_object_id);
    row.has_component_attachments = row.component_count > 0U;
    row.has_resource_bindings = row.resource_binding_count > 0U;
    row.separated_runtime_and_editor_fields = true;
    row.selected = true;
    return row;
}

void FillResultMetadata(
    const WorldSceneAuthoringDocument &document,
    SceneEditorSurfaceResult *result) {
    if (result == nullptr) {
        return;
    }

    result->scene_document_id = document.header.scene_document_id;
    result->deterministic_document_hash = document.header.deterministic_document_hash;
    result->component_attachment_count = document.header.attachment_record_count;
    result->resource_binding_count = document.header.binding_record_count;
    result->validated_sidecar_count = document.header.sidecar_record_count;
    result->selected_object_count =
        CountSidecarKind(document, WorldSceneEditorSidecarKind::Selection);
    result->folded_object_count =
        CountSidecarKind(document, WorldSceneEditorSidecarKind::Foldout);
    result->consumed_authoring_document = true;
    result->consumed_editor_sidecar = document.header.sidecar_record_count > 0U;
}

}

SceneEditorSurfaceStatus BuildSceneEditorNativeSurface(
    const SceneEditorSurfaceRequest &request,
    SceneEditorSurfaceResult *out_result) {
    if (out_result == nullptr) {
        return SceneEditorSurfaceStatus::InvalidArgument;
    }

    SceneEditorSurfaceResult result{};
    if (request.document == nullptr ||
        !IsSpanStorageValid(request.hierarchy_rows) ||
        !IsSpanStorageValid(request.inspector_rows)) {
        *out_result = result;
        return result.status;
    }

    const WorldSceneAuthoringDocument &document = *request.document;
    const WorldSceneAuthoringDocumentStatus authoring_status =
        ValidateAuthoringDocument(document);
    result.authoring_status = authoring_status;
    if (authoring_status != WorldSceneAuthoringDocumentStatus::Success) {
        result.status = MapAuthoringStatus(authoring_status);
        *out_result = result;
        return result.status;
    }

    FillResultMetadata(document, &result);
    if (request.hierarchy_rows.size() < document.header.identity_record_count) {
        result.status = SceneEditorSurfaceStatus::OutputCapacityExceeded;
        *out_result = result;
        return result.status;
    }

    if (request.inspector_rows.size() < result.selected_object_count) {
        result.status = SceneEditorSurfaceStatus::OutputCapacityExceeded;
        *out_result = result;
        return result.status;
    }

    if (request.require_selection && result.selected_object_count == 0U) {
        result.status = SceneEditorSurfaceStatus::SelectionRequired;
        *out_result = result;
        return result.status;
    }

    std::uint32_t identity_index = 0U;
    while (identity_index < document.header.identity_record_count) {
        request.hierarchy_rows[identity_index] = BuildHierarchyRow(
            document,
            document.identity_records[identity_index],
            identity_index);
        ++result.hierarchy_row_count;
        ++identity_index;
    }

    std::uint32_t sidecar_index = 0U;
    while (sidecar_index < document.header.sidecar_record_count) {
        const WorldSceneEditorSidecarRecord &sidecar =
            document.sidecar_records[sidecar_index];
        if (sidecar.kind != WorldSceneEditorSidecarKind::Selection) {
            ++sidecar_index;
            continue;
        }

        const WorldSceneObjectTransformRestoreIdentityRecord *identity =
            FindIdentity(document, sidecar.world_object_id);
        if (identity != nullptr) {
            request.inspector_rows[result.inspector_row_count] =
                BuildInspectorRow(document, *identity);
            ++result.inspector_row_count;
        }

        ++sidecar_index;
    }

    result.status = SceneEditorSurfaceStatus::Success;
    *out_result = result;
    return result.status;
}

}
