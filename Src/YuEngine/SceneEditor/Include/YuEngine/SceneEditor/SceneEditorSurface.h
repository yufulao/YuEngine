// 模块: YuEngine SceneEditor
// 文件: Src/YuEngine/SceneEditor/Include/YuEngine/SceneEditor/SceneEditorSurface.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldSceneAuthoringDocument.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::sceneeditor {

enum class SceneEditorSurfaceStatus {
    Success,
    InvalidArgument,
    InvalidAuthoringDocument,
    OutputCapacityExceeded,
    SelectionRequired
};

struct SceneEditorHierarchyRow final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
    std::uint32_t row_index = 0U;
    std::uint32_t depth = 0U;
    std::uint32_t component_count = 0U;
    std::uint32_t resource_binding_count = 0U;
    bool has_transform = false;
    bool selected = false;
    bool expanded = true;
};

struct SceneEditorInspectorRow final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
    yuengine::world::WorldTransformState transform{};
    std::uint32_t component_count = 0U;
    std::uint32_t resource_binding_count = 0U;
    bool has_transform = false;
    bool has_component_attachments = false;
    bool has_resource_bindings = false;
    bool selected = false;
};

struct SceneEditorSurfaceRequest final {
    const yuengine::world::WorldSceneAuthoringDocument *document = nullptr;
    std::span<SceneEditorHierarchyRow> hierarchy_rows{};
    std::span<SceneEditorInspectorRow> inspector_rows{};
    bool require_selection = false;
};

struct SceneEditorSurfaceResult final {
    SceneEditorSurfaceStatus status = SceneEditorSurfaceStatus::InvalidArgument;
    std::uint64_t scene_document_id = yuengine::world::INVALID_WORLD_SCENE_DOCUMENT_ID;
    std::uint64_t deterministic_document_hash =
        yuengine::world::INVALID_WORLD_SCENE_DOCUMENT_HASH;
    std::uint32_t hierarchy_row_count = 0U;
    std::uint32_t inspector_row_count = 0U;
    std::uint32_t selected_object_count = 0U;
    std::uint32_t folded_object_count = 0U;
    std::uint32_t component_attachment_count = 0U;
    std::uint32_t resource_binding_count = 0U;
    std::uint32_t validated_sidecar_count = 0U;
    yuengine::world::WorldSceneAuthoringDocumentStatus authoring_status =
        yuengine::world::WorldSceneAuthoringDocumentStatus::Success;
    bool consumed_authoring_document = false;
    bool consumed_editor_sidecar = false;
    bool exported_runtime_data = false;
    bool mutated_runtime_data = false;
    bool opened_native_window = false;
    bool used_preview_feedback = false;

    bool Succeeded() const {
        return status == SceneEditorSurfaceStatus::Success;
    }
};

SceneEditorSurfaceStatus BuildSceneEditorNativeSurface(
    const SceneEditorSurfaceRequest &request,
    SceneEditorSurfaceResult *out_result);

}
