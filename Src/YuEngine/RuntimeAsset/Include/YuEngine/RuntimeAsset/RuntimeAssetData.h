// 模块: YuEngine RuntimeAsset
// 文件: Src/YuEngine/RuntimeAsset/Include/YuEngine/RuntimeAsset/RuntimeAssetData.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetStatus.h"
#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/File/FileStatus.h"
#include "YuEngine/File/MountId.h"
#include "YuEngine/File/VirtualPath.h"
#include "YuEngine/Kernel/RuntimeAppDesc.h"
#include "YuEngine/Kernel/RuntimeAppRunResult.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryKind.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameDrawRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameEntityRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameResult.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h"
#include "YuEngine/RenderScene/RenderSceneThreePrimitiveCaptureRoute.h"
#include "YuEngine/Package/PackageArtifact.h"
#include "YuEngine/Package/PackageId.h"
#include "YuEngine/Package/PackageLoadPlan.h"
#include "YuEngine/Package/PackageRegistryDesc.h"
#include "YuEngine/Package/PackageStatus.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiFormat.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"
#include "YuEngine/Rhi/RhiShaderStage.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"
#include "YuEngine/World/WorldSceneAuthoringDocument.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::asset {
class AssetManager;
}

namespace yuengine::file {
class MountTable;
}

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::runtimeasset {

/**
 * @brief Runtime asset source file family.
 */
enum class RuntimeAssetFileKind {
    Unknown,
    Mesh,
    Material,
    Texture,
    Shader,
    Scene,
    Animation,
    Camera
};

/**
 * @brief Runtime mesh geometry family parsed from RuntimeAsset mesh records.
 */
enum class RuntimeAssetMeshGeometryKind {
    Unknown,
    Cube,
    Cylinder,
    Cone
};

/**
 * @brief Runtime material alpha mode parsed from RuntimeAsset material records.
 */
enum class RuntimeAssetMaterialAlphaMode {
    Unknown,
    Opaque,
    Blend
};

/**
 * @brief RuntimeAsset v0 artifact class parsed from internal metadata.
 */
enum class RuntimeAssetArtifactClass {
    Unknown,
    Source,
    Cooked
};

/**
 * @brief Runtime asset data validation, cook, and load status.
 */
enum class RuntimeAssetDataStatus {
    Success,
    InvalidArgument,
    InvalidHeader,
    UnsupportedVersion,
    InvalidKind,
    InvalidSchema,
    InvalidCount,
    InvalidSize,
    InvalidAlignment,
    InvalidBounds,
    InvalidDependency,
    MissingDependency,
    DuplicateDependency,
    TypeMismatch,
    HashMismatch,
    UnsupportedFieldValue,
    CapacityExceeded,
    BudgetExceeded,
    FileReadFailed,
    FileWriteFailed,
    ResourceRegistrationFailed,
    ResourceLoadCommitFailed,
    ResourceResidencyFailed,
    CachePayloadStoreFailed,
    DecodePlanFailed,
    DecodeResultFailed,
    DecodedPayloadStoreFailed,
    AssetRegistrationFailed,
    ResourceDependencyFailed,
    AssetDependencyFailed,
    InvalidInputLayout,
    RhiShaderModuleFailed,
    RhiPipelineFailed,
    RhiTextureFailed,
    RhiSamplerFailed,
    RenderSceneMaterialFailed,
    RhiCaptureFailed
};

/**
 * @brief Import/cook command kinds exposed before editor UI work exists.
 */
enum class RuntimeAssetImportCookCommandKind {
    Unknown,
    GenerateDeterministicDiskFixture
};

/**
 * @brief First engine layer that blocked an import/cook command.
 */
enum class RuntimeAssetImportCookMissingLayer {
    None,
    Command,
    FileVfs,
    RuntimeAssetData,
    Resource,
    Asset
};

/**
 * @brief Coarse phase reported by the RuntimeAsset loader transaction.
 */
enum class RuntimeAssetLoadTransactionPhase {
    Preflight,
    ReadBytes,
    ParseHeader,
    ValidateRecord,
    ValidateDependencies,
    PreflightCommit,
    StageSceneOutput,
    CommitResources,
    CommitAssets,
    CommitPayloads,
    CommitDependencies,
    CommitSceneOutput
};

/**
 * @brief Rollback proof reported after a RuntimeAsset graph commit failure.
 */
enum class RuntimeAssetLoadTransactionRollbackStatus {
    NotRequired,
    NotAttempted,
    Success,
    AssetRollbackFailed,
    ResourceRollbackFailed,
    SnapshotMismatch
};

/**
 * @brief Describes one runtime asset file expected by a graph load request.
 */
struct RuntimeAssetFileDesc final {
    const char *path = nullptr;
    RuntimeAssetFileKind kind = RuntimeAssetFileKind::Unknown;
    yuengine::resource::ResourceTypeId resource_type;
    yuengine::asset::AssetTypeId asset_type;
    std::uint64_t stable_id = 0U;
    yuengine::resource::ResourceDecodePlanAssetClass decode_asset_class =
        yuengine::resource::ResourceDecodePlanAssetClass::Unknown;
    yuengine::resource::ResourceDecodeResultClass decode_result_class =
        yuengine::resource::ResourceDecodeResultClass::Unknown;
    std::uint32_t decoded_byte_count = 0U;
};

constexpr std::uint32_t RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT = 10U;

/**
 * @brief Requests source/cooked RuntimeAsset fixture generation through File/VFS.
 */
struct RuntimeAssetDeterministicDiskFixtureRequest final {
    yuengine::file::MountTable *mount_table = nullptr;
    yuengine::file::MountId mount;
    RuntimeAssetFileDesc *source_files = nullptr;
    std::uint32_t source_file_capacity = 0U;
    RuntimeAssetFileDesc *cooked_files = nullptr;
    std::uint32_t cooked_file_capacity = 0U;
};

/**
 * @brief Reports deterministic fixture paths, hashes, validation, and failure layer.
 */
struct RuntimeAssetDeterministicDiskFixtureResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetImportCookMissingLayer missing_layer = RuntimeAssetImportCookMissingLayer::None;
    yuengine::file::FileStatus file_status = yuengine::file::FileStatus::Success;
    RuntimeAssetDataStatus validation_status = RuntimeAssetDataStatus::Success;
    RuntimeAssetFileKind first_failed_kind = RuntimeAssetFileKind::Unknown;
    std::uint32_t first_failed_artifact_index = 0U;
    RuntimeAssetFileDesc source_scene{};
    RuntimeAssetFileDesc cooked_scene{};
    std::uint32_t source_file_count = 0U;
    std::uint32_t cooked_file_count = 0U;
    std::uint32_t source_artifact_write_count = 0U;
    std::uint32_t cooked_artifact_write_count = 0U;
    std::uint32_t validation_count = 0U;
    std::uint64_t source_scene_hash = 0U;
    std::uint64_t cooked_scene_hash = 0U;
    std::uint64_t source_graph_hash = 0U;
    std::uint64_t cooked_graph_hash = 0U;
    bool wrote_to_disk = false;
    bool validated_source_files = false;
    bool validated_cooked_files = false;
};

/**
 * @brief Import/cook command request. The first command is deterministic fixture generation.
 */
struct RuntimeAssetImportCookCommandRequest final {
    RuntimeAssetImportCookCommandKind command = RuntimeAssetImportCookCommandKind::Unknown;
    RuntimeAssetDeterministicDiskFixtureRequest fixture{};
};

/**
 * @brief Import/cook command result with explicit missing-layer diagnostics.
 */
struct RuntimeAssetImportCookCommandResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetImportCookMissingLayer missing_layer = RuntimeAssetImportCookMissingLayer::None;
    RuntimeAssetDeterministicDiskFixtureResult fixture{};
};

/**
 * @brief Reports validation status and deterministic identity for one source file.
 */
struct RuntimeAssetValidationResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetArtifactClass artifact_class = RuntimeAssetArtifactClass::Unknown;
    RuntimeAssetFileKind kind = RuntimeAssetFileKind::Unknown;
    std::uint32_t version = 0U;
    std::uint32_t schema_version = 0U;
    std::uint64_t hash = 0U;
    std::uint64_t identity_hash = 0U;
    std::uint64_t source_hash = 0U;
    std::uint64_t payload_hash = 0U;
    std::size_t byte_count = 0U;
    std::size_t dependency_count = 0U;
    std::uint32_t dependency_table_count = 0U;
    RuntimeAssetDataStatus first_failed_dependency_status = RuntimeAssetDataStatus::Success;
    std::uint32_t first_failed_dependency_index = 0U;
    std::uint32_t first_failed_dependency_token_index = 0U;
    RuntimeAssetFileKind first_failed_expected_kind = RuntimeAssetFileKind::Unknown;
    RuntimeAssetFileKind first_failed_actual_kind = RuntimeAssetFileKind::Unknown;
    std::uint32_t record_table_count = 0U;
    std::uint32_t record_table_byte_count = 0U;
    std::uint32_t payload_byte_count = 0U;
    std::uint32_t payload_alignment = 0U;
    std::uint32_t vertex_count = 0U;
    std::uint32_t index_count = 0U;
    RuntimeAssetMeshGeometryKind mesh_geometry_kind = RuntimeAssetMeshGeometryKind::Unknown;
    yuengine::rhi::RhiInputLayoutDesc mesh_input_layout{};
    yuengine::rhi::RhiIndexFormat mesh_index_format = yuengine::rhi::RhiIndexFormat::Unsupported;
    yuengine::rhi::RhiPrimitiveTopology mesh_topology =
        yuengine::rhi::RhiPrimitiveTopology::Unsupported;
    std::uint32_t mesh_vertex_stride_bytes = 0U;
    std::uint32_t mesh_index_stride_bytes = 0U;
    std::uint32_t texture_width = 0U;
    std::uint32_t texture_height = 0U;
    std::uint32_t texture_slot_count = 0U;
    std::uint32_t material_parameter_count = 0U;
    std::uint32_t material_base_color_rgba = 0U;
    std::uint32_t material_emissive_strength = 0U;
    std::uint32_t material_metallic = 0U;
    std::uint32_t material_roughness = 0U;
    std::uint32_t material_opacity = 0U;
    RuntimeAssetMaterialAlphaMode material_alpha_mode = RuntimeAssetMaterialAlphaMode::Unknown;
    std::uint32_t shader_stage_count = 0U;
    std::uint32_t shader_bytecode_byte_count = 0U;
    std::uint32_t shader_import_policy_count = 0U;
    std::uint64_t shader_import_policy_hash = 0U;
};

/**
 * @brief Reports Resource and Asset records created from one loaded file.
 */
struct RuntimeAssetLoadedFile final {
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    RuntimeAssetFileKind kind = RuntimeAssetFileKind::Unknown;
    yuengine::resource::ResourceTypeId resource_type;
    yuengine::asset::AssetTypeId asset_type;
    std::uint64_t stable_id = 0U;
    std::uint64_t hash = 0U;
    RuntimeAssetArtifactClass artifact_class = RuntimeAssetArtifactClass::Unknown;
    std::uint32_t schema_version = 0U;
    std::uint64_t identity_hash = 0U;
    std::uint64_t source_hash = 0U;
    std::uint64_t payload_hash = 0U;
    std::uint32_t byte_count = 0U;
    std::uint32_t record_table_count = 0U;
    std::uint32_t record_table_byte_count = 0U;
    std::uint32_t payload_byte_count = 0U;
    std::uint32_t payload_alignment = 0U;
    std::uint32_t vertex_count = 0U;
    std::uint32_t index_count = 0U;
    RuntimeAssetMeshGeometryKind mesh_geometry_kind = RuntimeAssetMeshGeometryKind::Unknown;
    yuengine::rhi::RhiInputLayoutDesc mesh_input_layout{};
    yuengine::rhi::RhiIndexFormat mesh_index_format = yuengine::rhi::RhiIndexFormat::Unsupported;
    yuengine::rhi::RhiPrimitiveTopology mesh_topology =
        yuengine::rhi::RhiPrimitiveTopology::Unsupported;
    std::uint32_t mesh_vertex_stride_bytes = 0U;
    std::uint32_t mesh_index_stride_bytes = 0U;
    std::uint32_t texture_width = 0U;
    std::uint32_t texture_height = 0U;
    std::uint32_t texture_slot_count = 0U;
    std::uint32_t material_parameter_count = 0U;
    std::uint32_t material_base_color_rgba = 0U;
    std::uint32_t material_emissive_strength = 0U;
    std::uint32_t material_metallic = 0U;
    std::uint32_t material_roughness = 0U;
    std::uint32_t material_opacity = 0U;
    RuntimeAssetMaterialAlphaMode material_alpha_mode = RuntimeAssetMaterialAlphaMode::Unknown;
    std::uint32_t shader_stage_count = 0U;
    std::uint32_t shader_bytecode_byte_count = 0U;
    std::uint32_t shader_import_policy_count = 0U;
    std::uint64_t shader_import_policy_hash = 0U;
    std::uint64_t cache_payload_id = 0U;
    std::uint64_t source_payload_logical_byte_count = 0U;
    std::uint64_t source_payload_window_byte_offset = 0U;
    std::uint64_t source_payload_window_byte_size = 0U;
    std::uint64_t decode_plan_payload_id = 0U;
    std::uint64_t decode_plan_payload_logical_byte_count = 0U;
    std::uint64_t decode_plan_id = 0U;
    std::uint64_t decode_result_id = 0U;
    std::uint64_t decoded_payload_id = 0U;
    std::uint64_t decoded_payload_logical_byte_count = 0U;
    yuengine::resource::ResourceDecodePlanAssetClass decode_asset_class =
        yuengine::resource::ResourceDecodePlanAssetClass::Unknown;
    yuengine::resource::ResourceDecodeResultClass decode_result_class =
        yuengine::resource::ResourceDecodeResultClass::Unknown;
    std::uint32_t decoded_byte_count = 0U;
    bool cache_payload_stored = false;
    bool source_payload_window_from_package = false;
    bool decode_plan_created = false;
    bool decode_result_committed = false;
    bool decoded_payload_stored = false;
};

/**
 * @brief Identifies a loaded runtime asset referenced by scene records.
 */
struct RuntimeAssetSceneResourceRef final {
    RuntimeAssetFileKind kind = RuntimeAssetFileKind::Unknown;
    std::uint64_t stable_id = 0U;
    std::uint32_t loaded_file_index = 0U;
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
};

/**
 * @brief Explicit asset dependency row exported by RuntimeAsset authoring data.
 */
struct RuntimeAssetDataAssetDependencyRecord final {
    std::uint64_t stable_resource_id = 0U;
    yuengine::asset::AssetHandle dependent_asset;
    yuengine::asset::AssetHandle dependency_asset;
    yuengine::resource::ResourceHandle expected_resource;
    yuengine::resource::ResourceTypeId expected_resource_type;
};

/**
 * @brief Ordered batch commit diagnostics for RuntimeAsset asset dependencies.
 */
struct RuntimeAssetDataAssetDependencyBatchResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    yuengine::asset::AssetStatus asset_status = yuengine::asset::AssetStatus::Success;
    std::uint32_t committed_dependency_edge_count = 0U;
    std::uint32_t first_failed_dependency_index = 0U;
};

/**
 * @brief Atomic traversal diagnostics for RuntimeAsset asset dependencies.
 */
struct RuntimeAssetDataAssetDependencyTraverseResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    yuengine::asset::AssetStatus asset_status = yuengine::asset::AssetStatus::Success;
    std::uint32_t copied_dependency_count = 0U;
    std::uint32_t required_dependency_count = 0U;
};

/**
 * @brief Type-filtered enumeration key for committed RuntimeAsset asset dependency rows.
 */
struct RuntimeAssetDataAssetDependencyTypeEnumerationRequest final {
    yuengine::asset::AssetManager *asset_manager = nullptr;
    yuengine::asset::AssetHandle dependent_asset;
    yuengine::resource::ResourceTypeId expected_resource_type;
};

/**
 * @brief Atomic type-filtered enumeration diagnostics for RuntimeAsset asset dependency rows.
 */
struct RuntimeAssetDataAssetDependencyTypeEnumerationResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    yuengine::asset::AssetStatus asset_status = yuengine::asset::AssetStatus::Success;
    std::uint32_t copied_dependency_count = 0U;
    std::uint32_t required_dependency_count = 0U;
};

/**
 * @brief Count-only snapshot diagnostics for RuntimeAsset asset dependency type filters.
 */
struct RuntimeAssetDataAssetDependencyTypeCountSnapshotResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    yuengine::asset::AssetStatus asset_status = yuengine::asset::AssetStatus::Success;
    std::uint32_t dependency_count = 0U;
};

/**
 * @brief Exact lookup key for a committed RuntimeAsset asset dependency row.
 */
struct RuntimeAssetDataAssetDependencyExactLookupRequest final {
    yuengine::asset::AssetManager *asset_manager = nullptr;
    RuntimeAssetDataAssetDependencyRecord dependency;
};

/**
 * @brief Exact lookup diagnostics for RuntimeAsset asset dependency rows.
 */
struct RuntimeAssetDataAssetDependencyExactLookupResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    yuengine::asset::AssetStatus asset_status = yuengine::asset::AssetStatus::Success;
    bool found = false;
};

/**
 * @brief Caller-owned asset handles paired with WorldSceneAuthoring runtime export rows.
 */
struct RuntimeAssetDataWorldSceneAuthoringAssetDependencyBatchRequest final {
    yuengine::asset::AssetManager *asset_manager = nullptr;
    const yuengine::world::WorldSceneAuthoringRuntimeExport *runtime_export = nullptr;
    yuengine::asset::AssetHandle dependent_asset;
    const yuengine::asset::AssetHandle *dependency_assets = nullptr;
    std::uint32_t dependency_asset_count = 0U;
};

/**
 * @brief Disk scene camera record emitted by the RuntimeAsset production loader.
 */
struct RuntimeAssetSceneCameraRecord final {
    std::uint32_t camera_id = 0U;
    bool is_active = false;
};

/**
 * @brief Disk scene entity record emitted after animation sampling.
 */
struct RuntimeAssetSceneEntityRecord final {
    std::uint32_t entity_id = 0U;
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState transform{};
    std::uint32_t mesh_ref_index = 0U;
    std::uint32_t material_ref_index = 0U;
    std::uint32_t texture_ref_index = 0U;
    std::uint32_t shader_ref_index = 0U;
    std::uint32_t camera_index = 0U;
    std::uint32_t animation_ref_index = 0U;
    bool is_visible = true;
    bool is_active = true;
};

/**
 * @brief Scene transform output record consumed by runtime scene builders.
 */
struct RuntimeAssetSceneTransformOutputRecord final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState transform{};
};

/**
 * @brief RuntimeAsset asset-internal target identity kind.
 */
enum class RuntimeAssetTargetIdentityKind {
    Unknown,
    SceneNode,
    ModelNode,
    SkeletonJoint
};

/**
 * @brief Asset-internal identity for scene, model, and skeleton animation targets.
 */
struct RuntimeAssetTargetIdentityRecord final {
    RuntimeAssetTargetIdentityKind kind = RuntimeAssetTargetIdentityKind::Unknown;
    std::uint64_t target_id = 0U;
    std::uint64_t parent_target_id = 0U;
    std::uint32_t scene_entity_id = 0U;
    std::uint32_t ordinal = 0U;
    bool is_valid = false;
};

/**
 * @brief Asset target identity resolved to runtime scene output rows.
 */
struct RuntimeAssetRuntimeInstanceMappingRecord final {
    RuntimeAssetTargetIdentityKind target_kind = RuntimeAssetTargetIdentityKind::Unknown;
    std::uint64_t target_id = 0U;
    std::uint32_t scene_entity_id = 0U;
    std::uint32_t scene_entity_index = 0U;
    std::uint32_t scene_transform_index = 0U;
    bool is_valid = false;
};

/**
 * @brief Asset-internal animation target property bound by RuntimeAsset tracks.
 */
enum class RuntimeAssetAnimationTargetProperty {
    Unknown,
    TransformTranslationX,
    TransformTranslationY,
    TransformTranslationZ,
    TransformRotationX,
    TransformRotationY,
    TransformRotationZ,
    TransformRotationW,
    TransformScaleX,
    TransformScaleY,
    TransformScaleZ
};

/**
 * @brief RuntimeAsset track binding from animation track id to target id and property.
 */
struct RuntimeAssetAnimationTrackTargetBindingRecord final {
    std::uint32_t track_id = 0U;
    std::uint64_t target_id = 0U;
    RuntimeAssetAnimationTargetProperty property = RuntimeAssetAnimationTargetProperty::Unknown;
    bool is_valid = false;
};

/**
 * @brief Deterministic RuntimeAsset scene loader output and diagnostics.
 */
struct RuntimeAssetSceneLoaderOutput final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    std::uint64_t scene_id = 0U;
    std::uint64_t scene_hash = 0U;
    std::uint32_t entity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t resource_ref_count = 0U;
    std::uint32_t camera_count = 0U;
    std::uint32_t target_identity_count = 0U;
    std::uint32_t runtime_instance_mapping_count = 0U;
    std::uint32_t animation_clip_count = 0U;
    std::uint32_t animation_track_count = 0U;
    std::uint32_t animation_target_binding_count = 0U;
    std::uint32_t animation_keyframe_count = 0U;
    std::uint32_t animation_sampled_value_count = 0U;
    std::uint32_t selected_animation_clip_id = 0U;
    std::uint32_t entity_capacity = 0U;
    std::uint32_t transform_capacity = 0U;
    std::uint32_t resource_ref_capacity = 0U;
    std::uint32_t camera_capacity = 0U;
    std::uint32_t target_identity_capacity = 0U;
    std::uint32_t runtime_instance_mapping_capacity = 0U;
    std::uint32_t animation_clip_capacity = 0U;
    std::uint32_t animation_track_capacity = 0U;
    std::uint32_t animation_target_binding_capacity = 0U;
    std::uint32_t animation_keyframe_capacity = 0U;
    std::uint32_t file_read_count = 0U;
    std::uint32_t dependency_count = 0U;
    std::uint32_t cache_payload_count = 0U;
    std::uint32_t decoded_payload_count = 0U;
    yuengine::animation::AnimationRuntimeStatus animation_sample_status =
        yuengine::animation::AnimationRuntimeStatus::MissingClip;
    yuengine::animation::AnimationRuntimeStatus animation_apply_status =
        yuengine::animation::AnimationRuntimeStatus::MissingSample;
};

/**
 * @brief Requests a disk-backed runtime asset graph load through File, Resource, and Asset.
 */
struct RuntimeAssetGraphLoadRequest final {
    yuengine::file::MountTable *mount_table = nullptr;
    yuengine::file::MountId mount;
    yuengine::file::VirtualPath scene_path;
    const yuengine::package::PackageLoadPlan *package_load_plan = nullptr;
    yuengine::resource::ResourceTypeId scene_resource_type;
    yuengine::asset::AssetTypeId scene_asset_type;
    std::uint64_t scene_stable_id = 0U;
    const RuntimeAssetFileDesc *files = nullptr;
    std::uint32_t file_count = 0U;
    yuengine::resource::ResourceRegistry *resource_registry = nullptr;
    yuengine::asset::AssetManager *asset_manager = nullptr;
    RuntimeAssetLoadedFile *loaded_files = nullptr;
    std::uint32_t loaded_file_capacity = 0U;
    RuntimeAssetSceneResourceRef *scene_resource_refs = nullptr;
    std::uint32_t scene_resource_ref_capacity = 0U;
    RuntimeAssetSceneCameraRecord *scene_cameras = nullptr;
    std::uint32_t scene_camera_capacity = 0U;
    RuntimeAssetSceneEntityRecord *scene_entities = nullptr;
    std::uint32_t scene_entity_capacity = 0U;
    RuntimeAssetSceneTransformOutputRecord *scene_transforms = nullptr;
    std::uint32_t scene_transform_capacity = 0U;
    RuntimeAssetTargetIdentityRecord *target_identities = nullptr;
    std::uint32_t target_identity_capacity = 0U;
    RuntimeAssetRuntimeInstanceMappingRecord *runtime_instance_mappings = nullptr;
    std::uint32_t runtime_instance_mapping_capacity = 0U;
    yuengine::animation::AnimationRuntimeClipRecord *animation_clips = nullptr;
    std::uint32_t animation_clip_capacity = 0U;
    yuengine::animation::AnimationRuntimeTrackRecord *animation_tracks = nullptr;
    std::uint32_t animation_track_capacity = 0U;
    RuntimeAssetAnimationTrackTargetBindingRecord *animation_target_bindings = nullptr;
    std::uint32_t animation_target_binding_capacity = 0U;
    yuengine::animation::AnimationRuntimeKeyframeRecord *animation_keyframes = nullptr;
    std::uint32_t animation_keyframe_capacity = 0U;
    RuntimeAssetSceneLoaderOutput *scene_output = nullptr;
    yuengine::kernel::RuntimeFrameContext animation_frame_context{};
    std::uint32_t selected_animation_clip_id = 0U;
    std::uint64_t animation_clip_start_time_nanoseconds = 0U;
};

/**
 * @brief Bounded commit plan summary produced before runtime mutation begins.
 */
struct RuntimeAssetLoadTransactionPlan final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetLoadTransactionPhase phase = RuntimeAssetLoadTransactionPhase::Preflight;
    std::uint32_t record_count = 0U;
    std::uint32_t dependency_count = 0U;
    std::uint32_t resource_commit_count = 0U;
    std::uint32_t asset_commit_count = 0U;
    std::uint32_t cache_payload_commit_count = 0U;
    std::uint32_t decoded_payload_commit_count = 0U;
    std::uint32_t dependency_edge_commit_count = 0U;
};

/**
 * @brief Diagnostics for graph-load transaction preflight and commit.
 */
struct RuntimeAssetLoadTransactionResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetLoadTransactionPhase phase = RuntimeAssetLoadTransactionPhase::Preflight;
    std::uint32_t first_failed_record_index = 0U;
    std::uint32_t first_failed_dependency_index = 0U;
    std::uint32_t committed_resource_count = 0U;
    std::uint32_t committed_asset_count = 0U;
    std::uint32_t committed_cache_payload_count = 0U;
    std::uint32_t committed_decoded_payload_count = 0U;
    std::uint32_t committed_dependency_edge_count = 0U;
    RuntimeAssetLoadTransactionRollbackStatus rollback_status =
        RuntimeAssetLoadTransactionRollbackStatus::NotRequired;
    std::uint32_t rolled_back_resource_count = 0U;
    std::uint32_t rolled_back_asset_count = 0U;
    std::uint32_t rolled_back_cache_payload_count = 0U;
    std::uint32_t rolled_back_decoded_payload_count = 0U;
    std::uint32_t rolled_back_dependency_edge_count = 0U;
    bool mutated_state = false;
    bool rollback_attempted = false;
    bool rollback_completed = false;
    bool snapshot_restored = false;
    bool no_output_mutation_proven = false;
};

/**
 * @brief Reports the closed-loop graph load result.
 */
struct RuntimeAssetGraphLoadResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetLoadTransactionPlan transaction_plan;
    RuntimeAssetLoadTransactionResult transaction_result;
    RuntimeAssetLoadedFile scene;
    std::uint32_t loaded_file_count = 0U;
    std::uint32_t file_read_count = 0U;
    std::uint32_t cache_payload_count = 0U;
    std::uint32_t decoded_payload_count = 0U;
    std::uint32_t package_payload_window_count = 0U;
    std::uint64_t package_payload_window_byte_count = 0U;
    std::uint32_t resource_dependency_count = 0U;
    std::uint32_t asset_dependency_count = 0U;
    std::size_t scene_dependency_count = 0U;
    std::uint32_t selected_animation_clip_id = 0U;
    std::uint32_t animation_sampled_value_count = 0U;
    yuengine::animation::AnimationRuntimeStatus animation_sample_status =
        yuengine::animation::AnimationRuntimeStatus::MissingClip;
    yuengine::animation::AnimationRuntimeStatus animation_apply_status =
        yuengine::animation::AnimationRuntimeStatus::MissingSample;
    RuntimeAssetTargetIdentityRecord last_failed_target_identity{};
    std::uint64_t last_failed_target_identity_scene_stable_id = 0U;
    yuengine::resource::ResourceTypeId last_failed_target_identity_resource_type{};
    yuengine::asset::AssetTypeId last_failed_target_identity_asset_type{};
    std::uint32_t last_failed_target_identity_table_capacity = 0U;
    std::uint32_t last_failed_target_identity_current_count = 0U;
    std::uint32_t last_required_target_identity_count = 0U;
    bool scene_registered = false;
    bool scene_references_runtime_asset_families = false;
};

/**
 * @brief Owns decoded RuntimeAsset shader program data before RHI upload.
 */
struct RuntimeAssetLoadedShaderProgramData final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    std::uint32_t program_id = 0U;
    RuntimeAssetValidationResult validation{};
    std::array<std::uint8_t, yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES> vertex_bytecode{};
    std::array<std::uint8_t, yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES> pixel_bytecode{};
    std::size_t vertex_bytecode_size = 0U;
    std::size_t pixel_bytecode_size = 0U;
    std::uint64_t vertex_bytecode_hash = 0U;
    std::uint64_t pixel_bytecode_hash = 0U;
    yuengine::rhi::RhiInputLayoutDesc input_layout{};
    std::uint32_t texture_slot_count = 0U;
};

/**
 * @brief RuntimeAsset-owned shader compiler backend kind.
 */
enum class RuntimeAssetShaderCompilerBackendKind {
    Unknown,
    DeterministicFixture,
    NativeHlsl
};

/**
 * @brief Request for compiling a runtime shader source record into loaded program data.
 */
struct RuntimeAssetShaderCompilerBackendRequest final {
    RuntimeAssetShaderCompilerBackendKind backend_kind =
        RuntimeAssetShaderCompilerBackendKind::Unknown;
    std::span<const std::uint8_t> source_bytes{};
    std::uint32_t program_id = 0U;
    std::uint64_t expected_import_policy_hash = 0U;
};

/**
 * @brief Reports RuntimeAsset shader compiler output and reflection identity.
 */
struct RuntimeAssetShaderCompilerBackendResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetShaderCompilerBackendKind backend_kind =
        RuntimeAssetShaderCompilerBackendKind::Unknown;
    std::uint64_t import_policy_hash = 0U;
    std::uint64_t vertex_bytecode_hash = 0U;
    std::uint64_t pixel_bytecode_hash = 0U;
    std::uint32_t compiled_shader_stage_count = 0U;
    std::uint32_t reflection_input_element_count = 0U;
    std::uint32_t reflection_texture_slot_count = 0U;
    bool compiled_program = false;
    RuntimeAssetLoadedShaderProgramData program{};
};

/**
 * @brief Requests a RuntimeAsset-owned shader program bridge into RHI primitives.
 */
struct RuntimeAssetShaderProgramPipelineRequest final {
    yuengine::rhi::IRhiDevice *device = nullptr;
    const RuntimeAssetLoadedShaderProgramData *program = nullptr;
};

/**
 * @brief Reports shader module and pipeline ownership created from RuntimeAsset bytecode.
 */
struct RuntimeAssetShaderProgramPipelineResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    std::uint32_t program_id = 0U;
    std::uint64_t vertex_bytecode_hash = 0U;
    std::uint64_t pixel_bytecode_hash = 0U;
    std::uint32_t texture_slot_count = 0U;
    yuengine::rhi::RhiShaderModuleHandle vertex_shader{};
    yuengine::rhi::RhiShaderModuleHandle pixel_shader{};
    yuengine::rhi::RhiPipelineHandle pipeline{};
    yuengine::rhi::RhiPipelineDesc pipeline_desc{};
};

/**
 * @brief Color-space tag carried by cooked RuntimeAsset texture payload descriptors.
 */
enum class RuntimeAssetCookedTextureColorSpace {
    Unknown,
    Linear,
    Srgb
};

/**
 * @brief Describes one cooked texture payload that can be verified before RHI upload.
 */
struct RuntimeAssetCookedTexturePayloadDesc final {
    const RuntimeAssetLoadedFile *loaded_texture = nullptr;
    yuengine::rhi::RhiTextureDesc texture_desc{};
    RuntimeAssetCookedTextureColorSpace color_space = RuntimeAssetCookedTextureColorSpace::Unknown;
    std::uint32_t row_pitch_bytes = 0U;
    std::uint32_t slice_pitch_bytes = 0U;
    std::uint32_t payload_offset_bytes = 0U;
    std::uint32_t payload_byte_count = 0U;
    std::uint32_t payload_alignment_bytes = 1U;
    std::uint64_t payload_hash = 0U;
    std::uint64_t decoded_payload_id = 0U;
    std::uint64_t staging_request_id = 0U;
    std::uint64_t upload_id = 0U;
};

/**
 * @brief Describes one material texture slot that references a cooked texture payload.
 */
struct RuntimeAssetCookedMaterialSlotDesc final {
    std::uint32_t material_slot = 0U;
    std::uint32_t texture_payload_index = 0U;
    yuengine::rhi::RhiFormat expected_format = yuengine::rhi::RhiFormat::Rgba8Unorm;
    RuntimeAssetCookedTextureColorSpace expected_color_space = RuntimeAssetCookedTextureColorSpace::Unknown;
    std::uint32_t texture_binding_slot = 0U;
    std::uint32_t sampler_binding_slot = 0U;
    yuengine::rhi::RhiSamplerDesc sampler_desc{};
};

constexpr std::size_t RUNTIME_ASSET_PACKED_MATERIAL_CONSTANT_BYTES = 16U;

/**
 * @brief Packed material parameter bytes ready for RenderScene and RenderCore material requests.
 */
struct RuntimeAssetPackedMaterialConstants final {
    std::array<
        std::uint8_t,
        yuengine::renderscene::MAX_RENDER_SCENE_RUNTIME_MATERIAL_CONSTANT_BYTES> bytes{};
    std::size_t byte_count = 0U;
    std::uint64_t hash = 0U;
};

/**
 * @brief Requests a cooked texture payload bridge into RHI textures and RenderScene material slots.
 */
struct RuntimeAssetCookedTextureMaterialBridgeRequest final {
    yuengine::resource::ResourceRegistry *resource_registry = nullptr;
    yuengine::asset::AssetManager *asset_manager = nullptr;
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    const RuntimeAssetLoadedFile *loaded_material = nullptr;
    yuengine::asset::AssetHandle material_asset{};
    std::uint32_t material_id = 0U;
    yuengine::rhi::RhiPipelineHandle pipeline{};
    std::span<const RuntimeAssetCookedTexturePayloadDesc> textures{};
    std::span<const RuntimeAssetCookedMaterialSlotDesc> material_slots{};
    std::span<std::uint8_t> scratch_bytes{};
    yuengine::renderscene::RenderSceneRuntimeMaterialRecord *out_material = nullptr;
};

/**
 * @brief Reports cooked payload bridge validation, upload, material binding, and cleanup state.
 */
struct RuntimeAssetCookedTextureMaterialBridgeResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    yuengine::resource::ResourceDecodedPayloadStatus decoded_payload_status =
        yuengine::resource::ResourceDecodedPayloadStatus::Success;
    yuengine::streaming::ResourceDecodedTextureBridgeStatus texture_bridge_status =
        yuengine::streaming::ResourceDecodedTextureBridgeStatus::Success;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::Success;
    yuengine::asset::AssetStatus asset_status = yuengine::asset::AssetStatus::Success;
    yuengine::renderscene::RenderSceneRuntimeMaterialStatus material_status =
        yuengine::renderscene::RenderSceneRuntimeMaterialStatus::Success;
    std::uint32_t runtime_texture_upload_count = 0U;
    std::uint32_t material_texture_slot_count = 0U;
    std::size_t material_constant_byte_count = 0U;
    std::uint64_t material_constant_hash = 0U;
    std::uint32_t cleanup_texture_count = 0U;
    std::uint32_t cleanup_sampler_count = 0U;
    bool mutated_state = false;
    bool published_material = false;
};

/**
 * @brief Cooked shader bytecode format accepted by the RuntimeAsset bridge.
 */
enum class RuntimeAssetCookedShaderBytecodeFormat {
    Unknown,
    OpaqueBytecode
};

/**
 * @brief Runtime pipeline class declared by a cooked RuntimeAsset program.
 */
enum class RuntimeAssetCookedProgramPipelineClass {
    Unknown,
    Graphics
};

/**
 * @brief One cooked shader stage payload row owned by RuntimeAsset data.
 */
struct RuntimeAssetCookedShaderStagePayloadDesc final {
    yuengine::rhi::RhiShaderStage stage = yuengine::rhi::RhiShaderStage::Unsupported;
    const char *entry_point = nullptr;
    const char *bytecode_profile = nullptr;
    RuntimeAssetCookedShaderBytecodeFormat bytecode_format =
        RuntimeAssetCookedShaderBytecodeFormat::Unknown;
    std::uint64_t payload_id = 0U;
    const std::uint8_t *payload_bytes = nullptr;
    std::uint32_t payload_byte_count = 0U;
    std::uint32_t bytecode_offset = 0U;
    std::uint32_t bytecode_byte_count = 0U;
    std::uint32_t bytecode_alignment = 0U;
    std::uint64_t bytecode_hash = 0U;
    std::uint64_t expected_stage_hash = 0U;
};

/**
 * @brief Cooked shader program reflection and stage table used for RHI creation.
 */
struct RuntimeAssetCookedProgramDesc final {
    std::uint32_t program_id = 0U;
    RuntimeAssetCookedProgramPipelineClass pipeline_class =
        RuntimeAssetCookedProgramPipelineClass::Unknown;
    const RuntimeAssetCookedShaderStagePayloadDesc *stages = nullptr;
    std::uint32_t stage_count = 0U;
    yuengine::rhi::RhiInputLayoutDesc input_layout{};
    std::uint32_t vertex_stride_bytes = 0U;
    std::uint32_t texture_slot_count = 0U;
    std::uint32_t sampler_slot_count = 0U;
    std::array<yuengine::rhi::RhiInputElementSemantic, yuengine::rhi::MAX_RHI_INPUT_ELEMENTS>
        required_input_semantics{};
    std::uint32_t required_input_semantic_count = 0U;
    std::uint32_t constant_range_count = 0U;
};

/**
 * @brief Requests cooked RuntimeAsset shader/program RHI bridge creation.
 */
struct RuntimeAssetCookedShaderProgramPipelineRequest final {
    yuengine::rhi::IRhiDevice *device = nullptr;
    const RuntimeAssetCookedProgramDesc *program = nullptr;
};

/**
 * @brief Reports cooked shader/program bridge outputs and cleanup ledger counts.
 */
struct RuntimeAssetCookedShaderProgramPipelineResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    std::uint32_t program_id = 0U;
    std::uint64_t vertex_bytecode_hash = 0U;
    std::uint64_t pixel_bytecode_hash = 0U;
    std::uint32_t texture_slot_count = 0U;
    std::uint32_t sampler_slot_count = 0U;
    std::uint32_t preflight_stage_count = 0U;
    std::uint32_t created_shader_module_count = 0U;
    std::uint32_t destroyed_shader_module_count = 0U;
    bool published_handles = false;
    yuengine::rhi::RhiShaderModuleHandle vertex_shader{};
    yuengine::rhi::RhiShaderModuleHandle pixel_shader{};
    yuengine::rhi::RhiPipelineHandle pipeline{};
    yuengine::rhi::RhiPipelineDesc pipeline_desc{};
};

/**
 * @brief First missing runtime visual layer when proving cooked RuntimeAsset records.
 */
enum class RuntimeAssetVisualProofMissingLayer {
    None,
    Model,
    MaterialSlot,
    ShaderPipeline,
    SceneTransform,
    Camera,
    RhiCapture
};

/**
 * @brief Request to prove a cooked RuntimeAsset graph through RenderScene, RenderCore, and RHI.
 */
struct RuntimeAssetVisualProofRequest final {
    yuengine::resource::ResourceRegistry *resource_registry = nullptr;
    yuengine::asset::AssetManager *asset_manager = nullptr;
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    const RuntimeAssetLoadedFile *scene = nullptr;
    std::span<const RuntimeAssetLoadedFile> loaded_files{};
    std::span<const RuntimeAssetSceneCameraRecord> scene_cameras{};
    std::span<const RuntimeAssetSceneEntityRecord> scene_entities{};
    std::span<const RuntimeAssetSceneTransformOutputRecord> scene_transforms{};
    const RuntimeAssetSceneLoaderOutput *scene_output = nullptr;
    const RuntimeAssetLoadedShaderProgramData *shader_program = nullptr;
    std::span<std::uint8_t> scratch_bytes{};
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget_per_entity = 0U;
    std::uint32_t first_frame_id = 0U;
    std::uint32_t frame_count = 0U;
    const char *output_path = nullptr;
    std::size_t output_path_byte_count = 0U;
    bool require_cooked_records = true;
};

/**
 * @brief Result of the cooked RuntimeAsset visual proof route.
 */
struct RuntimeAssetVisualProofResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetVisualProofMissingLayer first_missing_layer =
        RuntimeAssetVisualProofMissingLayer::None;
    std::uint32_t cooked_record_count = 0U;
    std::uint32_t source_record_count = 0U;
    std::uint32_t mesh_record_count = 0U;
    std::uint32_t texture_record_count = 0U;
    std::uint32_t scene_entity_count = 0U;
    std::uint32_t scene_transform_count = 0U;
    std::uint32_t scene_camera_count = 0U;
    std::uint32_t animation_sampled_value_count = 0U;
    std::uint32_t material_texture_slot_count = 0U;
    std::uint32_t runtime_texture_upload_count = 0U;
    std::uint32_t mesh_decoded_payload_count = 0U;
    std::size_t mesh_vertex_payload_byte_count = 0U;
    std::size_t mesh_index_payload_byte_count = 0U;
    std::uint32_t submitted_draw_count = 0U;
    std::uint32_t completed_frame_count = 0U;
    std::size_t capture_bytes_written = 0U;
    bool loaded_records_verified = false;
    bool mesh_buffers_from_decoded_payloads = false;
    bool shader_pipeline_from_runtime_asset = false;
    bool material_slots_from_cooked_payloads = false;
    bool scene_transforms_from_animation_sampling = false;
    bool render_scene_routed = false;
    bool render_core_rhi_capture_routed = false;
    RuntimeAssetShaderProgramPipelineResult shader_pipeline_result{};
    RuntimeAssetCookedTextureMaterialBridgeResult material_result{};
    yuengine::renderscene::RenderSceneThreePrimitiveCaptureResult capture_result{};
};

/**
 * @brief Maps a loaded RuntimeAsset scene resource ref to a RenderScene geometry record.
 */
struct RuntimeAssetRenderSceneGeometryBinding final {
    std::uint32_t resource_ref_index = 0U;
    yuengine::renderscene::RenderScenePrimitiveGeometryRecord geometry{};
};

/**
 * @brief Maps a loaded RuntimeAsset scene resource ref to a RenderScene material record.
 */
struct RuntimeAssetRenderSceneMaterialBinding final {
    std::uint32_t resource_ref_index = 0U;
    yuengine::renderscene::RenderSceneRuntimeMaterialRecord material{};
};

/**
 * @brief Builds a CPU-side RenderScene frame submission from loaded RuntimeAsset scene records.
 */
struct RuntimeAssetRenderSceneSubmissionRequest final {
    const RuntimeAssetSceneLoaderOutput *scene_output = nullptr;
    std::span<const RuntimeAssetSceneEntityRecord> scene_entities{};
    std::span<const RuntimeAssetSceneTransformOutputRecord> scene_transforms{};
    std::span<const RuntimeAssetRenderSceneGeometryBinding> geometry_bindings{};
    std::span<const RuntimeAssetRenderSceneMaterialBinding> material_bindings{};
    yuengine::renderscene::RenderSceneCameraBindingResult camera{};
    std::span<yuengine::renderscene::RenderSceneRuntimeFrameEntityRequest> out_frame_entities{};
    std::span<yuengine::renderscene::RenderSceneRuntimeMaterialRecord> out_frame_materials{};
    std::span<yuengine::renderscene::RenderSceneRuntimeFrameDrawRecord> out_draws{};
    std::uint32_t frame_id = 0U;
    bool require_shared_material = true;
};

/**
 * @brief Reports CPU-side RuntimeAsset scene submission into RenderScene frame records.
 */
struct RuntimeAssetRenderSceneSubmissionResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    yuengine::renderscene::RenderSceneRuntimeFrameStatus frame_status =
        yuengine::renderscene::RenderSceneRuntimeFrameStatus::InvalidFrameId;
    yuengine::renderscene::RenderSceneRuntimeFrameResult frame_result{};
    std::uint32_t frame_id = 0U;
    std::uint32_t submitted_entity_count = 0U;
    std::uint32_t skipped_entity_count = 0U;
    std::uint32_t output_draw_count = 0U;
    std::uint32_t resolved_geometry_count = 0U;
    std::uint32_t resolved_material_count = 0U;
    std::uint32_t material_variant_count = 0U;
    std::uint32_t material_table_count = 0U;
    std::uint32_t shared_material_ref_index = 0xFFFFFFFFU;
    std::uint32_t first_failed_entity_index = 0xFFFFFFFFU;
    std::uint32_t first_missing_resource_ref_index = 0xFFFFFFFFU;
};

/**
 * @brief First layer that can block a packaged RuntimeAsset run entrypoint.
 */
enum class RuntimeAssetPackagedRunBlockedLayer {
    None,
    PackageLoadPlan,
    RuntimeAssetData,
    ResourceAsset,
    ShaderProgram,
    RenderSceneRenderCoreRhi,
    RuntimeAppFrameLoop
};

/**
 * @brief Reports packaged RuntimeAsset validation before graph-load mutation begins.
 */
struct RuntimeAssetPackagedValidationResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    yuengine::package::PackageStatus package_status =
        yuengine::package::PackageStatus::NotFound;
    std::uint32_t record_count = 0U;
    std::uint32_t validated_record_count = 0U;
    std::uint32_t first_failed_record_index = 0U;
    std::uint64_t archive_byte_count = 0ULL;
    std::uint64_t validated_archive_byte_count = 0ULL;
    bool dependency_records_validated = false;
    bool archive_ranges_validated = false;
    bool payload_hashes_validated = false;
    bool runtime_asset_payloads_validated = false;
    bool no_graph_mutation_required = true;
};

/**
 * @brief Request for the first engine-owned packaged RuntimeAsset run entrypoint.
 */
struct RuntimeAssetPackagedRunRequest final {
    yuengine::file::MountTable *mount_table = nullptr;
    yuengine::file::MountId mount;
    const yuengine::package::PackageLoadPlan *package_load_plan = nullptr;
    RuntimeAssetFileDesc scene{};
    const RuntimeAssetFileDesc *files = nullptr;
    std::uint32_t file_count = 0U;
    yuengine::resource::ResourceRegistry *resource_registry = nullptr;
    yuengine::asset::AssetManager *asset_manager = nullptr;
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    RuntimeAssetLoadedFile *loaded_files = nullptr;
    std::uint32_t loaded_file_capacity = 0U;
    RuntimeAssetSceneResourceRef *scene_resource_refs = nullptr;
    std::uint32_t scene_resource_ref_capacity = 0U;
    RuntimeAssetSceneCameraRecord *scene_cameras = nullptr;
    std::uint32_t scene_camera_capacity = 0U;
    RuntimeAssetSceneEntityRecord *scene_entities = nullptr;
    std::uint32_t scene_entity_capacity = 0U;
    RuntimeAssetSceneTransformOutputRecord *scene_transforms = nullptr;
    std::uint32_t scene_transform_capacity = 0U;
    RuntimeAssetTargetIdentityRecord *target_identities = nullptr;
    std::uint32_t target_identity_capacity = 0U;
    RuntimeAssetRuntimeInstanceMappingRecord *runtime_instance_mappings = nullptr;
    std::uint32_t runtime_instance_mapping_capacity = 0U;
    yuengine::animation::AnimationRuntimeClipRecord *animation_clips = nullptr;
    std::uint32_t animation_clip_capacity = 0U;
    yuengine::animation::AnimationRuntimeTrackRecord *animation_tracks = nullptr;
    std::uint32_t animation_track_capacity = 0U;
    RuntimeAssetAnimationTrackTargetBindingRecord *animation_target_bindings = nullptr;
    std::uint32_t animation_target_binding_capacity = 0U;
    yuengine::animation::AnimationRuntimeKeyframeRecord *animation_keyframes = nullptr;
    std::uint32_t animation_keyframe_capacity = 0U;
    RuntimeAssetSceneLoaderOutput *scene_output = nullptr;
    RuntimeAssetLoadedShaderProgramData *shader_program = nullptr;
    yuengine::kernel::RuntimeFrameContext animation_frame_context{};
    std::uint32_t selected_animation_clip_id = 0U;
    std::uint64_t animation_clip_start_time_nanoseconds = 0U;
    std::span<std::uint8_t> scratch_bytes{};
    std::span<std::uint8_t> capture_output{};
    std::span<RuntimeAssetRenderSceneGeometryBinding> generic_geometry_bindings{};
    std::span<RuntimeAssetRenderSceneMaterialBinding> generic_material_bindings{};
    std::span<yuengine::renderscene::RenderSceneRuntimeFrameEntityRequest> generic_frame_entities{};
    std::span<yuengine::renderscene::RenderSceneRuntimeMaterialRecord> generic_frame_materials{};
    std::span<yuengine::renderscene::RenderSceneRuntimeFrameDrawRecord> generic_draws{};
    std::size_t capture_byte_budget_per_entity = 0U;
    std::uint32_t first_frame_id = 0U;
    std::uint32_t visual_frame_count = 1U;
    const char *output_path = nullptr;
    std::size_t output_path_byte_count = 0U;
    yuengine::kernel::RuntimeAppDesc runtime_app{};
};

/**
 * @brief Result of the packaged RuntimeAsset run entrypoint.
 */
struct RuntimeAssetPackagedRunResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetPackagedRunBlockedLayer blocked_layer =
        RuntimeAssetPackagedRunBlockedLayer::PackageLoadPlan;
    yuengine::package::PackageStatus package_status =
        yuengine::package::PackageStatus::NotFound;
    RuntimeAssetPackagedValidationResult packaged_validation{};
    RuntimeAssetGraphLoadResult graph_load_result{};
    RuntimeAssetVisualProofResult visual_proof_result{};
    RuntimeAssetRenderSceneSubmissionResult generic_submission_result{};
    yuengine::kernel::RuntimeAppRunResult runtime_app_result{};
    std::uint32_t package_load_plan_record_count = 0U;
    std::uint32_t loaded_file_count = 0U;
    std::uint32_t resource_dependency_count = 0U;
    std::uint32_t asset_dependency_count = 0U;
    std::uint32_t resource_payload_window_count = 0U;
    std::uint64_t resource_payload_window_byte_count = 0U;
    std::uint32_t runtime_app_completed_frame_count = 0U;
    bool package_load_plan_consumed = false;
    bool runtime_asset_validation_load_success = false;
    bool resource_asset_registration_success = false;
    bool resource_payload_windows_loaded = false;
    bool shader_program_decoded = false;
    bool render_scene_render_core_rhi_success = false;
    bool generic_render_scene_submission_success = false;
    bool runtime_app_frame_loop_success = false;
    bool packaged_runtime_entrypoint_available = false;
};

/**
 * @brief First layer that can block a file-backed Package artifact product run command.
 */
enum class RuntimeAssetPackageArtifactProductRunMissingLayer {
    None,
    Command,
    FileVfs,
    PackageArtifact,
    PackageLoadPlan,
    PackagedRuntimeEntryPoint
};

/**
 * @brief Command request that starts a RuntimeAsset product run from a Package artifact path.
 */
struct RuntimeAssetPackageArtifactProductRunRequest final {
    yuengine::file::MountTable *mount_table = nullptr;
    yuengine::file::MountId mount;
    yuengine::file::VirtualPath package_artifact_path;
    yuengine::package::PackageId package;
    yuengine::package::PackageRegistryDesc package_registry{};
    yuengine::resource::ResourceTypeId scene_resource_type;
    yuengine::resource::ResourceLogicalKey scene_logical_key;
    RuntimeAssetPackagedRunRequest packaged_run{};
};

/**
 * @brief Product run command ledger from Package artifact read through RuntimeApp.
 */
struct RuntimeAssetPackageArtifactProductRunResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetPackageArtifactProductRunMissingLayer missing_layer =
        RuntimeAssetPackageArtifactProductRunMissingLayer::Command;
    yuengine::file::FileStatus file_status = yuengine::file::FileStatus::Success;
    yuengine::package::PackageStatus package_status =
        yuengine::package::PackageStatus::NotFound;
    yuengine::package::PackageArtifactResult package_artifact{};
    RuntimeAssetPackagedRunResult packaged_run{};
    std::uint32_t package_load_plan_record_count = 0U;
    bool package_artifact_read = false;
    bool package_registry_rebuilt = false;
    bool package_load_plan_resolved = false;
    bool packaged_run_executed = false;
};

/**
 * @brief Returns the file kind token used by the runtime asset header.
 * @param kind Input file family.
 * @return Static token string.
 */
const char *RuntimeAssetFileKindName(RuntimeAssetFileKind kind);
/**
 * @brief Computes a deterministic identity hash for runtime asset bytes.
 * @param bytes Input byte span.
 * @return Stable hash value.
 */
std::uint64_t HashRuntimeAssetDataBytes(std::span<const std::uint8_t> bytes);
/**
 * @brief Packs loaded RuntimeAsset material parameters into RenderScene material constant bytes.
 * @param material Loaded material record.
 * @param out_constants Output packed constants.
 * @return Explicit pack status.
 */
RuntimeAssetDataStatus PackRuntimeAssetMaterialConstants(
    const RuntimeAssetLoadedFile &material,
    RuntimeAssetPackedMaterialConstants *out_constants);
/**
 * @brief Validates runtime asset bytes without mutating runtime output state.
 * @param bytes Input byte span.
 * @param expected_kind Expected file family.
 * @param out_result Output validation result.
 * @return Explicit validation status.
 */
RuntimeAssetDataStatus ValidateRuntimeAssetDataBytes(
    std::span<const std::uint8_t> bytes,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetValidationResult *out_result);
/**
 * @brief Writes deterministic source and cooked RuntimeAsset fixture files through File/VFS.
 * @param request Input mount and descriptor output buffers.
 * @param out_result Output write, validation, and hash diagnostics.
 * @return Explicit command status.
 */
RuntimeAssetDataStatus GenerateRuntimeAssetDeterministicDiskFixture(
    const RuntimeAssetDeterministicDiskFixtureRequest &request,
    RuntimeAssetDeterministicDiskFixtureResult *out_result);
/**
 * @brief Executes a RuntimeAsset import/cook command without editor UI dependencies.
 * @param request Input command and command-specific request payload.
 * @param out_result Output command diagnostics.
 * @return Explicit command status.
 */
RuntimeAssetDataStatus ExecuteRuntimeAssetImportCookCommand(
    const RuntimeAssetImportCookCommandRequest &request,
    RuntimeAssetImportCookCommandResult *out_result);
/**
 * @brief Decodes RuntimeAsset shader program bytes into bridge-owned data.
 * @param bytes Source/cooked program bytes.
 * @param program_id Stable shader program id.
 * @param out_data Output decoded program payload.
 * @return Explicit decode status.
 */
RuntimeAssetDataStatus DecodeRuntimeAssetShaderProgramData(
    std::span<const std::uint8_t> bytes,
    std::uint32_t program_id,
    RuntimeAssetLoadedShaderProgramData *out_data);
/**
 * @brief Compiles RuntimeAsset shader source through a runtime-owned backend boundary.
 * @param request Input source bytes, backend kind, and expected policy identity.
 * @param out_result Output compiler, reflection, and loaded program data.
 * @return Explicit compiler status.
 */
RuntimeAssetDataStatus CompileRuntimeAssetShaderProgram(
    const RuntimeAssetShaderCompilerBackendRequest &request,
    RuntimeAssetShaderCompilerBackendResult *out_result);
/**
 * @brief Commits explicit RuntimeAsset asset dependency rows through AssetManager.
 * @param asset_manager Target Asset manager.
 * @param records Input dependency rows.
 * @param record_count Input row count.
 * @param out_result Output batch diagnostics.
 * @return Explicit RuntimeAssetData status.
 */
RuntimeAssetDataStatus CommitRuntimeAssetDataAssetDependencyBatch(
    yuengine::asset::AssetManager *asset_manager,
    const RuntimeAssetDataAssetDependencyRecord *records,
    std::uint32_t record_count,
    RuntimeAssetDataAssetDependencyBatchResult *out_result);
/**
 * @brief Commits WorldSceneAuthoring runtime export dependency rows through RuntimeAssetData.
 * @param request Input runtime export and caller-owned asset handles.
 * @param out_result Output batch diagnostics.
 * @return Explicit RuntimeAssetData status.
 */
RuntimeAssetDataStatus CommitRuntimeAssetDataWorldSceneAuthoringAssetDependencyBatch(
    const RuntimeAssetDataWorldSceneAuthoringAssetDependencyBatchRequest &request,
    RuntimeAssetDataAssetDependencyBatchResult *out_result);
/**
 * @brief Traverses RuntimeAsset asset dependencies without partial caller output mutation.
 * @param asset_manager Target Asset manager.
 * @param root_asset Root asset handle.
 * @param output_assets Caller-owned output asset handle rows.
 * @param output_asset_capacity Caller-owned output asset row capacity.
 * @param output_asset_count Caller-owned output asset row count.
 * @param out_result Output traversal diagnostics.
 * @return Explicit RuntimeAssetData status.
 */
RuntimeAssetDataStatus TraverseRuntimeAssetDataAssetDependencies(
    yuengine::asset::AssetManager *asset_manager,
    yuengine::asset::AssetHandle root_asset,
    yuengine::asset::AssetHandle *output_assets,
    std::uint32_t output_asset_capacity,
    std::uint32_t *output_asset_count,
    RuntimeAssetDataAssetDependencyTraverseResult *out_result);
/**
 * @brief Enumerates direct RuntimeAsset asset dependency rows by dependency ResourceType.
 * @param request Input manager, dependent asset, and ResourceType filter.
 * @param output_records Caller-owned output records.
 * @param output_record_capacity Caller-owned output record capacity.
 * @param output_record_count Caller-owned output record count.
 * @param out_result Output enumeration diagnostics.
 * @return Explicit RuntimeAssetData status.
 */
RuntimeAssetDataStatus EnumerateRuntimeAssetDataAssetDependenciesByType(
    const RuntimeAssetDataAssetDependencyTypeEnumerationRequest &request,
    RuntimeAssetDataAssetDependencyRecord *output_records,
    std::uint32_t output_record_capacity,
    std::uint32_t *output_record_count,
    RuntimeAssetDataAssetDependencyTypeEnumerationResult *out_result);
/**
 * @brief Counts direct RuntimeAsset asset dependency rows by dependency ResourceType.
 * @param request Input manager, dependent asset, and ResourceType filter.
 * @param output_dependency_count Caller-owned output count.
 * @param out_result Output count snapshot diagnostics.
 * @return Explicit RuntimeAssetData status.
 */
RuntimeAssetDataStatus SnapshotRuntimeAssetDataAssetDependencyTypeCount(
    const RuntimeAssetDataAssetDependencyTypeEnumerationRequest &request,
    std::uint32_t *output_dependency_count,
    RuntimeAssetDataAssetDependencyTypeCountSnapshotResult *out_result);
/**
 * @brief Looks up one exact committed RuntimeAsset asset dependency row.
 * @param request Input manager and explicit dependency row key.
 * @param out_record Caller-owned output record.
 * @param out_result Output exact lookup diagnostics.
 * @return Explicit RuntimeAssetData status.
 */
RuntimeAssetDataStatus LookupRuntimeAssetDataAssetDependencyExact(
    const RuntimeAssetDataAssetDependencyExactLookupRequest &request,
    RuntimeAssetDataAssetDependencyRecord *out_record,
    RuntimeAssetDataAssetDependencyExactLookupResult *out_result);
/**
 * @brief Loads a runtime asset graph from File/VFS into Resource and Asset records.
 * @param request Input graph load request.
 * @param out_result Output load result.
 * @return Explicit graph load status.
 */
RuntimeAssetDataStatus LoadRuntimeAssetDataGraph(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetGraphLoadResult *out_result);
/**
 * @brief Creates RHI shader modules and a pipeline from RuntimeAsset-owned bytecode.
 * @param request Input bytecode, layout, and RHI device.
 * @param out_result Output module and pipeline ownership records.
 * @return Explicit bridge status.
 */
RuntimeAssetDataStatus BuildRuntimeAssetShaderProgramPipeline(
    const RuntimeAssetShaderProgramPipelineRequest &request,
    RuntimeAssetShaderProgramPipelineResult *out_result);
/**
 * @brief Bridges validated cooked texture payloads to RHI handles and RenderScene material texture slots.
 * @param request Input descriptors, registries, RHI device, and output material storage.
 * @param out_result Output bridge result and cleanup ledger.
 * @return Explicit bridge status.
 */
RuntimeAssetDataStatus BuildRuntimeAssetCookedTextureMaterialBridge(
    const RuntimeAssetCookedTextureMaterialBridgeRequest &request,
    RuntimeAssetCookedTextureMaterialBridgeResult *out_result);
/**
 * @brief Creates RHI shader modules and a pipeline from cooked RuntimeAsset bytecode payload rows.
 * @param request Input cooked stage payloads, reflection, and RHI device.
 * @param out_result Output module, pipeline, and cleanup ledger records.
 * @return Explicit bridge status.
 */
RuntimeAssetDataStatus BuildRuntimeAssetCookedShaderProgramPipeline(
    const RuntimeAssetCookedShaderProgramPipelineRequest &request,
    RuntimeAssetCookedShaderProgramPipelineResult *out_result);
/**
 * @brief Proves cooked RuntimeAsset loaded records through RenderScene, RenderCore, and RHI capture.
 * @param request Loaded cooked records, registries, RHI, shader data, and output buffers.
 * @param out_result Output proof ledger and first missing layer.
 * @return Explicit RuntimeAsset visual proof status.
 */
RuntimeAssetDataStatus BuildRuntimeAssetCookedVisualProofRoute(
    const RuntimeAssetVisualProofRequest &request,
    RuntimeAssetVisualProofResult *out_result);
/**
 * @brief Builds RenderScene frame records from loaded RuntimeAsset scene records.
 * @param request Loaded scene records, geometry/material bindings, camera, and output storage.
 * @param out_result Output submission result.
 * @return Explicit RuntimeAsset submission status.
 */
RuntimeAssetDataStatus BuildRuntimeAssetRenderSceneSubmission(
    const RuntimeAssetRenderSceneSubmissionRequest &request,
    RuntimeAssetRenderSceneSubmissionResult *out_result);
/**
 * @brief Runs a Package-load-plan backed RuntimeAsset graph through RuntimeApp.
 * @param request Package load plan, graph buffers, RHI, and RuntimeApp descriptor.
 * @param out_result Output package consumption, graph, render, and RuntimeApp ledger.
 * @return Explicit RuntimeAsset packaged run status.
 */
RuntimeAssetDataStatus RunRuntimeAssetPackagedEntryPoint(
    const RuntimeAssetPackagedRunRequest &request,
    RuntimeAssetPackagedRunResult *out_result);
/**
 * @brief Reads a file-backed Package artifact and runs the packaged RuntimeAsset entrypoint.
 * @param request Artifact path, package lookup key, and packaged-run buffers.
 * @param out_result Output artifact, load-plan, packaged run, and missing-layer ledger.
 * @return Explicit product run status.
 */
RuntimeAssetDataStatus RunRuntimeAssetPackageArtifactProductCommand(
    const RuntimeAssetPackageArtifactProductRunRequest &request,
    RuntimeAssetPackageArtifactProductRunResult *out_result);

}
