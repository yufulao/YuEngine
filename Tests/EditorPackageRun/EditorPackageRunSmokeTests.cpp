// Module: Tests EditorPackageRun
// File: Tests/EditorPackageRun/EditorPackageRunSmokeTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/AnimationEditor/AnimationEditorSurface.h"
#include "YuEngine/Asset/AssetManager.h"
#include "YuEngine/EditorPackageRun/EditorPackageRunSmoke.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/Package/PackageArtifact.h"
#include "YuEngine/Package/PackageEntryDescriptor.h"
#include "YuEngine/Package/PackageSourceKey.h"
#include "YuEngine/PreviewHost/PreviewHost.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/ResourceBrowser/ResourceBrowserSurface.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"
#include "YuEngine/SceneEditor/SceneEditorSurface.h"
#include "YuEngine/UiEditor/UiEditorSurface.h"
#include "YuEngine/World/WorldSceneAuthoringDocument.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"

namespace {
using yuengine::animation::AnimationRuntimeChannel;
using yuengine::animation::AnimationRuntimeClipRecord;
using yuengine::animation::AnimationRuntimeInterpolation;
using yuengine::animation::AnimationRuntimeKeyframeRecord;
using yuengine::animation::AnimationRuntimeTrackRecord;
using yuengine::animationeditor::AnimationEditorPreviewFeedbackRecord;
using yuengine::animationeditor::AnimationEditorSurfaceStatus;
using yuengine::animationeditor::AnimationEditorTimelineClipRow;
using yuengine::animationeditor::AnimationEditorTimelineKeyframeMarker;
using yuengine::animationeditor::AnimationEditorTimelineSelectionFeedbackRecord;
using yuengine::animationeditor::AnimationEditorTimelineTrackRow;
using yuengine::animationeditor::AnimationEditorTimelineWorkflowCommand;
using yuengine::animationeditor::AnimationEditorTimelineWorkflowRequest;
using yuengine::animationeditor::AnimationEditorTimelineWorkflowResult;
using yuengine::animationeditor::BuildAnimationEditorTimelineWorkflow;
using yuengine::asset::AssetHandle;
using yuengine::asset::AssetManager;
using yuengine::asset::AssetTypeId;
using yuengine::editorpackagerun::AuthoredEditorPackageRunBlockedLayer;
using yuengine::editorpackagerun::AuthoredEditorPackageRunRequest;
using yuengine::editorpackagerun::AuthoredEditorPackageRunResult;
using yuengine::editorpackagerun::AuthoredEditorPackageRunStatus;
using yuengine::editorpackagerun::RunAuthoredEditorPackageRunSmoke;
using yuengine::file::FileReadResult;
using yuengine::file::FileStatus;
using yuengine::file::MountId;
using yuengine::file::MountTable;
using yuengine::file::VirtualPath;
using yuengine::kernel::RuntimeFrameContext;
using yuengine::kernel::RuntimeFrameMode;
using yuengine::kernel::RuntimeFramePhase;
using yuengine::object::ObjectHandle;
using yuengine::package::PackageArtifactDependency;
using yuengine::package::PackageArtifactWriteRequest;
using yuengine::package::PackageEntryDescriptor;
using yuengine::package::PackageEntryId;
using yuengine::package::PackageId;
using yuengine::package::PackageSourceKey;
using yuengine::package::PackageStatus;
using yuengine::package::WritePackageArtifact;
using yuengine::previewhost::PreviewHostEditorViewportInteractionResult;
using yuengine::previewhost::PreviewHostFrameFormat;
using yuengine::previewhost::PreviewHostFrameResult;
using yuengine::previewhost::PreviewHostStatus;
using yuengine::previewhost::PreviewHostTransformFeedback;
using yuengine::previewhost::PreviewHostViewportSessionResult;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLogicalKey;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceTypeId;
using yuengine::resourcebrowser::ResourceBrowserDependencyState;
using yuengine::resourcebrowser::ResourceBrowserSurfaceDocumentKind;
using yuengine::resourcebrowser::ResourceBrowserSurfacePreviewState;
using yuengine::resourcebrowser::ResourceBrowserSurfaceSelectionState;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::NullRhiDevice;
using yuengine::rhi::RhiBlendStateDesc;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiColorTargetDesc;
using yuengine::rhi::RhiCommandList;
using yuengine::rhi::RhiConstantBufferBinding;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiDeviceSnapshot;
using yuengine::rhi::RhiDrawDesc;
using yuengine::rhi::RhiDrawIndexedDesc;
using yuengine::rhi::RhiFenceHandle;
using yuengine::rhi::RhiFormat;
using yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiPipelineDesc;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveRetirementDrainRequest;
using yuengine::rhi::RhiPrimitiveRetirementDrainResult;
using yuengine::rhi::RhiPrimitiveRetirementRecord;
using yuengine::rhi::RhiPrimitiveRetirementRequest;
using yuengine::rhi::RhiSampledTextureBinding;
using yuengine::rhi::RhiSamplerBinding;
using yuengine::rhi::RhiSamplerDesc;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiShaderModuleDesc;
using yuengine::rhi::RhiShaderModuleHandle;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiSwapchainResizeRequest;
using yuengine::rhi::RhiSwapchainResizeResult;
using yuengine::rhi::RhiTextureDesc;
using yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::RhiVertexBufferView;
using yuengine::runtimeasset::ExecuteRuntimeAssetImportCookCommand;
using yuengine::runtimeasset::RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT;
using yuengine::runtimeasset::RuntimeAssetDataStatus;
using yuengine::runtimeasset::RuntimeAssetFileDesc;
using yuengine::runtimeasset::RuntimeAssetFileKind;
using yuengine::runtimeasset::RuntimeAssetImportCookCommandKind;
using yuengine::runtimeasset::RuntimeAssetImportCookCommandRequest;
using yuengine::runtimeasset::RuntimeAssetImportCookCommandResult;
using yuengine::runtimeasset::RuntimeAssetLoadedFile;
using yuengine::runtimeasset::RuntimeAssetLoadedShaderProgramData;
using yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunMissingLayer;
using yuengine::runtimeasset::RuntimeAssetPackageArtifactProductRunRequest;
using yuengine::runtimeasset::RuntimeAssetPackagedRunBlockedLayer;
using yuengine::runtimeasset::RuntimeAssetPackagedRunRequest;
using yuengine::runtimeasset::RuntimeAssetSceneCameraRecord;
using yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using yuengine::runtimeasset::RuntimeAssetSceneLoaderOutput;
using yuengine::runtimeasset::RuntimeAssetSceneResourceRef;
using yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord;
using yuengine::sceneeditor::BuildSceneEditorUsableWorkflowSurface;
using yuengine::sceneeditor::SceneEditorHierarchyRow;
using yuengine::sceneeditor::SceneEditorInspectorRow;
using yuengine::sceneeditor::SceneEditorTransformCommandMode;
using yuengine::sceneeditor::SceneEditorTransformLedgerRecord;
using yuengine::sceneeditor::SceneEditorWorkflowLedgerRecord;
using yuengine::sceneeditor::SceneEditorWorkflowRequest;
using yuengine::sceneeditor::SceneEditorWorkflowResult;
using yuengine::sceneeditor::SceneEditorWorkflowStatus;
using yuengine::uieditor::BuildUiEditorDesignInspectorWorkflowSurface;
using yuengine::uieditor::UiEditorComponentKind;
using yuengine::uieditor::UiEditorDesignCommand;
using yuengine::uieditor::UiEditorDesignCommandKind;
using yuengine::uieditor::UiEditorDesignCommandLedgerRecord;
using yuengine::uieditor::UiEditorDesignInspectorWorkflowRequest;
using yuengine::uieditor::UiEditorDesignInspectorWorkflowResult;
using yuengine::uieditor::UiEditorDesignWorkflowStatus;
using yuengine::uieditor::UiEditorDesignSurfaceRow;
using yuengine::uieditor::UiEditorHierarchyRow;
using yuengine::uieditor::UiEditorInspectorFieldRow;
using yuengine::uieditor::UiEditorPreviewFeedbackRecord;
using yuengine::uieditor::UiEditorRuntimeDocument;
using yuengine::uieditor::UiEditorRuntimeDocumentHeader;
using yuengine::uieditor::UiEditorRuntimeNodeRecord;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UiRectTransform;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldSceneAuthoringDocument;
using yuengine::world::WorldSceneEditorSidecarExportPolicy;
using yuengine::world::WorldSceneEditorSidecarKind;
using yuengine::world::WorldSceneEditorSidecarRecord;
using yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;
using yuengine::world::WorldTransformState;

constexpr const char *TEST_AUTHORED_PACKAGE_RUN =
    "EditorPackageRun_ConsumesAuthoredWorkflowOutputsAndRunsProductCommand";
constexpr const char *TEST_MISSING_UI =
    "EditorPackageRun_MissingUiWorkflowStopsBeforePackageCommand";
constexpr const char *TEST_MISSING_ARTIFACT =
    "EditorPackageRun_MissingPackageArtifactReportsFileLayerWithoutEntryPoint";
constexpr const char *MOUNT_ID = "runtime";
constexpr PackageId RUNTIME_ASSET_SMOKE_PACKAGE{7001U};
constexpr PackageEntryId RUNTIME_ASSET_SMOKE_SCENE_ENTRY{100U};
constexpr const char *RUNTIME_ASSET_SMOKE_PACKAGE_ARTIFACT_PATH =
    "Package/AuthoredEditorSmoke.yupackage";
constexpr const char *RUNTIME_ASSET_MISSING_PACKAGE_ARTIFACT_PATH =
    "Package/MissingAuthoredEditorSmoke.yupackage";
constexpr std::uint64_t HALF_SECOND_NANOSECONDS = 500000000ULL;
constexpr std::size_t CAPTURE_BYTES_PER_ENTITY = 64U;
constexpr std::size_t TOTAL_CAPTURE_BYTES = CAPTURE_BYTES_PER_ENTITY * 3U;
constexpr std::uint32_t CLIP_ID = 4101U;
constexpr std::uint32_t TRACK_ID = 4201U;
constexpr std::uint32_t WORLD_OBJECT_VALUE = 4301U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool FailStep(std::string_view message) {
    Fail(message);
    return false;
}

std::filesystem::path TestRoot(std::string_view test_name) {
    std::filesystem::path root = std::filesystem::temp_directory_path();
    root /= "YuEngineEditorPackageRunTests";
    root /= std::string(test_name);
    return root;
}

class SmokeRhiDevice final : public IRhiDevice {
public:
    RhiStatus Initialize(const RhiDeviceDesc &desc) override {
        const RhiStatus status = device_.Initialize(desc);
        if (status != RhiStatus::Success) {
            return status;
        }

        RhiColorTargetDesc target_desc{};
        target_desc.format = RhiFormat::Rgba8Unorm;
        target_desc.extent = {2U, 2U};
        const RhiStatus target_status = device_.CreateColorTarget(target_desc, swapchain_target_);
        if (target_status != RhiStatus::Success) {
            return target_status;
        }

        swapchain_extent_ = target_desc.extent;
        swapchain_valid_ = true;
        return RhiStatus::Success;
    }

    RhiStatus CreateColorTarget(const RhiColorTargetDesc &desc, RhiTextureHandle &out_handle) override {
        return device_.CreateColorTarget(desc, out_handle);
    }

    RhiStatus GetSwapchainColorTarget(RhiTextureHandle &out_handle) const override {
        out_handle = RhiTextureHandle{};
        if (!swapchain_valid_) {
            return RhiStatus::UnsupportedBackend;
        }

        out_handle = swapchain_target_;
        return RhiStatus::Success;
    }

    RhiStatus ResizeSwapchain(
        const RhiSwapchainResizeRequest &request,
        RhiSwapchainResizeResult &out_result) override {
        out_result = RhiSwapchainResizeResult{};
        RhiColorTargetDesc target_desc{};
        target_desc.format = RhiFormat::Rgba8Unorm;
        target_desc.extent = request.extent;
        const RhiStatus target_status = device_.CreateColorTarget(target_desc, swapchain_target_);
        out_result.status = target_status;
        if (target_status != RhiStatus::Success) {
            return target_status;
        }

        swapchain_extent_ = target_desc.extent;
        swapchain_valid_ = true;
        out_result.snapshot = Snapshot().swapchain;
        return RhiStatus::Success;
    }

    RhiStatus DestroyTarget(RhiTextureHandle handle) override {
        return device_.DestroyTarget(handle);
    }

    RhiStatus RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) override {
        return device_.RecordClear(command_list, handle, color);
    }

    RhiStatus RecordBindPipeline(RhiCommandList &command_list, RhiPipelineHandle handle) override {
        return device_.RecordBindPipeline(command_list, handle);
    }

    RhiStatus RecordBindVertexBuffer(RhiCommandList &command_list, const RhiVertexBufferView &view) override {
        return device_.RecordBindVertexBuffer(command_list, view);
    }

    RhiStatus RecordBindIndexBuffer(RhiCommandList &command_list, const RhiIndexBufferView &view) override {
        return device_.RecordBindIndexBuffer(command_list, view);
    }

    RhiStatus RecordBindSampledTexture(RhiCommandList &command_list, const RhiSampledTextureBinding &binding) override {
        return device_.RecordBindSampledTexture(command_list, binding);
    }

    RhiStatus RecordBindSampler(RhiCommandList &command_list, const RhiSamplerBinding &binding) override {
        return device_.RecordBindSampler(command_list, binding);
    }

    RhiStatus RecordBindConstantBuffer(
        RhiCommandList &command_list,
        const RhiConstantBufferBinding &binding) override {
        return device_.RecordBindConstantBuffer(command_list, binding);
    }

    RhiStatus RecordBindBlendState(RhiCommandList &command_list, const RhiBlendStateDesc &desc) override {
        return device_.RecordBindBlendState(command_list, desc);
    }

    RhiStatus RecordDraw(RhiCommandList &command_list, const RhiDrawDesc &desc) override {
        return device_.RecordDraw(command_list, desc);
    }

    RhiStatus RecordDrawIndexed(RhiCommandList &command_list, const RhiDrawIndexedDesc &desc) override {
        return device_.RecordDrawIndexed(command_list, desc);
    }

    RhiStatus Submit(const RhiCommandList &command_list) override {
        return device_.Submit(command_list);
    }

    RhiStatus Present() override {
        return device_.Present();
    }

    yuengine::rhi::RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination) override {
        return device_.CapturePresentedTarget(destination);
    }

    RhiStatus CreateBuffer(
        const RhiBufferDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiBufferHandle &out_handle) override {
        return device_.CreateBuffer(desc, initial_bytes, out_handle);
    }

    RhiStatus UpdateBuffer(
        RhiBufferHandle handle,
        std::span<const std::uint8_t> bytes,
        RhiFenceHandle &out_fence) override {
        return device_.UpdateBuffer(handle, bytes, out_fence);
    }

    RhiStatus DestroyBuffer(RhiBufferHandle handle) override {
        return device_.DestroyBuffer(handle);
    }

    RhiStatus CreateTexture(
        const RhiTextureDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiTextureHandle &out_handle) override {
        return device_.CreateTexture(desc, initial_bytes, out_handle);
    }

    RhiStatus UpdateTexture(
        RhiTextureHandle handle,
        std::span<const std::uint8_t> bytes,
        RhiFenceHandle &out_fence) override {
        return device_.UpdateTexture(handle, bytes, out_fence);
    }

    RhiStatus DestroyTexture(RhiTextureHandle handle) override {
        return device_.DestroyTexture(handle);
    }

    RhiStatus CreateSampler(const RhiSamplerDesc &desc, RhiSamplerHandle &out_handle) override {
        return device_.CreateSampler(desc, out_handle);
    }

    RhiStatus DestroySampler(RhiSamplerHandle handle) override {
        return device_.DestroySampler(handle);
    }

    RhiStatus CreateShaderModule(const RhiShaderModuleDesc &desc, RhiShaderModuleHandle &out_handle) override {
        return device_.CreateShaderModule(desc, out_handle);
    }

    RhiStatus DestroyShaderModule(RhiShaderModuleHandle handle) override {
        return device_.DestroyShaderModule(handle);
    }

    RhiStatus CreatePipeline(const RhiPipelineDesc &desc, RhiPipelineHandle &out_handle) override {
        return device_.CreatePipeline(desc, out_handle);
    }

    RhiStatus DestroyPipeline(RhiPipelineHandle handle) override {
        return device_.DestroyPipeline(handle);
    }

    RhiStatus RequestPrimitiveRetirement(
        const RhiPrimitiveRetirementRequest &request,
        RhiPrimitiveRetirementRecord &out_record) override {
        return device_.RequestPrimitiveRetirement(request, out_record);
    }

    RhiStatus QueryPrimitiveRetirement(
        std::uint64_t retirement_id,
        RhiPrimitiveRetirementRecord &out_record) const override {
        return device_.QueryPrimitiveRetirement(retirement_id, out_record);
    }

    RhiStatus DrainPrimitiveRetirements(
        const RhiPrimitiveRetirementDrainRequest &request,
        RhiPrimitiveRetirementDrainResult &out_result) override {
        return device_.DrainPrimitiveRetirements(request, out_result);
    }

    yuengine::rhi::RhiCapabilities Capabilities() const override {
        return device_.Capabilities();
    }

    RhiDeviceSnapshot Snapshot() const override {
        RhiDeviceSnapshot snapshot = device_.Snapshot();
        snapshot.swapchain.valid = swapchain_valid_;
        snapshot.swapchain.extent = swapchain_extent_;
        snapshot.swapchain.color_target = swapchain_target_;
        return snapshot;
    }

private:
    NullRhiDevice device_{};
    RhiTextureHandle swapchain_target_{};
    yuengine::rhi::RhiExtent2D swapchain_extent_{};
    bool swapchain_valid_ = false;
};

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
    transform.scale_x = 1.0F;
    transform.scale_y = 1.0F;
    transform.scale_z = 1.0F;
    return transform;
}

WorldSceneEditorSidecarRecord SelectionSidecar(WorldObjectId world_object_id) {
    return WorldSceneEditorSidecarRecord{
        WorldSceneEditorSidecarKind::Selection,
        WorldSceneEditorSidecarExportPolicy::EditorOnly,
        world_object_id,
        0U,
        1U};
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

struct SceneWorkflowEvidence final {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 2U> transforms{};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{};
    WorldSceneAuthoringDocument document{};
    ResourceBrowserSurfaceSelectionState selection{};
    PreviewHostViewportSessionResult session{};
    PreviewHostEditorViewportInteractionResult interaction{};
    std::array<SceneEditorHierarchyRow, 2U> hierarchy_rows{};
    std::array<SceneEditorInspectorRow, 1U> inspector_rows{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 2U> transform_output{};
    std::array<SceneEditorTransformLedgerRecord, 1U> transform_ledger{};
    std::array<SceneEditorWorkflowLedgerRecord, 1U> workflow_ledger{};
    SceneEditorWorkflowResult result{};
};

bool BuildSceneWorkflowEvidence(SceneWorkflowEvidence *evidence) {
    if (evidence == nullptr) {
        return FailStep("null scene evidence");
    }

    evidence->identities = {
        WorldSceneObjectTransformRestoreIdentityRecord{WorldObjectId{1U}, MakeObjectHandle(1U, 10U)},
        WorldSceneObjectTransformRestoreIdentityRecord{WorldObjectId{2U}, MakeObjectHandle(2U, 10U)}};
    evidence->transforms = {
        WorldSceneObjectTransformRestoreTransformRecord{WorldObjectId{1U}, Transform(10.0F)},
        WorldSceneObjectTransformRestoreTransformRecord{WorldObjectId{2U}, Transform(20.0F)}};
    evidence->sidecars = {SelectionSidecar(WorldObjectId{2U})};
    evidence->document.header.scene_document_id = 0x5001U;
    evidence->document.header.deterministic_document_hash = 0xABCDU;
    evidence->document.header.identity_record_count =
        static_cast<std::uint32_t>(evidence->identities.size());
    evidence->document.header.transform_record_count =
        static_cast<std::uint32_t>(evidence->transforms.size());
    evidence->document.header.sidecar_record_count =
        static_cast<std::uint32_t>(evidence->sidecars.size());
    evidence->document.identity_records = evidence->identities.data();
    evidence->document.transform_records = evidence->transforms.data();
    evidence->document.sidecar_records = evidence->sidecars.data();
    evidence->selection = ReadyResourceSelection();
    evidence->session = ReadyViewportSession(1U);
    evidence->interaction = ReadyViewportInteraction(WorldObjectId{2U}, 1U);

    SceneEditorWorkflowRequest request{};
    request.document = &evidence->document;
    request.resource_browser_selection = &evidence->selection;
    request.viewport_session = &evidence->session;
    request.viewport_interaction = &evidence->interaction;
    request.requested_transform = Transform(200.0F);
    request.transform_mode = SceneEditorTransformCommandMode::Apply;
    request.hierarchy_rows = evidence->hierarchy_rows;
    request.inspector_rows = evidence->inspector_rows;
    request.transform_output = evidence->transform_output;
    request.transform_ledger_output = evidence->transform_ledger;
    request.workflow_ledger_output = evidence->workflow_ledger;
    const SceneEditorWorkflowStatus status =
        BuildSceneEditorUsableWorkflowSurface(request, &evidence->result);
    if (status != SceneEditorWorkflowStatus::Success ||
        !evidence->result.Succeeded() ||
        !evidence->workflow_ledger[0U].committed_workflow) {
        return FailStep("scene workflow evidence failed");
    }

    return true;
}

RuntimeFrameContext FrameContextAt(
    std::uint64_t fixed_time_nanoseconds,
    std::uint64_t delta_time_nanoseconds = HALF_SECOND_NANOSECONDS) {
    RuntimeFrameContext context{};
    context.frame_index = 11U;
    context.delta_time_nanoseconds = delta_time_nanoseconds;
    context.fixed_time_nanoseconds = fixed_time_nanoseconds;
    context.frame_mode = RuntimeFrameMode::Fixed;
    return context;
}

WorldTransformState AnimationTransform(float translation_x, float rotation_y) {
    WorldTransformState transform{};
    transform.translation_x = translation_x;
    transform.rotation_y = rotation_y;
    transform.rotation_w = 1.0F;
    transform.scale_x = 1.0F;
    transform.scale_y = 1.0F;
    transform.scale_z = 1.0F;
    return transform;
}

AnimationRuntimeClipRecord ClipRecord() {
    AnimationRuntimeClipRecord clip{};
    clip.clip_id = CLIP_ID;
    clip.duration_seconds = 1.0F;
    clip.first_track_index = 0U;
    clip.track_count = 2U;
    clip.layer_count = 1U;
    clip.is_valid = true;
    return clip;
}

AnimationRuntimeTrackRecord TrackRecord(
    AnimationRuntimeChannel channel,
    std::size_t first_keyframe_index) {
    AnimationRuntimeTrackRecord track{};
    track.track_id = TRACK_ID + static_cast<std::uint32_t>(first_keyframe_index);
    track.target = WorldObjectId{WORLD_OBJECT_VALUE};
    track.channel = channel;
    track.interpolation = AnimationRuntimeInterpolation::Linear;
    track.first_keyframe_index = first_keyframe_index;
    track.keyframe_count = 2U;
    track.is_valid = true;
    return track;
}

AnimationRuntimeKeyframeRecord Keyframe(float time_seconds, float value) {
    AnimationRuntimeKeyframeRecord keyframe{};
    keyframe.time_seconds = time_seconds;
    keyframe.value = value;
    keyframe.is_valid = true;
    return keyframe;
}

PreviewHostTransformFeedback AnimationPreviewFeedback() {
    PreviewHostTransformFeedback feedback{};
    feedback.world_object_id = WorldObjectId{WORLD_OBJECT_VALUE};
    feedback.transform = AnimationTransform(5.0F, 0.5F);
    feedback.transform_available = true;
    return feedback;
}

struct AnimationWorkflowEvidence final {
    std::array<AnimationRuntimeClipRecord, 1U> clips{};
    std::array<AnimationRuntimeTrackRecord, 2U> tracks{};
    std::array<AnimationRuntimeKeyframeRecord, 4U> keyframes{};
    std::array<PreviewHostTransformFeedback, 1U> preview_feedback{};
    std::array<AnimationEditorTimelineClipRow, 1U> clip_rows{};
    std::array<AnimationEditorTimelineTrackRow, 2U> track_rows{};
    std::array<AnimationEditorTimelineKeyframeMarker, 4U> keyframe_markers{};
    std::array<AnimationEditorPreviewFeedbackRecord, 2U> preview_output{};
    std::array<AnimationEditorTimelineSelectionFeedbackRecord, 1U> selection_output{};
    AnimationEditorTimelineWorkflowResult result{};
};

bool BuildAnimationWorkflowEvidence(AnimationWorkflowEvidence *evidence) {
    if (evidence == nullptr) {
        return FailStep("null animation evidence");
    }

    evidence->clips = {ClipRecord()};
    evidence->tracks = {
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U),
        TrackRecord(AnimationRuntimeChannel::RotationY, 2U)};
    evidence->keyframes = {
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F),
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 1.0F)};
    evidence->preview_feedback = {AnimationPreviewFeedback()};

    AnimationEditorTimelineWorkflowRequest request{};
    request.command = AnimationEditorTimelineWorkflowCommand::Scrub;
    request.clip_id = CLIP_ID;
    request.clips = evidence->clips;
    request.tracks = evidence->tracks;
    request.keyframes = evidence->keyframes;
    request.frame_context = FrameContextAt(HALF_SECOND_NANOSECONDS);
    request.current_sample_time_seconds = 0.0F;
    request.requested_sample_time_seconds = 0.25F;
    request.selected_track_id = TRACK_ID;
    request.selected_keyframe_index = 0U;
    request.preview_transform_feedback = evidence->preview_feedback;
    request.clip_rows = evidence->clip_rows;
    request.track_rows = evidence->track_rows;
    request.keyframe_markers = evidence->keyframe_markers;
    request.preview_feedback_output = evidence->preview_output;
    request.selection_feedback_output = evidence->selection_output;
    const AnimationEditorSurfaceStatus status =
        BuildAnimationEditorTimelineWorkflow(request, &evidence->result);
    if (status != AnimationEditorSurfaceStatus::Success ||
        !evidence->result.Succeeded() ||
        !evidence->selection_output[0U].selected_track) {
        return FailStep("animation workflow evidence failed");
    }

    return true;
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

UiEditorRuntimeDocumentHeader UiHeader() {
    UiEditorRuntimeDocumentHeader header{};
    header.document_id = 5101U;
    header.schema_version = 1U;
    header.node_count = 2U;
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

PreviewHostFrameResult UiPreviewFrame() {
    PreviewHostFrameResult result{};
    result.status = PreviewHostStatus::Success;
    result.frame.frame_id = 71U;
    result.frame.width = 800U;
    result.frame.height = 600U;
    result.frame.format = PreviewHostFrameFormat::Headless;
    result.capture_bytes_written = 128U;
    result.submitted_render_scene_frame = true;
    result.headless_output = true;
    return result;
}

struct UiWorkflowEvidence final {
    std::array<UiEditorRuntimeNodeRecord, 2U> nodes{};
    UiEditorRuntimeDocument document{};
    PreviewHostFrameResult preview_frame{};
    std::array<UiEditorHierarchyRow, 2U> hierarchy_output{};
    std::array<UiEditorDesignSurfaceRow, 2U> design_output{};
    std::array<UiEditorInspectorFieldRow, 7U> inspector_output{};
    std::array<UiEditorPreviewFeedbackRecord, 1U> preview_output{};
    std::array<UiEditorRuntimeNodeRecord, 2U> staged_output{};
    std::array<UiEditorDesignCommandLedgerRecord, 1U> ledger_output{};
    UiEditorDesignInspectorWorkflowResult result{};
};

bool BuildUiWorkflowEvidence(UiWorkflowEvidence *evidence) {
    if (evidence == nullptr) {
        return FailStep("null ui evidence");
    }

    evidence->nodes = {RootNode(), ChildNode()};
    evidence->document.header = UiHeader();
    evidence->document.nodes = evidence->nodes;
    evidence->preview_frame = UiPreviewFrame();
    UiEditorDesignCommand command{};
    command.kind = UiEditorDesignCommandKind::SetEnabled;
    command.bool_value = false;
    command.command_sequence = 9U;

    UiEditorDesignInspectorWorkflowRequest request{};
    request.document = &evidence->document;
    request.selected_node_id = UiNodeId{2U};
    request.preview_frame = &evidence->preview_frame;
    request.command = command;
    request.hierarchy_output = evidence->hierarchy_output;
    request.design_surface_output = evidence->design_output;
    request.inspector_output = evidence->inspector_output;
    request.preview_feedback_output = evidence->preview_output;
    request.staged_document_output = evidence->staged_output;
    request.command_ledger_output = evidence->ledger_output;
    const UiEditorDesignWorkflowStatus status =
        BuildUiEditorDesignInspectorWorkflowSurface(request, &evidence->result);
    if (status != UiEditorDesignWorkflowStatus::Success ||
        !evidence->result.Succeeded() ||
        !evidence->result.staged_document_update ||
        !evidence->ledger_output[0U].command_applied) {
        return FailStep("ui workflow evidence failed");
    }

    return true;
}

bool CreateMountedTable(const std::filesystem::path &root, MountTable *out_table) {
    if (out_table == nullptr) {
        return FailStep("null mount table output");
    }

    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    MountTable table;
    const FileStatus mount_status = table.RegisterLooseMount(MountId(MOUNT_ID), root);
    if (mount_status != FileStatus::Success) {
        return FailStep("register loose mount failed");
    }

    *out_table = table;
    return true;
}

bool ReadFile(MountTable &table, const char *path, std::vector<std::uint8_t> *out_bytes) {
    if (out_bytes == nullptr) {
        return FailStep("null file output");
    }

    const FileReadResult read_result = table.Read({MountId(MOUNT_ID), VirtualPath(path)});
    if (!read_result.Succeeded()) {
        return FailStep("read file failed");
    }

    *out_bytes = read_result.bytes;
    return true;
}

struct GeneratedFixtureCommandContext final {
    MountTable table;
    std::array<RuntimeAssetFileDesc, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> source_files{};
    std::array<RuntimeAssetFileDesc, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> cooked_files{};
    RuntimeAssetImportCookCommandResult command{};
};

bool ExecuteGeneratedFixtureCommand(std::string_view root_name, GeneratedFixtureCommandContext *out_context) {
    if (out_context == nullptr) {
        return FailStep("null generated fixture context");
    }

    GeneratedFixtureCommandContext context{};
    if (!CreateMountedTable(TestRoot(root_name), &context.table)) {
        return FailStep("generated fixture mount setup failed");
    }

    RuntimeAssetImportCookCommandRequest request{};
    request.command = RuntimeAssetImportCookCommandKind::GenerateDeterministicDiskFixture;
    request.fixture.mount_table = &context.table;
    request.fixture.mount = MountId(MOUNT_ID);
    request.fixture.source_files = context.source_files.data();
    request.fixture.source_file_capacity =
        static_cast<std::uint32_t>(context.source_files.size());
    request.fixture.cooked_files = context.cooked_files.data();
    request.fixture.cooked_file_capacity =
        static_cast<std::uint32_t>(context.cooked_files.size());

    const RuntimeAssetDataStatus status =
        ExecuteRuntimeAssetImportCookCommand(request, &context.command);
    if (status != RuntimeAssetDataStatus::Success ||
        context.command.status != RuntimeAssetDataStatus::Success) {
        return FailStep("generated fixture command failed");
    }

    *out_context = context;
    return true;
}

struct ProductRunContext final {
    GeneratedFixtureCommandContext fixture;
    ResourceRegistry registry;
    AssetManager manager;
    SmokeRhiDevice device;
    std::array<RuntimeAssetLoadedFile, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> loaded_files{};
    std::array<RuntimeAssetSceneResourceRef, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT> scene_refs{};
    std::array<RuntimeAssetSceneCameraRecord, 1U> scene_cameras{};
    std::array<RuntimeAssetSceneEntityRecord, 3U> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, 3U> scene_transforms{};
    RuntimeAssetSceneLoaderOutput scene_output{};
    RuntimeAssetLoadedShaderProgramData shader_program{};
    std::array<std::uint8_t, 16U> scratch_bytes{};
    std::array<std::uint8_t, TOTAL_CAPTURE_BYTES * 2U> capture_bytes{};
};

bool BuildRuntimeAssetPackageEntryDescriptor(
    MountTable &table,
    const RuntimeAssetFileDesc &desc,
    PackageEntryId entry,
    PackageEntryDescriptor *out_descriptor) {
    if (out_descriptor == nullptr) {
        return FailStep("null package entry descriptor");
    }

    std::vector<std::uint8_t> bytes{};
    if (!ReadFile(table, desc.path, &bytes)) {
        return FailStep("package source was not readable");
    }

    const std::string logical_key = std::string("runtime_asset_") +
        std::to_string(desc.stable_id);
    const std::string source_key = std::string("cooked_record_") +
        std::to_string(desc.stable_id);
    out_descriptor->package = RUNTIME_ASSET_SMOKE_PACKAGE;
    out_descriptor->entry = entry;
    out_descriptor->type = desc.resource_type;
    out_descriptor->logical_key = ResourceLogicalKey(logical_key);
    out_descriptor->source_key = PackageSourceKey(source_key);
    out_descriptor->byte_size = static_cast<std::uint32_t>(bytes.size());
    return true;
}

bool WriteRuntimeAssetPackageArtifact(ProductRunContext *context) {
    if (context == nullptr) {
        return FailStep("null product run context");
    }

    std::array<PackageEntryDescriptor, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U>
        entries{};
    std::array<PackageArtifactDependency, RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT>
        dependencies{};
    const RuntimeAssetFileDesc &scene_desc = context->fixture.command.fixture.cooked_scene;
    if (!BuildRuntimeAssetPackageEntryDescriptor(
            context->fixture.table,
            scene_desc,
            RUNTIME_ASSET_SMOKE_SCENE_ENTRY,
            &entries[0U])) {
        return false;
    }

    for (std::uint32_t index = 0U;
         index < context->fixture.command.fixture.cooked_file_count;
         ++index) {
        const RuntimeAssetFileDesc &desc = context->fixture.cooked_files[index];
        const PackageEntryId entry{index + 1U};
        if (!BuildRuntimeAssetPackageEntryDescriptor(
                context->fixture.table,
                desc,
                entry,
                &entries[index + 1U])) {
            return false;
        }

        dependencies[index] = PackageArtifactDependency{RUNTIME_ASSET_SMOKE_SCENE_ENTRY, entry};
    }

    PackageArtifactWriteRequest write_request{};
    write_request.mount_table = &context->fixture.table;
    write_request.mount = MountId(MOUNT_ID);
    write_request.artifact_path = VirtualPath(RUNTIME_ASSET_SMOKE_PACKAGE_ARTIFACT_PATH);
    write_request.package = RUNTIME_ASSET_SMOKE_PACKAGE;
    write_request.entries = entries.data();
    write_request.entry_count = static_cast<std::uint32_t>(entries.size());
    write_request.dependencies = dependencies.data();
    write_request.dependency_count = context->fixture.command.fixture.cooked_file_count;
    const yuengine::package::PackageArtifactResult write_result =
        WritePackageArtifact(write_request);
    if (write_result.status != PackageStatus::Success ||
        !write_result.wrote_artifact ||
        write_result.artifact_byte_count == 0U) {
        return FailStep("package artifact write failed");
    }

    return true;
}

RuntimeAssetPackagedRunRequest BuildPackagedRunRequest(ProductRunContext *context) {
    RuntimeAssetPackagedRunRequest request{};
    request.mount_table = &context->fixture.table;
    request.mount = MountId(MOUNT_ID);
    request.scene = context->fixture.command.fixture.cooked_scene;
    request.files = context->fixture.cooked_files.data();
    request.file_count = static_cast<std::uint32_t>(context->fixture.cooked_files.size());
    request.resource_registry = &context->registry;
    request.asset_manager = &context->manager;
    request.rhi_device = &context->device;
    request.loaded_files = context->loaded_files.data();
    request.loaded_file_capacity = static_cast<std::uint32_t>(context->loaded_files.size());
    request.scene_resource_refs = context->scene_refs.data();
    request.scene_resource_ref_capacity = static_cast<std::uint32_t>(context->scene_refs.size());
    request.scene_cameras = context->scene_cameras.data();
    request.scene_camera_capacity = static_cast<std::uint32_t>(context->scene_cameras.size());
    request.scene_entities = context->scene_entities.data();
    request.scene_entity_capacity = static_cast<std::uint32_t>(context->scene_entities.size());
    request.scene_transforms = context->scene_transforms.data();
    request.scene_transform_capacity = static_cast<std::uint32_t>(context->scene_transforms.size());
    request.scene_output = &context->scene_output;
    request.shader_program = &context->shader_program;
    request.animation_frame_context.frame_index = 1U;
    request.animation_frame_context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    request.animation_frame_context.fixed_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    request.animation_frame_context.phase = RuntimeFramePhase::LoadOrCommitResources;
    request.scratch_bytes = context->scratch_bytes;
    request.capture_output = context->capture_bytes;
    request.capture_byte_budget_per_entity = CAPTURE_BYTES_PER_ENTITY;
    request.first_frame_id = 9700U;
    request.visual_frame_count = 2U;
    request.output_path = "Artifacts/EditorPackageRun/AuthoredSmoke.rvf";
    request.output_path_byte_count = 44U;
    request.runtime_app.frame_count = 1U;
    request.runtime_app.fixed_delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    return request;
}

ResourceLogicalKey RuntimeAssetSmokeSceneLogicalKey(const RuntimeAssetFileDesc &scene_desc) {
    return ResourceLogicalKey(
        std::string("runtime_asset_") + std::to_string(scene_desc.stable_id));
}

RuntimeAssetPackageArtifactProductRunRequest BuildProductRunCommandRequest(
    ProductRunContext *context,
    const char *artifact_path) {
    RuntimeAssetPackageArtifactProductRunRequest request{};
    request.mount_table = &context->fixture.table;
    request.mount = MountId(MOUNT_ID);
    request.package_artifact_path = VirtualPath(artifact_path);
    request.package = RUNTIME_ASSET_SMOKE_PACKAGE;
    request.scene_resource_type = context->fixture.command.fixture.cooked_scene.resource_type;
    request.scene_logical_key =
        RuntimeAssetSmokeSceneLogicalKey(context->fixture.command.fixture.cooked_scene);
    request.packaged_run = BuildPackagedRunRequest(context);
    request.packaged_run.package_load_plan = nullptr;
    return request;
}

bool BuildProductRunContext(std::string_view root_name, ProductRunContext *context) {
    if (context == nullptr) {
        return FailStep("null product run context");
    }

    if (!ExecuteGeneratedFixtureCommand(root_name, &context->fixture)) {
        return false;
    }

    const RhiStatus rhi_status = context->device.Initialize(RhiDeviceDesc{});
    if (rhi_status != RhiStatus::Success) {
        return FailStep("rhi init failed");
    }

    return true;
}

struct AuthoredWorkflowEvidence final {
    SceneWorkflowEvidence scene;
    AnimationWorkflowEvidence animation;
    UiWorkflowEvidence ui;
};

bool BuildAuthoredWorkflowEvidence(AuthoredWorkflowEvidence *evidence) {
    if (evidence == nullptr) {
        return FailStep("null authored workflow evidence");
    }

    if (!BuildSceneWorkflowEvidence(&evidence->scene)) {
        return false;
    }

    if (!BuildAnimationWorkflowEvidence(&evidence->animation)) {
        return false;
    }

    if (!BuildUiWorkflowEvidence(&evidence->ui)) {
        return false;
    }

    return true;
}

AuthoredEditorPackageRunRequest BuildAuthoredRunRequest(
    AuthoredWorkflowEvidence *evidence,
    ProductRunContext *context,
    RuntimeAssetPackageArtifactProductRunRequest *product_request) {
    AuthoredEditorPackageRunRequest request{};
    request.scene_workflow = &evidence->scene.result;
    request.scene_workflow_ledger = evidence->scene.workflow_ledger;
    request.scene_transform_output = evidence->scene.transform_output;
    request.animation_workflow = &evidence->animation.result;
    request.animation_track_rows = evidence->animation.track_rows;
    request.animation_selection_feedback = evidence->animation.selection_output;
    request.ui_workflow = &evidence->ui.result;
    request.ui_runtime_document = &evidence->ui.document;
    request.ui_staged_document = evidence->ui.staged_output;
    request.ui_command_ledger = evidence->ui.ledger_output;
    request.import_cook_result = &context->fixture.command;
    request.product_run_request = product_request;
    return request;
}

int EditorPackageRunConsumesAuthoredWorkflowOutputsAndRunsProductCommand() {
    AuthoredWorkflowEvidence evidence{};
    if (!BuildAuthoredWorkflowEvidence(&evidence)) {
        return Fail("authored workflow evidence setup failed");
    }

    ProductRunContext context{};
    if (!BuildProductRunContext("AuthoredEditorPackageRunSuccess", &context)) {
        return Fail("product run context setup failed");
    }

    if (!WriteRuntimeAssetPackageArtifact(&context)) {
        return Fail("package artifact setup failed");
    }

    RuntimeAssetPackageArtifactProductRunRequest product_request =
        BuildProductRunCommandRequest(
            &context,
            RUNTIME_ASSET_SMOKE_PACKAGE_ARTIFACT_PATH);
    AuthoredEditorPackageRunResult result{};
    const AuthoredEditorPackageRunStatus status =
        RunAuthoredEditorPackageRunSmoke(
            BuildAuthoredRunRequest(&evidence, &context, &product_request),
            &result);
    if (status != AuthoredEditorPackageRunStatus::Success ||
        !result.Succeeded() ||
        result.blocked_layer != AuthoredEditorPackageRunBlockedLayer::None ||
        result.runtime_status != RuntimeAssetDataStatus::Success ||
        result.product_run_layer != RuntimeAssetPackageArtifactProductRunMissingLayer::None) {
        return Fail("authored editor package run did not close");
    }

    if (!result.consumed_scene_editor_workflow ||
        !result.consumed_animation_editor_workflow ||
        !result.consumed_ui_editor_workflow ||
        !result.scene_document_output_available ||
        !result.animation_clip_binding_output_available ||
        !result.ui_runtime_document_output_available ||
        !result.consumed_runtime_asset_import_cook ||
        !result.import_cook_wrote_source_and_cooked_files) {
        return Fail("authored workflow or import/cook evidence was not consumed");
    }

    if (!result.package_artifact_read ||
        !result.package_registry_rebuilt ||
        !result.package_load_plan_resolved ||
        !result.packaged_runtime_entrypoint_executed ||
        !result.runtime_app_frame_loop_success ||
        result.product_run.packaged_run.blocked_layer != RuntimeAssetPackagedRunBlockedLayer::None ||
        result.package_load_plan_record_count !=
            RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT + 1U) {
        return Fail("package artifact product-run ledger was incomplete");
    }

    if (result.scene_hierarchy_row_count != 2U ||
        result.scene_transform_record_count != 2U ||
        result.animation_track_row_count != 2U ||
        result.animation_selection_feedback_count != 1U ||
        result.ui_runtime_node_count != 2U ||
        result.ui_staged_node_count != 2U ||
        result.ui_command_ledger_count != 1U ||
        result.wrote_fake_artifact ||
        result.opened_native_window ||
        result.used_forbidden_preview_path) {
        return Fail("authored editor package run evidence counters mismatch");
    }

    return 0;
}

int EditorPackageRunMissingUiWorkflowStopsBeforePackageCommand() {
    AuthoredWorkflowEvidence evidence{};
    if (!BuildAuthoredWorkflowEvidence(&evidence)) {
        return Fail("authored workflow evidence setup failed");
    }

    ProductRunContext context{};
    if (!BuildProductRunContext("AuthoredEditorPackageRunMissingUi", &context)) {
        return Fail("product run context setup failed");
    }

    if (!WriteRuntimeAssetPackageArtifact(&context)) {
        return Fail("package artifact setup failed");
    }

    RuntimeAssetPackageArtifactProductRunRequest product_request =
        BuildProductRunCommandRequest(
            &context,
            RUNTIME_ASSET_SMOKE_PACKAGE_ARTIFACT_PATH);
    AuthoredEditorPackageRunRequest request =
        BuildAuthoredRunRequest(&evidence, &context, &product_request);
    request.ui_workflow = nullptr;

    AuthoredEditorPackageRunResult result{};
    const AuthoredEditorPackageRunStatus status =
        RunAuthoredEditorPackageRunSmoke(request, &result);
    if (status != AuthoredEditorPackageRunStatus::MissingUiWorkflowOutput ||
        result.blocked_layer != AuthoredEditorPackageRunBlockedLayer::UiEditorWorkflow ||
        result.product_run.packaged_run_executed ||
        result.product_run.package_artifact_read ||
        result.package_artifact_read ||
        result.packaged_runtime_entrypoint_executed ||
        result.wrote_fake_artifact) {
        return Fail("missing ui workflow did not stop before package command");
    }

    if (!result.consumed_scene_editor_workflow ||
        !result.consumed_animation_editor_workflow ||
        result.consumed_ui_editor_workflow ||
        result.consumed_runtime_asset_import_cook) {
        return Fail("missing ui workflow consumed the wrong evidence");
    }

    return 0;
}

int EditorPackageRunMissingPackageArtifactReportsFileLayerWithoutEntryPoint() {
    AuthoredWorkflowEvidence evidence{};
    if (!BuildAuthoredWorkflowEvidence(&evidence)) {
        return Fail("authored workflow evidence setup failed");
    }

    ProductRunContext context{};
    if (!BuildProductRunContext("AuthoredEditorPackageRunMissingArtifact", &context)) {
        return Fail("product run context setup failed");
    }

    RuntimeAssetPackageArtifactProductRunRequest product_request =
        BuildProductRunCommandRequest(
            &context,
            RUNTIME_ASSET_MISSING_PACKAGE_ARTIFACT_PATH);
    AuthoredEditorPackageRunResult result{};
    const AuthoredEditorPackageRunStatus status =
        RunAuthoredEditorPackageRunSmoke(
            BuildAuthoredRunRequest(&evidence, &context, &product_request),
            &result);
    if (status != AuthoredEditorPackageRunStatus::ProductRunFailed ||
        result.blocked_layer !=
            AuthoredEditorPackageRunBlockedLayer::RuntimeAssetFilePackageRunCommand ||
        result.runtime_status != RuntimeAssetDataStatus::FileReadFailed ||
        result.product_run_layer != RuntimeAssetPackageArtifactProductRunMissingLayer::FileVfs ||
        result.package_artifact_read ||
        result.package_registry_rebuilt ||
        result.package_load_plan_resolved ||
        result.packaged_runtime_entrypoint_executed ||
        result.product_run.packaged_run_executed ||
        result.wrote_fake_artifact) {
        return Fail("missing package artifact layer was not reported cleanly");
    }

    if (!result.consumed_scene_editor_workflow ||
        !result.consumed_animation_editor_workflow ||
        !result.consumed_ui_editor_workflow ||
        !result.consumed_runtime_asset_import_cook) {
        return Fail("missing artifact path lost authored evidence");
    }

    return 0;
}

struct TestCase final {
    const char *name = nullptr;
    int (*function)() = nullptr;
};

constexpr std::array<TestCase, 3U> TESTS{
    TestCase{TEST_AUTHORED_PACKAGE_RUN, EditorPackageRunConsumesAuthoredWorkflowOutputsAndRunsProductCommand},
    TestCase{TEST_MISSING_UI, EditorPackageRunMissingUiWorkflowStopsBeforePackageCommand},
    TestCase{TEST_MISSING_ARTIFACT, EditorPackageRunMissingPackageArtifactReportsFileLayerWithoutEntryPoint}};

}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail("expected one test name");
    }

    const std::string_view requested(argv[1]);
    for (const TestCase &test : TESTS) {
        if (requested == test.name) {
            return test.function();
        }
    }

    return Fail("unknown test name");
}
