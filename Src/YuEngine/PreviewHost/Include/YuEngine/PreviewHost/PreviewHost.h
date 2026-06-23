// Module: YuEngine PreviewHost
// File: Src/YuEngine/PreviewHost/Include/YuEngine/PreviewHost/PreviewHost.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameResult.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h"
#include "YuEngine/RenderScene/RenderSceneThreePrimitiveCaptureRoute.h"
#include "YuEngine/ResourceBrowser/ResourceBrowserDiagnostics.h"
#include "YuEngine/ResourceBrowser/ResourceBrowserSurface.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"
#include "YuEngine/World/WorldSceneAuthoringDocument.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::previewhost {
constexpr std::size_t MAX_PREVIEW_HOST_SESSIONS = 2U;
constexpr std::size_t MAX_PREVIEW_HOST_FRAME_ENTITIES = 16U;

enum class PreviewHostDocumentKind {
    Unknown,
    Scene,
    Animation,
    Ui,
    Resource
};

enum class PreviewHostFrameFormat {
    Unknown,
    Headless,
    Rgba8
};

enum class PreviewHostStatus {
    Success,
    InvalidArgument,
    SessionCapacityExceeded,
    StaleSession,
    UnsupportedDocumentKind,
    RuntimeAssetStatusFailed,
    RuntimeAssetGraphStale,
    MissingResourceRef,
    TypeMismatch,
    NotCooked,
    OutputCapacityExceeded,
    MissingCamera,
    UnsupportedPreviewRoute,
    RenderSceneFailed,
    RenderCoreRhiFailed,
    MissingCommandOutput,
    InvalidCookedRecord,
    UnsupportedBridgeLayer
};

enum class PreviewHostSceneDocumentViewportBlockedLayer {
    None,
    SceneAuthoringDocument,
    RuntimeExport,
    RuntimeSceneEntityMapping,
    ViewportSession
};

enum class PreviewHostDiagnosticCode {
    None,
    MissingRuntimeAssetGraph,
    StaleSession,
    RuntimeAssetStatusFailed,
    RuntimeAssetGraphStale,
    MissingResourceRef,
    StaleResourceRef,
    TypeMismatch,
    UnsupportedDocumentKind,
    UnsupportedPreviewRoute,
    NotCooked,
    OutputCapacityExceeded,
    MissingCamera,
    RenderSceneFailed,
    RenderCoreRhiFailed,
    MissingCommandOutput,
    InvalidCookedRecord,
    UnsupportedBridgeLayer
};

struct PreviewHostSessionId final {
    std::uint32_t slot = 0U;
    std::uint32_t generation = 0U;

    bool IsValid() const {
        if (slot == 0U) {
            return false;
        }

        return generation != 0U;
    }
};

struct PreviewHostSessionDesc final {
    PreviewHostDocumentKind document_kind = PreviewHostDocumentKind::Scene;
};

struct PreviewHostSessionResult final {
    PreviewHostStatus status = PreviewHostStatus::InvalidArgument;
    PreviewHostSessionId session{};
    std::size_t active_session_count = 0U;
};

struct PreviewHostFrameDescriptor final {
    std::uint32_t frame_id = 0U;
    std::uint16_t width = 0U;
    std::uint16_t height = 0U;
    PreviewHostFrameFormat format = PreviewHostFrameFormat::Headless;
    bool capture_requested = false;
};

struct PreviewHostCameraState final {
    std::uint32_t camera_id = 0U;
    float orbit_angle_radians = 0.0F;
    float orbit_radius = 0.0F;
    float orbit_height = 0.0F;
};

struct PreviewHostDiagnostic final {
    PreviewHostDiagnosticCode code = PreviewHostDiagnosticCode::None;
    PreviewHostStatus status = PreviewHostStatus::Success;
    yuengine::runtimeasset::RuntimeAssetDataStatus runtime_asset_status =
        yuengine::runtimeasset::RuntimeAssetDataStatus::Success;
    yuengine::runtimeasset::RuntimeAssetImportCookMissingLayer import_cook_missing_layer =
        yuengine::runtimeasset::RuntimeAssetImportCookMissingLayer::None;
    yuengine::renderscene::RenderSceneRuntimeFrameStatus frame_status =
        yuengine::renderscene::RenderSceneRuntimeFrameStatus::Success;
    yuengine::renderscene::RenderSceneThreePrimitiveCaptureMissingLayer missing_layer =
        yuengine::renderscene::RenderSceneThreePrimitiveCaptureMissingLayer::None;
    yuengine::runtimeasset::RuntimeAssetFileKind expected_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    yuengine::runtimeasset::RuntimeAssetFileKind actual_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    yuengine::resourcebrowser::ResourceBrowserDiagnosticCode resource_browser_code =
        yuengine::resourcebrowser::ResourceBrowserDiagnosticCode::None;
    yuengine::resourcebrowser::ResourceBrowserDiagnosticSeverity resource_browser_severity =
        yuengine::resourcebrowser::ResourceBrowserDiagnosticSeverity::Info;
    yuengine::resourcebrowser::ResourceBrowserDiagnosticPhase resource_browser_phase =
        yuengine::resourcebrowser::ResourceBrowserDiagnosticPhase::ImportSettings;
    yuengine::resourcebrowser::ResourceBrowserDependencyState resource_browser_dependency_state =
        yuengine::resourcebrowser::ResourceBrowserDependencyState::Unknown;
    std::uint64_t stable_id = 0U;
    std::uint32_t loaded_file_index = 0U;
    std::uint32_t resource_ref_index = 0U;
    std::uint32_t entity_index = 0U;
    bool from_resource_browser_diagnostics = false;
};

struct PreviewHostHitRecord final {
    yuengine::world::WorldObjectId world_object_id{};
    std::uint32_t entity_index = 0U;
    bool hit_available = false;
};

struct PreviewHostSelectionRecord final {
    yuengine::world::WorldObjectId world_object_id{};
    std::uint32_t entity_index = 0U;
    bool selectable = false;
};

struct PreviewHostTransformFeedback final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState transform{};
    bool transform_available = false;
};

struct PreviewHostCommandOutputRef final {
    const yuengine::runtimeasset::RuntimeAssetImportCookCommandResult *command = nullptr;
    const yuengine::runtimeasset::RuntimeAssetFileDesc *cooked_scene = nullptr;
    std::span<const yuengine::runtimeasset::RuntimeAssetFileDesc> cooked_files{};
    bool require_cooked_records = false;
};

struct PreviewHostResourceBrowserPreviewRequest final {
    const yuengine::resourcebrowser::ResourceBrowserResourceEntry *entry = nullptr;
    std::uint32_t entry_index = 0U;
    std::span<const yuengine::resourcebrowser::ResourceBrowserDiagnosticRecord> diagnostics{};
};

struct PreviewHostResourceBrowserPreviewResult final {
    PreviewHostStatus status = PreviewHostStatus::InvalidArgument;
    PreviewHostDocumentKind document_kind = PreviewHostDocumentKind::Unknown;
    PreviewHostDiagnostic diagnostic{};
    std::uint32_t resource_browser_diagnostic_count = 0U;
    bool accepted_resource_browser_entry = false;
    bool preview_eligible = false;
    bool used_locator_path_as_type_truth = false;
};

struct PreviewHostFrameRequest final {
    PreviewHostSessionId session{};
    PreviewHostDocumentKind document_kind = PreviewHostDocumentKind::Scene;
    PreviewHostFrameDescriptor frame{};
    PreviewHostCameraState camera_state{};
    PreviewHostCommandOutputRef command_output{};
    const yuengine::runtimeasset::RuntimeAssetGraphLoadResult *runtime_graph = nullptr;
    const yuengine::runtimeasset::RuntimeAssetSceneLoaderOutput *scene_output = nullptr;
    std::span<const yuengine::runtimeasset::RuntimeAssetLoadedFile> loaded_files{};
    std::span<const yuengine::runtimeasset::RuntimeAssetSceneResourceRef> resource_refs{};
    std::span<const yuengine::runtimeasset::RuntimeAssetSceneEntityRecord> scene_entities{};
    std::span<const yuengine::renderscene::RenderScenePrimitiveGeometryRecord> geometry_records{};
    yuengine::renderscene::RenderSceneCameraBindingResult camera{};
    yuengine::renderscene::RenderSceneRuntimeMaterialRecord material{};
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    const char *output_path = nullptr;
    std::size_t output_path_byte_count = 0U;
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget_per_entity = 0U;
    std::span<PreviewHostDiagnostic> diagnostics{};
    std::span<PreviewHostHitRecord> hit_records{};
    std::span<PreviewHostSelectionRecord> selection_records{};
    std::span<PreviewHostTransformFeedback> transform_feedback{};
};

struct PreviewHostFrameResult final {
    PreviewHostStatus status = PreviewHostStatus::InvalidArgument;
    PreviewHostFrameDescriptor frame{};
    PreviewHostCameraState camera_state{};
    yuengine::runtimeasset::RuntimeAssetDataStatus runtime_asset_status =
        yuengine::runtimeasset::RuntimeAssetDataStatus::Success;
    yuengine::runtimeasset::RuntimeAssetDataStatus import_cook_status =
        yuengine::runtimeasset::RuntimeAssetDataStatus::Success;
    yuengine::runtimeasset::RuntimeAssetImportCookMissingLayer import_cook_missing_layer =
        yuengine::runtimeasset::RuntimeAssetImportCookMissingLayer::None;
    yuengine::renderscene::RenderSceneRuntimeFrameResult render_frame{};
    yuengine::renderscene::RenderSceneThreePrimitiveCaptureResult capture{};
    std::size_t diagnostic_count = 0U;
    std::size_t hit_record_count = 0U;
    std::size_t selection_record_count = 0U;
    std::size_t transform_feedback_count = 0U;
    std::uint32_t runtime_loaded_file_count = 0U;
    std::uint32_t command_cooked_file_count = 0U;
    std::uint32_t resource_ref_count = 0U;
    std::uint32_t submitted_entity_count = 0U;
    std::size_t capture_bytes_written = 0U;
    bool consumed_runtime_asset_graph = false;
    bool consumed_import_cook_command_output = false;
    bool consumed_resource_refs = false;
    bool submitted_render_scene_frame = false;
    bool captured_through_render_core_rhi = false;
    bool headless_output = false;
};

struct PreviewHostViewportSessionRequest final {
    PreviewHostFrameRequest frame_request{};
    const yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionState
        *resource_browser_selection = nullptr;
    std::uint32_t selected_entity_index = 0U;
    bool require_selected_entity = false;
};

struct PreviewHostViewportSessionResult final {
    PreviewHostStatus status = PreviewHostStatus::InvalidArgument;
    PreviewHostFrameResult frame{};
    PreviewHostCameraState camera_state{};
    yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState
        resource_browser_preview_state =
            yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState::Unknown;
    yuengine::resourcebrowser::ResourceBrowserSurfaceDocumentKind
        resource_browser_document_kind =
            yuengine::resourcebrowser::ResourceBrowserSurfaceDocumentKind::None;
    std::uint32_t selected_entity_index = 0U;
    std::uint32_t viewport_width = 0U;
    std::uint32_t viewport_height = 0U;
    std::uint32_t matched_resource_browser_diagnostic_count = 0U;
    bool consumed_viewport_controls = false;
    bool consumed_resource_browser_selection = false;
    bool resource_browser_preview_eligible = false;
    bool resource_asset_mapping_preserved = false;
    bool selected_entity_available = false;
    bool used_locator_path_as_type_truth = false;
    bool built_frame = false;
    bool emitted_hit_feedback = false;
    bool emitted_selection_feedback = false;
    bool emitted_transform_feedback = false;
};

struct PreviewHostSceneDocumentViewportRequest final {
    const yuengine::world::WorldSceneAuthoringDocument *scene_document = nullptr;
    yuengine::world::WorldSceneAuthoringRuntimeExport runtime_export{};
    PreviewHostViewportSessionRequest viewport_request{};
    std::span<yuengine::runtimeasset::RuntimeAssetSceneEntityRecord> scene_entity_output{};
};

struct PreviewHostSceneDocumentViewportResult final {
    PreviewHostStatus status = PreviewHostStatus::InvalidArgument;
    yuengine::world::WorldSceneAuthoringDocumentStatus scene_document_status =
        yuengine::world::WorldSceneAuthoringDocumentStatus::Success;
    yuengine::world::WorldSceneAuthoringDocumentState scene_document_state{};
    PreviewHostSceneDocumentViewportBlockedLayer blocked_layer =
        PreviewHostSceneDocumentViewportBlockedLayer::SceneAuthoringDocument;
    PreviewHostViewportSessionResult viewport{};
    std::uint32_t source_entity_count = 0U;
    std::uint32_t exported_identity_count = 0U;
    std::uint32_t exported_transform_count = 0U;
    std::uint32_t exported_attachment_count = 0U;
    std::uint32_t exported_binding_count = 0U;
    std::uint32_t exported_dependency_count = 0U;
    std::uint32_t updated_scene_entity_count = 0U;
    bool consumed_scene_authoring_document = false;
    bool exported_runtime_records = false;
    bool consumed_runtime_scene_entities = false;
    bool updated_scene_entities_from_document = false;
    bool built_viewport_session = false;
    bool preserved_resource_browser_selection = false;
    bool emitted_transform_feedback = false;
};

class PreviewHost final {
public:
    PreviewHostStatus StartSession(
        const PreviewHostSessionDesc &desc,
        PreviewHostSessionResult *out_result);
    PreviewHostStatus StopSession(
        PreviewHostSessionId session,
        PreviewHostSessionResult *out_result);
    PreviewHostStatus BuildFrame(
        const PreviewHostFrameRequest &request,
        PreviewHostFrameResult *out_result) const;
    PreviewHostStatus BuildViewportSessionSurface(
        const PreviewHostViewportSessionRequest &request,
        PreviewHostViewportSessionResult *out_result) const;
    PreviewHostStatus BuildSceneDocumentViewportSession(
        const PreviewHostSceneDocumentViewportRequest &request,
        PreviewHostSceneDocumentViewportResult *out_result) const;
    PreviewHostStatus ResolveResourceBrowserPreview(
        const PreviewHostResourceBrowserPreviewRequest &request,
        PreviewHostResourceBrowserPreviewResult *out_result) const;

private:
    struct SessionSlot final {
        PreviewHostSessionId id{};
        PreviewHostDocumentKind document_kind = PreviewHostDocumentKind::Unknown;
        bool active = false;
    };

    std::array<SessionSlot, MAX_PREVIEW_HOST_SESSIONS> sessions_{};
    std::uint32_t next_generation_ = 1U;

    std::size_t ActiveSessionCount() const;
    bool IsSessionActive(
        PreviewHostSessionId session,
        PreviewHostDocumentKind document_kind) const;
};
}
