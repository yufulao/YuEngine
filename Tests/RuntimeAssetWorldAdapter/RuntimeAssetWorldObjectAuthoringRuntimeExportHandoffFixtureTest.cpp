// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/Asset/AssetDescriptor.h"
#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetManager.h"
#include "YuEngine/Asset/AssetManagerDesc.h"
#include "YuEngine/Asset/AssetRecord.h"
#include "YuEngine/Asset/AssetRegistrationResult.h"
#include "YuEngine/Asset/AssetSnapshot.h"
#include "YuEngine/Asset/AssetStatus.h"
#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/Object/ObjectDescriptor.h"
#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Object/ObjectRegistrationResult.h"
#include "YuEngine/Object/ObjectRegistry.h"
#include "YuEngine/Object/ObjectRegistryDesc.h"
#include "YuEngine/Object/ObjectTypeId.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceRegistrationResult.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceSnapshot.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterIdentityRecord.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridge.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffResult.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffSnapshot.h"
#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingBridge.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldDesc.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectDesc.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldObjectIdentityBridge.h"
#include "YuEngine/World/WorldObjectIdentitySnapshot.h"
#include "YuEngine/World/WorldRegistrationResult.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateRecord.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofRecord.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofSliceRecord.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofStatus.h"
#include "YuEngine/World/WorldSceneAuthoringDocument.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanRecord.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldSceneRecordValueStreamBridge.h"
#include "YuEngine/World/WorldSceneRecordValueStreamResult.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformSnapshot.h"
#include "YuEngine/World/WorldTransformState.h"

using yuengine::asset::AssetDescriptor;
using yuengine::asset::AssetHandle;
using yuengine::asset::AssetManager;
using yuengine::asset::AssetManagerDesc;
using yuengine::asset::AssetRecord;
using yuengine::asset::AssetRegistrationResult;
using yuengine::asset::AssetSnapshot;
using yuengine::asset::AssetStatus;
using yuengine::asset::AssetTypeId;
using yuengine::object::ObjectDescriptor;
using yuengine::object::ObjectHandle;
using yuengine::object::ObjectRegistrationResult;
using yuengine::object::ObjectRegistry;
using yuengine::object::ObjectRegistryDesc;
using yuengine::object::ObjectTypeId;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLogicalKey;
using yuengine::resource::ResourceRegistrationResult;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceSnapshot;
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord;
using yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord;
using yuengine::runtimeasset::RuntimeAssetTargetIdentityKind;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterIdentityRecord;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffBridge;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffResult;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffSnapshot;
using yuengine::serialize::MAX_STREAM_BYTE_COUNT;
using yuengine::serialize::SerializeReader;
using yuengine::serialize::SerializeWriter;
using yuengine::world::WorldComponentAttachmentBridge;
using yuengine::world::WorldComponentAttachmentSnapshotRecord;
using yuengine::world::WorldComponentResourceBindingBridge;
using yuengine::world::WorldComponentResourceBindingSnapshotRecord;
using yuengine::world::WorldComponentSlotId;
using yuengine::world::WorldComponentTypeId;
using yuengine::world::WorldDesc;
using yuengine::world::WorldInstance;
using yuengine::world::WorldObjectDesc;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldObjectIdentityBridge;
using yuengine::world::WorldObjectIdentitySnapshot;
using yuengine::world::WorldRegistrationResult;
using yuengine::world::WorldSceneActiveRestoreGateRecord;
using yuengine::world::WorldSceneActiveRestoreGateStatus;
using yuengine::world::WorldSceneApplyTimeRestoreProofFamily;
using yuengine::world::WorldSceneApplyTimeRestoreProofRecord;
using yuengine::world::WorldSceneApplyTimeRestoreProofSliceRecord;
using yuengine::world::WorldSceneApplyTimeRestoreProofStatus;
using yuengine::world::WorldSceneAuthoringDependencyRecord;
using yuengine::world::WorldSceneAuthoringDocument;
using yuengine::world::WorldSceneAuthoringDocumentResult;
using yuengine::world::WorldSceneAuthoringDocumentSnapshot;
using yuengine::world::WorldSceneAuthoringDocumentValidator;
using yuengine::world::WorldSceneAuthoringRuntimeExport;
using yuengine::world::WorldSceneDecodedRestorePlanRecord;
using yuengine::world::WorldSceneDecodedRestorePlanRecordFamily;
using yuengine::world::WorldSceneDecodedRestorePlanStatus;
using yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;
using yuengine::world::WorldSceneRecordValueStreamBridge;
using yuengine::world::WorldSceneRecordValueStreamResult;
using yuengine::world::WorldTransformBridge;
using yuengine::world::WorldTransformResult;
using yuengine::world::WorldTransformSnapshot;
using yuengine::world::WorldTransformState;

namespace {
constexpr std::uint32_t DATA_RECORD_COUNT = 3U;
constexpr std::uint32_t STREAM_PLAN_RECORD_COUNT = DATA_RECORD_COUNT + DATA_RECORD_COUNT;
constexpr std::uint32_t SCRATCH_RECORD_COUNT = 8U;
constexpr std::uint32_t EMPTY_SIDECAR_COUNT = 1U;
constexpr std::uint32_t EMPTY_DEPENDENCY_COUNT = 1U;
constexpr std::uint32_t SIDECAR_RECORD_COUNT = 1U;
constexpr std::uint32_t DEPENDENCY_RECORD_COUNT = 1U;
constexpr std::uint32_t SIDECAR_STREAM_PLAN_RECORD_COUNT =
    STREAM_PLAN_RECORD_COUNT + SIDECAR_RECORD_COUNT + SIDECAR_RECORD_COUNT;
constexpr std::uint64_t SCENE_DOCUMENT_ID = 0x4101U;
constexpr std::uint64_t SCENE_DOCUMENT_HASH = 0xA55A4101U;
constexpr std::uint64_t STABLE_RESOURCE_ID = 0x5150U;
constexpr std::uint64_t ASSET_STABLE_SCENE_DOCUMENT = 0xA510U;
constexpr std::uint64_t ASSET_STABLE_TEXTURE = 0xA511U;
constexpr const char *TEST_FEED_AUTHORING_RUNTIME_EXPORT =
    "RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_FeedsAuthoringRuntimeExportThroughRecordStreamIntoRestoreHandoff";
constexpr const char *TEST_FEED_AUTHORING_SIDECAR_RUNTIME_EXPORT =
    "RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_FeedsAttachmentBindingDependencyExportThroughRecordStreamIntoRestoreHandoff";
constexpr const char *TEST_COMMIT_AUTHORING_DEPENDENCY_EDGE_RUNTIME_EXPORT =
    "RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_CommitsExportedDependencyAsCallerOwnedResourceEdge";
constexpr const char *TEST_COMMIT_AUTHORING_ASSET_DEPENDENCY_EDGE_RUNTIME_EXPORT =
    "RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_CommitsExportedDependencyAsCallerOwnedAssetEdge";
constexpr WorldObjectId WORLD_OBJECT_SCENE{31U};
constexpr WorldObjectId WORLD_OBJECT_MODEL{32U};
constexpr WorldObjectId WORLD_OBJECT_SKELETON{33U};
constexpr WorldObjectId SENTINEL_WORLD_OBJECT{99U};
constexpr ObjectHandle SENTINEL_OBJECT_HANDLE{9U, 9U};
constexpr ObjectTypeId OBJECT_TYPE_SCENE{1U};
constexpr ObjectTypeId OBJECT_TYPE_MODEL{2U};
constexpr ObjectTypeId OBJECT_TYPE_SKELETON{3U};
constexpr AssetTypeId ASSET_TYPE_SCENE_DOCUMENT{1U};
constexpr AssetTypeId ASSET_TYPE_TEXTURE{2U};
constexpr ResourceTypeId RESOURCE_TYPE_TEXTURE{1U};
constexpr ResourceTypeId RESOURCE_TYPE_SCENE_DOCUMENT{2U};
constexpr WorldComponentTypeId COMPONENT_TYPE_MESH{1U};
constexpr WorldComponentSlotId COMPONENT_SLOT_MESH{11U};
constexpr std::uint64_t TARGET_SCENE = 8101U;
constexpr std::uint64_t TARGET_MODEL = 8102U;
constexpr std::uint64_t TARGET_SKELETON = 8103U;
constexpr std::uint32_t ENTITY_SCENE = 401U;
constexpr std::uint32_t ENTITY_MODEL = 402U;
constexpr std::uint32_t ENTITY_SKELETON = 403U;

using SerializeBuffer = std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT>;

struct AuthoringRuntimeExportHandoffFixture final {
    std::array<RuntimeAssetRuntimeInstanceMappingRecord, DATA_RECORD_COUNT> mappings{};
    std::array<RuntimeAssetSceneEntityRecord, DATA_RECORD_COUNT> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, DATA_RECORD_COUNT> scene_transforms{};
    std::array<RuntimeAssetWorldObjectAdapterIdentityRecord, DATA_RECORD_COUNT> identity_records{};
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, DATA_RECORD_COUNT> authoring_input_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, DATA_RECORD_COUNT> authoring_input_transforms{};
    std::array<WorldComponentAttachmentSnapshotRecord, EMPTY_SIDECAR_COUNT> authoring_input_attachments{};
    std::array<WorldComponentResourceBindingSnapshotRecord, EMPTY_SIDECAR_COUNT> authoring_input_bindings{};
    std::array<WorldSceneAuthoringDependencyRecord, EMPTY_DEPENDENCY_COUNT> authoring_input_dependencies{};
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, DATA_RECORD_COUNT> authoring_output_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, DATA_RECORD_COUNT> authoring_output_transforms{};
    std::array<WorldComponentAttachmentSnapshotRecord, EMPTY_SIDECAR_COUNT> authoring_output_attachments{};
    std::array<WorldComponentResourceBindingSnapshotRecord, EMPTY_SIDECAR_COUNT> authoring_output_bindings{};
    std::array<WorldSceneAuthoringDependencyRecord, EMPTY_DEPENDENCY_COUNT> authoring_output_dependencies{};
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, DATA_RECORD_COUNT> stream_output_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, DATA_RECORD_COUNT> stream_output_transforms{};
    std::array<WorldComponentAttachmentSnapshotRecord, EMPTY_SIDECAR_COUNT> stream_output_attachments{};
    std::array<WorldComponentResourceBindingSnapshotRecord, EMPTY_SIDECAR_COUNT> stream_output_bindings{};
    std::array<WorldComponentAttachmentSnapshotRecord, EMPTY_SIDECAR_COUNT> empty_attachments{};
    std::array<WorldComponentResourceBindingSnapshotRecord, EMPTY_SIDECAR_COUNT> empty_bindings{};
    std::array<WorldSceneDecodedRestorePlanRecord, SCRATCH_RECORD_COUNT> plan_scratch{};
    std::array<WorldSceneApplyTimeRestoreProofRecord, SCRATCH_RECORD_COUNT> proof_scratch{};
    std::array<WorldSceneApplyTimeRestoreProofSliceRecord, SCRATCH_RECORD_COUNT> slice_scratch{};
    std::array<WorldSceneActiveRestoreGateRecord, SCRATCH_RECORD_COUNT> gate_outputs{};
    SerializeBuffer stream_buffer{};
    std::uint32_t authoring_identity_count = 0U;
    std::uint32_t authoring_transform_count = 0U;
    std::uint32_t authoring_attachment_count = 0U;
    std::uint32_t authoring_binding_count = 0U;
    std::uint32_t authoring_dependency_count = 0U;
};

int Fail(std::string_view message) {
    std::fprintf(stderr, "%.*s\n", static_cast<int>(message.size()), message.data());
    return 1;
}

WorldTransformState Transform(float translation_x) {
    WorldTransformState transform{};
    transform.translation_x = translation_x;
    transform.translation_y = translation_x + 1.0F;
    transform.translation_z = translation_x + 2.0F;
    transform.rotation_w = 1.0F;
    transform.scale_x = 1.0F;
    transform.scale_y = 1.0F;
    transform.scale_z = 1.0F;
    return transform;
}

RuntimeAssetRuntimeInstanceMappingRecord Mapping(
    RuntimeAssetTargetIdentityKind target_kind,
    std::uint64_t target_id,
    std::uint32_t entity_id,
    std::uint32_t scene_entity_index,
    std::uint32_t scene_transform_index) {
    RuntimeAssetRuntimeInstanceMappingRecord record{};
    record.target_kind = target_kind;
    record.target_id = target_id;
    record.scene_entity_id = entity_id;
    record.scene_entity_index = scene_entity_index;
    record.scene_transform_index = scene_transform_index;
    record.is_valid = true;
    return record;
}

RuntimeAssetSceneEntityRecord Entity(
    std::uint32_t entity_id,
    WorldObjectId world_object_id,
    WorldTransformState transform) {
    RuntimeAssetSceneEntityRecord record{};
    record.entity_id = entity_id;
    record.world_object_id = world_object_id;
    record.transform = transform;
    return record;
}

RuntimeAssetSceneTransformOutputRecord TransformOutput(
    WorldObjectId world_object_id,
    WorldTransformState transform) {
    RuntimeAssetSceneTransformOutputRecord record{};
    record.world_object_id = world_object_id;
    record.transform = transform;
    return record;
}

RuntimeAssetWorldObjectAdapterIdentityRecord Identity(
    std::uint64_t target_id,
    WorldObjectId world_object_id,
    ObjectHandle object_handle) {
    RuntimeAssetWorldObjectAdapterIdentityRecord record{};
    record.target_id = target_id;
    record.world_object_id = world_object_id;
    record.object_handle = object_handle;
    return record;
}

WorldSceneObjectTransformRestoreIdentityRecord RestoreIdentity(
    WorldObjectId world_object_id,
    ObjectHandle object_handle) {
    WorldSceneObjectTransformRestoreIdentityRecord record{};
    record.world_object_id = world_object_id;
    record.object_handle = object_handle;
    return record;
}

WorldSceneObjectTransformRestoreTransformRecord RestoreTransform(
    WorldObjectId world_object_id,
    WorldTransformState transform_state) {
    WorldSceneObjectTransformRestoreTransformRecord record{};
    record.world_object_id = world_object_id;
    record.transform_state = transform_state;
    return record;
}

WorldSceneObjectTransformRestoreIdentityRecord SentinelIdentityOutput() {
    return RestoreIdentity(SENTINEL_WORLD_OBJECT, SENTINEL_OBJECT_HANDLE);
}

WorldSceneObjectTransformRestoreTransformRecord SentinelTransformOutput() {
    return RestoreTransform(SENTINEL_WORLD_OBJECT, Transform(90.0F));
}

AuthoringRuntimeExportHandoffFixture MakeFixture() {
    AuthoringRuntimeExportHandoffFixture fixture{};
    fixture.mappings[0U] = Mapping(
        RuntimeAssetTargetIdentityKind::SceneNode,
        TARGET_SCENE,
        ENTITY_SCENE,
        0U,
        0U);
    fixture.mappings[1U] = Mapping(
        RuntimeAssetTargetIdentityKind::ModelNode,
        TARGET_MODEL,
        ENTITY_MODEL,
        1U,
        1U);
    fixture.mappings[2U] = Mapping(
        RuntimeAssetTargetIdentityKind::SkeletonJoint,
        TARGET_SKELETON,
        ENTITY_SKELETON,
        2U,
        2U);
    fixture.scene_entities[0U] = Entity(ENTITY_SCENE, WORLD_OBJECT_SCENE, Transform(10.0F));
    fixture.scene_entities[1U] = Entity(ENTITY_MODEL, WORLD_OBJECT_MODEL, Transform(20.0F));
    fixture.scene_entities[2U] = Entity(ENTITY_SKELETON, WORLD_OBJECT_SKELETON, Transform(30.0F));
    fixture.scene_transforms[0U] = TransformOutput(WORLD_OBJECT_SCENE, Transform(40.0F));
    fixture.scene_transforms[1U] = TransformOutput(WORLD_OBJECT_MODEL, Transform(50.0F));
    fixture.scene_transforms[2U] = TransformOutput(WORLD_OBJECT_SKELETON, Transform(60.0F));

    std::uint32_t index = 0U;
    while (index < DATA_RECORD_COUNT) {
        fixture.authoring_output_identities[index] = SentinelIdentityOutput();
        fixture.authoring_output_transforms[index] = SentinelTransformOutput();
        fixture.stream_output_identities[index] = SentinelIdentityOutput();
        fixture.stream_output_transforms[index] = SentinelTransformOutput();
        ++index;
    }

    return fixture;
}

ObjectRegistry MakeObjectRegistry() {
    ObjectRegistryDesc desc{};
    desc.object_capacity = 8U;
    desc.type_capacity = 8U;
    return ObjectRegistry(desc);
}

WorldInstance MakeWorld() {
    WorldDesc desc{};
    desc.object_capacity = 8U;
    return WorldInstance(desc);
}

AssetManager MakeAssetManager() {
    AssetManagerDesc desc{};
    desc.asset_capacity = 4U;
    desc.type_capacity = 4U;
    desc.dependency_edge_capacity = 2U;
    return AssetManager(desc);
}

ObjectRegistrationResult CreateObject(ObjectRegistry *object_registry, ObjectTypeId type) {
    ObjectDescriptor descriptor{};
    descriptor.type = type;
    descriptor.initial_reference_count = 0U;
    return object_registry->CreateSyntheticObject(descriptor);
}

AssetDescriptor Asset(
    std::uint64_t stable_id,
    AssetTypeId asset_type,
    ResourceHandle resource,
    ResourceTypeId resource_type) {
    AssetDescriptor descriptor{};
    descriptor.stable_id = stable_id;
    descriptor.asset_type = asset_type;
    descriptor.resource = resource;
    descriptor.resource_type = resource_type;
    return descriptor;
}

ResourceDescriptor Resource(ResourceTypeId type, const char *key) {
    ResourceDescriptor descriptor{};
    descriptor.type = type;
    descriptor.logical_key = ResourceLogicalKey(key);
    return descriptor;
}

int RegisterWorldObject(WorldInstance *world, WorldObjectId world_object_id) {
    WorldObjectDesc object_desc{};
    object_desc.id = world_object_id;
    object_desc.is_enabled = true;
    const WorldRegistrationResult result = world->RegisterObject(object_desc);
    if (!result.Succeeded()) {
        return Fail("authoring handoff world object setup failed");
    }

    return 0;
}

int PrepareCallerOwnedRegistries(
    WorldInstance *world,
    ObjectRegistry *object_registry,
    AuthoringRuntimeExportHandoffFixture *fixture) {
    if (world == nullptr) {
        return Fail("authoring handoff world fixture missing");
    }

    if (object_registry == nullptr) {
        return Fail("authoring handoff object registry fixture missing");
    }

    if (fixture == nullptr) {
        return Fail("authoring handoff fixture missing");
    }

    if (RegisterWorldObject(world, WORLD_OBJECT_SCENE) != 0) {
        return 1;
    }

    if (RegisterWorldObject(world, WORLD_OBJECT_MODEL) != 0) {
        return 1;
    }

    if (RegisterWorldObject(world, WORLD_OBJECT_SKELETON) != 0) {
        return 1;
    }

    const ObjectRegistrationResult scene_object = CreateObject(object_registry, OBJECT_TYPE_SCENE);
    if (!scene_object.Succeeded()) {
        return Fail("authoring handoff scene object setup failed");
    }

    const ObjectRegistrationResult model_object = CreateObject(object_registry, OBJECT_TYPE_MODEL);
    if (!model_object.Succeeded()) {
        return Fail("authoring handoff model object setup failed");
    }

    const ObjectRegistrationResult skeleton_object = CreateObject(object_registry, OBJECT_TYPE_SKELETON);
    if (!skeleton_object.Succeeded()) {
        return Fail("authoring handoff skeleton object setup failed");
    }

    fixture->identity_records[0U] = Identity(
        TARGET_SCENE,
        WORLD_OBJECT_SCENE,
        scene_object.handle);
    fixture->identity_records[1U] = Identity(
        TARGET_MODEL,
        WORLD_OBJECT_MODEL,
        model_object.handle);
    fixture->identity_records[2U] = Identity(
        TARGET_SKELETON,
        WORLD_OBJECT_SKELETON,
        skeleton_object.handle);

    std::uint32_t index = 0U;
    while (index < DATA_RECORD_COUNT) {
        const RuntimeAssetWorldObjectAdapterIdentityRecord &identity =
            fixture->identity_records[index];
        const RuntimeAssetSceneTransformOutputRecord &transform =
            fixture->scene_transforms[index];
        fixture->authoring_input_identities[index] = RestoreIdentity(
            identity.world_object_id,
            identity.object_handle);
        fixture->authoring_input_transforms[index] = RestoreTransform(
            transform.world_object_id,
            transform.transform);
        ++index;
    }

    return 0;
}

int PrepareAuthoringSidecarInputs(
    AuthoringRuntimeExportHandoffFixture *fixture,
    ResourceRegistry *resource_registry,
    ResourceHandle *texture_handle) {
    if (fixture == nullptr) {
        return Fail("authoring sidecar fixture missing");
    }

    if (resource_registry == nullptr) {
        return Fail("authoring sidecar resource registry missing");
    }

    ResourceDescriptor texture_descriptor = Resource(RESOURCE_TYPE_TEXTURE, "authoring_handoff_texture");
    const ResourceRegistrationResult texture_resource =
        resource_registry->RegisterSyntheticDescriptor(texture_descriptor);
    if (!texture_resource.Succeeded()) {
        return Fail("authoring sidecar texture setup failed");
    }

    fixture->authoring_input_attachments[0U].world_object_id = WORLD_OBJECT_MODEL;
    fixture->authoring_input_attachments[0U].component_type_id = COMPONENT_TYPE_MESH;
    fixture->authoring_input_attachments[0U].component_slot_id = COMPONENT_SLOT_MESH;

    fixture->authoring_input_bindings[0U].world_object_id = WORLD_OBJECT_MODEL;
    fixture->authoring_input_bindings[0U].component_type_id = COMPONENT_TYPE_MESH;
    fixture->authoring_input_bindings[0U].component_slot_id = COMPONENT_SLOT_MESH;
    fixture->authoring_input_bindings[0U].resource_handle = texture_resource.handle;
    fixture->authoring_input_bindings[0U].expected_resource_type = RESOURCE_TYPE_TEXTURE;

    fixture->authoring_input_dependencies[0U].stable_resource_id = STABLE_RESOURCE_ID;
    fixture->authoring_input_dependencies[0U].resource_handle = texture_resource.handle;
    fixture->authoring_input_dependencies[0U].expected_resource_type = RESOURCE_TYPE_TEXTURE;

    if (texture_handle != nullptr) {
        *texture_handle = texture_resource.handle;
    }

    return 0;
}

int PrepareAuthoringDependencyEdgeInputs(
    AuthoringRuntimeExportHandoffFixture *fixture,
    ResourceRegistry *resource_registry,
    ResourceHandle *document_handle,
    ResourceHandle *texture_handle) {
    if (fixture == nullptr) {
        return Fail("authoring dependency edge fixture missing");
    }

    if (resource_registry == nullptr) {
        return Fail("authoring dependency edge resource registry missing");
    }

    ResourceDescriptor document_descriptor = Resource(
        RESOURCE_TYPE_SCENE_DOCUMENT,
        "authoring_scene_document");
    const ResourceRegistrationResult document_resource =
        resource_registry->RegisterSyntheticDescriptor(document_descriptor);
    if (!document_resource.Succeeded()) {
        return Fail("authoring dependency edge document setup failed");
    }

    if (document_handle != nullptr) {
        *document_handle = document_resource.handle;
    }

    return PrepareAuthoringSidecarInputs(fixture, resource_registry, texture_handle);
}

int PrepareAuthoringAssetDependencyEdgeInputs(
    AuthoringRuntimeExportHandoffFixture *fixture,
    ResourceRegistry *resource_registry,
    AssetManager *asset_manager,
    AssetHandle *document_asset_handle,
    AssetHandle *dependency_asset_handle,
    ResourceHandle *texture_handle) {
    if (asset_manager == nullptr) {
        return Fail("authoring asset dependency edge manager missing");
    }

    ResourceHandle document_resource{};
    ResourceHandle dependency_resource{};
    if (PrepareAuthoringDependencyEdgeInputs(
            fixture,
            resource_registry,
            &document_resource,
            &dependency_resource) != 0) {
        return 1;
    }

    const AssetDescriptor document_asset = Asset(
        ASSET_STABLE_SCENE_DOCUMENT,
        ASSET_TYPE_SCENE_DOCUMENT,
        document_resource,
        RESOURCE_TYPE_SCENE_DOCUMENT);
    const AssetRegistrationResult document_result =
        asset_manager->RegisterRuntimeAsset(resource_registry, document_asset);
    if (!document_result.Succeeded()) {
        return Fail("authoring asset dependency edge document asset setup failed");
    }

    const AssetDescriptor dependency_asset = Asset(
        ASSET_STABLE_TEXTURE,
        ASSET_TYPE_TEXTURE,
        dependency_resource,
        RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult dependency_result =
        asset_manager->RegisterRuntimeAsset(resource_registry, dependency_asset);
    if (!dependency_result.Succeeded()) {
        return Fail("authoring asset dependency edge dependency asset setup failed");
    }

    if (document_asset_handle != nullptr) {
        *document_asset_handle = document_result.handle;
    }

    if (dependency_asset_handle != nullptr) {
        *dependency_asset_handle = dependency_result.handle;
    }

    if (texture_handle != nullptr) {
        *texture_handle = dependency_resource;
    }

    return 0;
}

WorldSceneAuthoringDocument MakeAuthoringDocument(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    std::uint32_t attachment_count,
    std::uint32_t binding_count,
    std::uint32_t dependency_count) {
    WorldSceneAuthoringDocument document{};
    document.header.scene_document_id = SCENE_DOCUMENT_ID;
    document.header.deterministic_document_hash = SCENE_DOCUMENT_HASH;
    document.header.identity_record_count = DATA_RECORD_COUNT;
    document.header.transform_record_count = DATA_RECORD_COUNT;
    document.header.attachment_record_count = attachment_count;
    document.header.binding_record_count = binding_count;
    document.header.dependency_record_count = dependency_count;
    document.identity_records = fixture.authoring_input_identities.data();
    document.transform_records = fixture.authoring_input_transforms.data();
    document.attachment_records = fixture.authoring_input_attachments.data();
    document.binding_records = fixture.authoring_input_bindings.data();
    document.dependency_records = fixture.authoring_input_dependencies.data();
    return document;
}

WorldSceneAuthoringRuntimeExport MakeRuntimeExport(
    AuthoringRuntimeExportHandoffFixture *fixture) {
    WorldSceneAuthoringRuntimeExport runtime_output{};
    if (fixture == nullptr) {
        return runtime_output;
    }

    runtime_output.identity_records = fixture->authoring_output_identities.data();
    runtime_output.identity_capacity = DATA_RECORD_COUNT;
    runtime_output.identity_count = &fixture->authoring_identity_count;
    runtime_output.transform_records = fixture->authoring_output_transforms.data();
    runtime_output.transform_capacity = DATA_RECORD_COUNT;
    runtime_output.transform_count = &fixture->authoring_transform_count;
    runtime_output.attachment_records = fixture->authoring_output_attachments.data();
    runtime_output.attachment_capacity = EMPTY_SIDECAR_COUNT;
    runtime_output.attachment_count = &fixture->authoring_attachment_count;
    runtime_output.binding_records = fixture->authoring_output_bindings.data();
    runtime_output.binding_capacity = EMPTY_SIDECAR_COUNT;
    runtime_output.binding_count = &fixture->authoring_binding_count;
    runtime_output.dependency_records = fixture->authoring_output_dependencies.data();
    runtime_output.dependency_capacity = EMPTY_DEPENDENCY_COUNT;
    runtime_output.dependency_count = &fixture->authoring_dependency_count;
    return runtime_output;
}

bool ObjectHandlesMatch(ObjectHandle left, ObjectHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool ResourceHandlesMatch(ResourceHandle left, ResourceHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool AssetHandlesMatch(AssetHandle left, AssetHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool TransformsMatch(WorldTransformState left, WorldTransformState right) {
    if (left.translation_x != right.translation_x) {
        return false;
    }

    if (left.translation_y != right.translation_y) {
        return false;
    }

    if (left.translation_z != right.translation_z) {
        return false;
    }

    if (left.rotation_x != right.rotation_x) {
        return false;
    }

    if (left.rotation_y != right.rotation_y) {
        return false;
    }

    if (left.rotation_z != right.rotation_z) {
        return false;
    }

    if (left.rotation_w != right.rotation_w) {
        return false;
    }

    if (left.scale_x != right.scale_x) {
        return false;
    }

    if (left.scale_y != right.scale_y) {
        return false;
    }

    return left.scale_z == right.scale_z;
}

int ExportAuthoringRuntimeRecords(
    AuthoringRuntimeExportHandoffFixture *fixture,
    std::uint32_t attachment_count,
    std::uint32_t binding_count,
    std::uint32_t dependency_count) {
    if (fixture == nullptr) {
        return Fail("authoring export fixture missing");
    }

    const WorldSceneAuthoringDocument document = MakeAuthoringDocument(
        *fixture,
        attachment_count,
        binding_count,
        dependency_count);
    WorldSceneAuthoringRuntimeExport runtime_output = MakeRuntimeExport(fixture);
    WorldSceneAuthoringDocumentValidator validator;
    const WorldSceneAuthoringDocumentResult result =
        validator.ValidateAndExport(document, &runtime_output);
    if (!result.Succeeded()) {
        return Fail("authoring runtime export failed");
    }

    if (result.state.scene_document_id != SCENE_DOCUMENT_ID) {
        return Fail("authoring runtime export scene document id mismatch");
    }

    if (result.state.deterministic_document_hash != SCENE_DOCUMENT_HASH) {
        return Fail("authoring runtime export hash mismatch");
    }

    if (result.state.exported_identity_count != DATA_RECORD_COUNT) {
        return Fail("authoring runtime export identity state count mismatch");
    }

    if (result.state.exported_transform_count != DATA_RECORD_COUNT) {
        return Fail("authoring runtime export transform state count mismatch");
    }

    if (result.state.exported_attachment_count != attachment_count) {
        return Fail("authoring runtime export attachment state count mismatch");
    }

    if (result.state.exported_binding_count != binding_count) {
        return Fail("authoring runtime export binding state count mismatch");
    }

    if (result.state.exported_dependency_count != dependency_count) {
        return Fail("authoring runtime export dependency state count mismatch");
    }

    const WorldSceneAuthoringDocumentSnapshot snapshot = validator.Snapshot();
    if (snapshot.runtime_export_count != 1U) {
        return Fail("authoring runtime export snapshot count mismatch");
    }

    return 0;
}

int VerifyAuthoringSidecarRuntimeOutputs(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    ResourceHandle texture_handle) {
    const WorldComponentAttachmentSnapshotRecord &attachment =
        fixture.authoring_output_attachments[0U];
    if (attachment.world_object_id.value != WORLD_OBJECT_MODEL.value) {
        return Fail("authoring runtime attachment world object mismatch");
    }

    if (attachment.component_type_id.value != COMPONENT_TYPE_MESH.value) {
        return Fail("authoring runtime attachment component type mismatch");
    }

    if (attachment.component_slot_id.value != COMPONENT_SLOT_MESH.value) {
        return Fail("authoring runtime attachment component slot mismatch");
    }

    const WorldComponentResourceBindingSnapshotRecord &binding =
        fixture.authoring_output_bindings[0U];
    if (binding.world_object_id.value != WORLD_OBJECT_MODEL.value) {
        return Fail("authoring runtime binding world object mismatch");
    }

    if (binding.component_type_id.value != COMPONENT_TYPE_MESH.value) {
        return Fail("authoring runtime binding component type mismatch");
    }

    if (binding.component_slot_id.value != COMPONENT_SLOT_MESH.value) {
        return Fail("authoring runtime binding component slot mismatch");
    }

    if (!ResourceHandlesMatch(binding.resource_handle, texture_handle)) {
        return Fail("authoring runtime binding resource handle mismatch");
    }

    if (binding.expected_resource_type.value != RESOURCE_TYPE_TEXTURE.value) {
        return Fail("authoring runtime binding resource type mismatch");
    }

    const WorldSceneAuthoringDependencyRecord &dependency =
        fixture.authoring_output_dependencies[0U];
    if (dependency.stable_resource_id != STABLE_RESOURCE_ID) {
        return Fail("authoring runtime dependency id mismatch");
    }

    if (!ResourceHandlesMatch(dependency.resource_handle, texture_handle)) {
        return Fail("authoring runtime dependency resource handle mismatch");
    }

    if (dependency.expected_resource_type.value != RESOURCE_TYPE_TEXTURE.value) {
        return Fail("authoring runtime dependency resource type mismatch");
    }

    return 0;
}

int VerifyAuthoringRuntimeOutputs(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    std::uint32_t attachment_count,
    std::uint32_t binding_count,
    std::uint32_t dependency_count,
    ResourceHandle texture_handle) {
    if (fixture.authoring_identity_count != DATA_RECORD_COUNT) {
        return Fail("authoring runtime identity count mismatch");
    }

    if (fixture.authoring_transform_count != DATA_RECORD_COUNT) {
        return Fail("authoring runtime transform count mismatch");
    }

    if (fixture.authoring_attachment_count != attachment_count) {
        return Fail("authoring runtime attachment count mismatch");
    }

    if (fixture.authoring_binding_count != binding_count) {
        return Fail("authoring runtime binding count mismatch");
    }

    if (fixture.authoring_dependency_count != dependency_count) {
        return Fail("authoring runtime dependency count mismatch");
    }

    std::uint32_t index = 0U;
    while (index < DATA_RECORD_COUNT) {
        const WorldSceneObjectTransformRestoreIdentityRecord &runtime_identity =
            fixture.authoring_output_identities[index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord &expected_identity =
            fixture.identity_records[index];
        if (runtime_identity.world_object_id.value != expected_identity.world_object_id.value) {
            return Fail("authoring runtime identity world object mismatch");
        }

        if (!ObjectHandlesMatch(runtime_identity.object_handle, expected_identity.object_handle)) {
            return Fail("authoring runtime identity object handle mismatch");
        }

        const WorldSceneObjectTransformRestoreTransformRecord &runtime_transform =
            fixture.authoring_output_transforms[index];
        const RuntimeAssetSceneTransformOutputRecord &expected_transform =
            fixture.scene_transforms[index];
        if (runtime_transform.world_object_id.value != expected_transform.world_object_id.value) {
            return Fail("authoring runtime transform world object mismatch");
        }

        if (!TransformsMatch(runtime_transform.transform_state, expected_transform.transform)) {
            return Fail("authoring runtime transform state mismatch");
        }

        ++index;
    }

    if (attachment_count == 0U) {
        return 0;
    }

    if (binding_count == 0U) {
        return Fail("authoring runtime missing binding sidecar");
    }

    if (dependency_count == 0U) {
        return Fail("authoring runtime missing dependency sidecar");
    }

    if (VerifyAuthoringSidecarRuntimeOutputs(fixture, texture_handle) != 0) {
        return 1;
    }

    return 0;
}

int RoundTripWorldSceneRecordStream(
    AuthoringRuntimeExportHandoffFixture *fixture,
    std::uint32_t attachment_count,
    std::uint32_t binding_count) {
    if (fixture == nullptr) {
        return Fail("authoring record stream fixture missing");
    }

    WorldSceneRecordValueStreamBridge stream_bridge{};
    const std::uint32_t stream_capacity =
        static_cast<std::uint32_t>(fixture->stream_buffer.size());
    SerializeWriter writer(fixture->stream_buffer.data(), stream_capacity);
    const WorldSceneRecordValueStreamResult write_result = stream_bridge.WriteSceneRecords(
        &writer,
        fixture->authoring_output_identities.data(),
        fixture->authoring_identity_count,
        fixture->authoring_output_transforms.data(),
        fixture->authoring_transform_count,
        fixture->authoring_output_attachments.data(),
        attachment_count,
        fixture->authoring_output_bindings.data(),
        binding_count);
    if (!write_result.Succeeded()) {
        return Fail("authoring record stream write failed");
    }

    if (write_result.state.committed_byte_count == 0U) {
        return Fail("authoring record stream write byte count missing");
    }

    std::uint32_t identity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t read_attachment_count = 0U;
    std::uint32_t read_binding_count = 0U;
    SerializeReader reader(fixture->stream_buffer.data(), write_result.state.committed_byte_count);
    const WorldSceneRecordValueStreamResult read_result = stream_bridge.ReadSceneRecords(
        &reader,
        fixture->stream_output_identities.data(),
        DATA_RECORD_COUNT,
        &identity_count,
        fixture->stream_output_transforms.data(),
        DATA_RECORD_COUNT,
        &transform_count,
        fixture->stream_output_attachments.data(),
        EMPTY_SIDECAR_COUNT,
        &read_attachment_count,
        fixture->stream_output_bindings.data(),
        EMPTY_SIDECAR_COUNT,
        &read_binding_count);
    if (!read_result.Succeeded()) {
        return Fail("authoring record stream read failed");
    }

    if (identity_count != DATA_RECORD_COUNT) {
        return Fail("authoring record stream identity count mismatch");
    }

    if (transform_count != DATA_RECORD_COUNT) {
        return Fail("authoring record stream transform count mismatch");
    }

    if (read_attachment_count != attachment_count) {
        return Fail("authoring record stream attachment count mismatch");
    }

    if (read_binding_count != binding_count) {
        return Fail("authoring record stream binding count mismatch");
    }

    return 0;
}

RuntimeAssetWorldObjectAdapterRequest MakeAdapterRequest(
    AuthoringRuntimeExportHandoffFixture *fixture) {
    RuntimeAssetWorldObjectAdapterRequest request{};
    if (fixture == nullptr) {
        return request;
    }

    request.runtime_instance_mappings = fixture->mappings.data();
    request.runtime_instance_mapping_count = DATA_RECORD_COUNT;
    request.scene_entities = fixture->scene_entities.data();
    request.scene_entity_count = DATA_RECORD_COUNT;
    request.scene_transforms = fixture->scene_transforms.data();
    request.scene_transform_count = DATA_RECORD_COUNT;
    request.identity_records = fixture->identity_records.data();
    request.identity_record_count = DATA_RECORD_COUNT;
    request.output_identities = fixture->stream_output_identities.data();
    request.output_identity_capacity = DATA_RECORD_COUNT;
    request.output_transforms = fixture->stream_output_transforms.data();
    request.output_transform_capacity = DATA_RECORD_COUNT;
    return request;
}

RuntimeAssetWorldObjectRestoreHandoffRequest MakeHandoffRequest(
    AuthoringRuntimeExportHandoffFixture *fixture,
    RuntimeAssetWorldObjectAdapterRequest *adapter_request,
    WorldInstance *world,
    ObjectRegistry *object_registry,
    ResourceRegistry *resource_registry,
    WorldObjectIdentityBridge *identity_destination,
    WorldTransformBridge *transform_destination,
    WorldComponentAttachmentBridge *attachment_destination,
    WorldComponentResourceBindingBridge *binding_destination,
    std::uint32_t attachment_count,
    std::uint32_t binding_count) {
    RuntimeAssetWorldObjectRestoreHandoffRequest request{};
    request.adapter_request = adapter_request;
    request.world = world;
    request.object_registry = object_registry;
    request.resource_registry = resource_registry;
    request.identity_destination = identity_destination;
    request.transform_destination = transform_destination;
    request.attachment_destination = attachment_destination;
    request.binding_destination = binding_destination;
    request.input_attachments = fixture->stream_output_attachments.data();
    request.input_attachment_count = attachment_count;
    request.input_bindings = fixture->stream_output_bindings.data();
    request.input_binding_count = binding_count;
    request.plan_scratch = fixture->plan_scratch.data();
    request.plan_scratch_capacity = SCRATCH_RECORD_COUNT;
    request.proof_scratch = fixture->proof_scratch.data();
    request.proof_scratch_capacity = SCRATCH_RECORD_COUNT;
    request.slice_scratch = fixture->slice_scratch.data();
    request.slice_scratch_capacity = SCRATCH_RECORD_COUNT;
    request.output_gates = fixture->gate_outputs.data();
    request.output_gate_capacity = SCRATCH_RECORD_COUNT;
    return request;
}

int VerifyRecordStreamSidecarOutputs(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    ResourceHandle texture_handle) {
    const WorldComponentAttachmentSnapshotRecord &stream_attachment =
        fixture.stream_output_attachments[0U];
    if (stream_attachment.world_object_id.value != WORLD_OBJECT_MODEL.value) {
        return Fail("authoring record stream attachment world object mismatch");
    }

    if (stream_attachment.component_type_id.value != COMPONENT_TYPE_MESH.value) {
        return Fail("authoring record stream attachment component type mismatch");
    }

    if (stream_attachment.component_slot_id.value != COMPONENT_SLOT_MESH.value) {
        return Fail("authoring record stream attachment component slot mismatch");
    }

    const WorldComponentResourceBindingSnapshotRecord &stream_binding =
        fixture.stream_output_bindings[0U];
    if (stream_binding.world_object_id.value != WORLD_OBJECT_MODEL.value) {
        return Fail("authoring record stream binding world object mismatch");
    }

    if (stream_binding.component_type_id.value != COMPONENT_TYPE_MESH.value) {
        return Fail("authoring record stream binding component type mismatch");
    }

    if (stream_binding.component_slot_id.value != COMPONENT_SLOT_MESH.value) {
        return Fail("authoring record stream binding component slot mismatch");
    }

    if (!ResourceHandlesMatch(stream_binding.resource_handle, texture_handle)) {
        return Fail("authoring record stream binding resource handle mismatch");
    }

    if (stream_binding.expected_resource_type.value != RESOURCE_TYPE_TEXTURE.value) {
        return Fail("authoring record stream binding resource type mismatch");
    }

    return 0;
}

int VerifyRecordStreamOutputs(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    std::uint32_t attachment_count,
    std::uint32_t binding_count,
    ResourceHandle texture_handle) {
    std::uint32_t index = 0U;
    while (index < DATA_RECORD_COUNT) {
        const WorldSceneObjectTransformRestoreIdentityRecord &stream_identity =
            fixture.stream_output_identities[index];
        const WorldSceneObjectTransformRestoreIdentityRecord &expected_identity =
            fixture.authoring_output_identities[index];
        if (stream_identity.world_object_id.value != expected_identity.world_object_id.value) {
            return Fail("authoring record stream identity world object mismatch");
        }

        if (!ObjectHandlesMatch(stream_identity.object_handle, expected_identity.object_handle)) {
            return Fail("authoring record stream identity object handle mismatch");
        }

        const WorldSceneObjectTransformRestoreTransformRecord &stream_transform =
            fixture.stream_output_transforms[index];
        const WorldSceneObjectTransformRestoreTransformRecord &expected_transform =
            fixture.authoring_output_transforms[index];
        if (stream_transform.world_object_id.value != expected_transform.world_object_id.value) {
            return Fail("authoring record stream transform world object mismatch");
        }

        if (!TransformsMatch(stream_transform.transform_state, expected_transform.transform_state)) {
            return Fail("authoring record stream transform state mismatch");
        }

        ++index;
    }

    if (attachment_count == 0U) {
        return 0;
    }

    if (binding_count == 0U) {
        return Fail("authoring record stream missing binding sidecar");
    }

    if (VerifyRecordStreamSidecarOutputs(fixture, texture_handle) != 0) {
        return 1;
    }

    return 0;
}

int VerifyIdentityRecordChain(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    std::uint32_t record_index) {
    const WorldSceneDecodedRestorePlanRecord &plan = fixture.plan_scratch[record_index];
    if (plan.family != WorldSceneDecodedRestorePlanRecordFamily::Identity) {
        return Fail("authoring decoded identity family mismatch");
    }

    if (plan.input_index != record_index) {
        return Fail("authoring decoded identity input mismatch");
    }

    const WorldSceneObjectTransformRestoreIdentityRecord &expected_identity =
        fixture.authoring_output_identities[record_index];
    if (plan.world_object_id.value != expected_identity.world_object_id.value) {
        return Fail("authoring decoded identity world object mismatch");
    }

    if (!ObjectHandlesMatch(plan.object_handle, expected_identity.object_handle)) {
        return Fail("authoring decoded identity object handle mismatch");
    }

    if (plan.projected_object_acquire_count != 1U) {
        return Fail("authoring decoded identity acquire count mismatch");
    }

    if (plan.status != WorldSceneDecodedRestorePlanStatus::Success) {
        return Fail("authoring decoded identity status mismatch");
    }

    const WorldSceneApplyTimeRestoreProofRecord &proof = fixture.proof_scratch[record_index];
    if (proof.family != WorldSceneApplyTimeRestoreProofFamily::Identity) {
        return Fail("authoring proof identity family mismatch");
    }

    if (proof.plan_index != record_index) {
        return Fail("authoring proof identity plan index mismatch");
    }

    if (proof.input_index != record_index) {
        return Fail("authoring proof identity input mismatch");
    }

    if (proof.status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return Fail("authoring proof identity status mismatch");
    }

    const WorldSceneApplyTimeRestoreProofSliceRecord &slice = fixture.slice_scratch[record_index];
    if (slice.family != WorldSceneApplyTimeRestoreProofFamily::Identity) {
        return Fail("authoring slice identity family mismatch");
    }

    if (slice.plan_index != record_index) {
        return Fail("authoring slice identity plan index mismatch");
    }

    const WorldSceneActiveRestoreGateRecord &gate = fixture.gate_outputs[record_index];
    if (gate.family != WorldSceneApplyTimeRestoreProofFamily::Identity) {
        return Fail("authoring gate identity family mismatch");
    }

    if (gate.gate_index != record_index) {
        return Fail("authoring gate identity index mismatch");
    }

    if (gate.plan_index != record_index) {
        return Fail("authoring gate identity plan index mismatch");
    }

    if (gate.status != WorldSceneActiveRestoreGateStatus::Success) {
        return Fail("authoring gate identity status mismatch");
    }

    return 0;
}

int VerifyTransformRecordChain(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    std::uint32_t transform_index) {
    const std::uint32_t plan_index = DATA_RECORD_COUNT + transform_index;
    const WorldSceneDecodedRestorePlanRecord &plan = fixture.plan_scratch[plan_index];
    if (plan.family != WorldSceneDecodedRestorePlanRecordFamily::Transform) {
        return Fail("authoring decoded transform family mismatch");
    }

    if (plan.input_index != transform_index) {
        return Fail("authoring decoded transform input mismatch");
    }

    const WorldSceneObjectTransformRestoreTransformRecord &expected_transform =
        fixture.authoring_output_transforms[transform_index];
    if (plan.world_object_id.value != expected_transform.world_object_id.value) {
        return Fail("authoring decoded transform world object mismatch");
    }

    if (plan.status != WorldSceneDecodedRestorePlanStatus::Success) {
        return Fail("authoring decoded transform status mismatch");
    }

    const WorldSceneApplyTimeRestoreProofRecord &proof = fixture.proof_scratch[plan_index];
    if (proof.family != WorldSceneApplyTimeRestoreProofFamily::Transform) {
        return Fail("authoring proof transform family mismatch");
    }

    if (proof.plan_index != plan_index) {
        return Fail("authoring proof transform plan index mismatch");
    }

    if (proof.input_index != transform_index) {
        return Fail("authoring proof transform input mismatch");
    }

    if (proof.status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return Fail("authoring proof transform status mismatch");
    }

    const WorldSceneApplyTimeRestoreProofSliceRecord &slice = fixture.slice_scratch[plan_index];
    if (slice.family != WorldSceneApplyTimeRestoreProofFamily::Transform) {
        return Fail("authoring slice transform family mismatch");
    }

    if (slice.plan_index != plan_index) {
        return Fail("authoring slice transform plan index mismatch");
    }

    const WorldSceneActiveRestoreGateRecord &gate = fixture.gate_outputs[plan_index];
    if (gate.family != WorldSceneApplyTimeRestoreProofFamily::Transform) {
        return Fail("authoring gate transform family mismatch");
    }

    if (gate.gate_index != plan_index) {
        return Fail("authoring gate transform index mismatch");
    }

    if (gate.input_index != transform_index) {
        return Fail("authoring gate transform input mismatch");
    }

    if (gate.status != WorldSceneActiveRestoreGateStatus::Success) {
        return Fail("authoring gate transform status mismatch");
    }

    return 0;
}

int VerifyAttachmentRecordChain(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    std::uint32_t attachment_index) {
    const std::uint32_t plan_index = STREAM_PLAN_RECORD_COUNT + attachment_index;
    const WorldSceneDecodedRestorePlanRecord &plan = fixture.plan_scratch[plan_index];
    if (plan.family != WorldSceneDecodedRestorePlanRecordFamily::Attachment) {
        return Fail("authoring decoded attachment family mismatch");
    }

    if (plan.input_index != attachment_index) {
        return Fail("authoring decoded attachment input mismatch");
    }

    if (plan.world_object_id.value != WORLD_OBJECT_MODEL.value) {
        return Fail("authoring decoded attachment world object mismatch");
    }

    if (plan.component_type_id.value != COMPONENT_TYPE_MESH.value) {
        return Fail("authoring decoded attachment component type mismatch");
    }

    if (plan.component_slot_id.value != COMPONENT_SLOT_MESH.value) {
        return Fail("authoring decoded attachment component slot mismatch");
    }

    if (plan.status != WorldSceneDecodedRestorePlanStatus::Success) {
        return Fail("authoring decoded attachment status mismatch");
    }

    const WorldSceneApplyTimeRestoreProofRecord &proof = fixture.proof_scratch[plan_index];
    if (proof.family != WorldSceneApplyTimeRestoreProofFamily::Attachment) {
        return Fail("authoring proof attachment family mismatch");
    }

    if (proof.plan_index != plan_index) {
        return Fail("authoring proof attachment plan index mismatch");
    }

    if (proof.input_index != attachment_index) {
        return Fail("authoring proof attachment input mismatch");
    }

    if (proof.status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return Fail("authoring proof attachment status mismatch");
    }

    const WorldSceneApplyTimeRestoreProofSliceRecord &slice = fixture.slice_scratch[plan_index];
    if (slice.family != WorldSceneApplyTimeRestoreProofFamily::Attachment) {
        return Fail("authoring slice attachment family mismatch");
    }

    if (slice.plan_index != plan_index) {
        return Fail("authoring slice attachment plan index mismatch");
    }

    const WorldSceneActiveRestoreGateRecord &gate = fixture.gate_outputs[plan_index];
    if (gate.family != WorldSceneApplyTimeRestoreProofFamily::Attachment) {
        return Fail("authoring gate attachment family mismatch");
    }

    if (gate.gate_index != plan_index) {
        return Fail("authoring gate attachment index mismatch");
    }

    if (gate.status != WorldSceneActiveRestoreGateStatus::Success) {
        return Fail("authoring gate attachment status mismatch");
    }

    return 0;
}

int VerifyBindingRecordChain(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    std::uint32_t binding_index,
    ResourceHandle texture_handle) {
    const std::uint32_t plan_index = STREAM_PLAN_RECORD_COUNT + SIDECAR_RECORD_COUNT + binding_index;
    const WorldSceneDecodedRestorePlanRecord &plan = fixture.plan_scratch[plan_index];
    if (plan.family != WorldSceneDecodedRestorePlanRecordFamily::Binding) {
        return Fail("authoring decoded binding family mismatch");
    }

    if (plan.input_index != binding_index) {
        return Fail("authoring decoded binding input mismatch");
    }

    if (plan.world_object_id.value != WORLD_OBJECT_MODEL.value) {
        return Fail("authoring decoded binding world object mismatch");
    }

    if (plan.component_type_id.value != COMPONENT_TYPE_MESH.value) {
        return Fail("authoring decoded binding component type mismatch");
    }

    if (plan.component_slot_id.value != COMPONENT_SLOT_MESH.value) {
        return Fail("authoring decoded binding component slot mismatch");
    }

    if (!ResourceHandlesMatch(plan.resource_handle, texture_handle)) {
        return Fail("authoring decoded binding resource handle mismatch");
    }

    if (plan.expected_resource_type.value != RESOURCE_TYPE_TEXTURE.value) {
        return Fail("authoring decoded binding resource type mismatch");
    }

    if (plan.projected_resource_acquire_count != 1U) {
        return Fail("authoring decoded binding resource acquire count mismatch");
    }

    const WorldSceneApplyTimeRestoreProofRecord &proof = fixture.proof_scratch[plan_index];
    if (proof.family != WorldSceneApplyTimeRestoreProofFamily::Binding) {
        return Fail("authoring proof binding family mismatch");
    }

    if (proof.plan_index != plan_index) {
        return Fail("authoring proof binding plan index mismatch");
    }

    if (!ResourceHandlesMatch(proof.resource_handle, texture_handle)) {
        return Fail("authoring proof binding resource handle mismatch");
    }

    if (proof.projected_resource_acquire_count != 1U) {
        return Fail("authoring proof binding resource acquire count mismatch");
    }

    if (proof.status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return Fail("authoring proof binding status mismatch");
    }

    const WorldSceneActiveRestoreGateRecord &gate = fixture.gate_outputs[plan_index];
    if (gate.family != WorldSceneApplyTimeRestoreProofFamily::Binding) {
        return Fail("authoring gate binding family mismatch");
    }

    if (gate.gate_index != plan_index) {
        return Fail("authoring gate binding index mismatch");
    }

    if (!ResourceHandlesMatch(gate.resource_handle, texture_handle)) {
        return Fail("authoring gate binding resource handle mismatch");
    }

    if (gate.expected_resource_type.value != RESOURCE_TYPE_TEXTURE.value) {
        return Fail("authoring gate binding resource type mismatch");
    }

    if (gate.status != WorldSceneActiveRestoreGateStatus::Success) {
        return Fail("authoring gate binding status mismatch");
    }

    return 0;
}

int VerifyDecodedRecordStreams(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    std::uint32_t attachment_count,
    std::uint32_t binding_count,
    ResourceHandle texture_handle) {
    std::uint32_t identity_index = 0U;
    while (identity_index < DATA_RECORD_COUNT) {
        if (VerifyIdentityRecordChain(fixture, identity_index) != 0) {
            return 1;
        }

        ++identity_index;
    }

    std::uint32_t transform_index = 0U;
    while (transform_index < DATA_RECORD_COUNT) {
        if (VerifyTransformRecordChain(fixture, transform_index) != 0) {
            return 1;
        }

        ++transform_index;
    }

    std::uint32_t attachment_index = 0U;
    while (attachment_index < attachment_count) {
        if (VerifyAttachmentRecordChain(fixture, attachment_index) != 0) {
            return 1;
        }

        ++attachment_index;
    }

    std::uint32_t binding_index = 0U;
    while (binding_index < binding_count) {
        if (VerifyBindingRecordChain(fixture, binding_index, texture_handle) != 0) {
            return 1;
        }

        ++binding_index;
    }

    return 0;
}

int VerifyRestoredTransforms(
    WorldTransformBridge *transform_destination,
    const AuthoringRuntimeExportHandoffFixture &fixture) {
    if (transform_destination == nullptr) {
        return Fail("authoring transform destination missing");
    }

    std::uint32_t index = 0U;
    while (index < DATA_RECORD_COUNT) {
        const WorldSceneObjectTransformRestoreTransformRecord &expected_transform =
            fixture.authoring_output_transforms[index];
        const WorldTransformResult result =
            transform_destination->Query(expected_transform.world_object_id);
        if (!result.Succeeded()) {
            return Fail("authoring restored transform query failed");
        }

        if (!TransformsMatch(result.transform_state, expected_transform.transform_state)) {
            return Fail("authoring restored transform mismatch");
        }

        ++index;
    }

    return 0;
}

int VerifyRestoredSidecarDestinations(
    WorldComponentAttachmentBridge *attachment_destination,
    WorldComponentResourceBindingBridge *binding_destination,
    ResourceHandle texture_handle) {
    if (attachment_destination == nullptr) {
        return Fail("authoring sidecar attachment destination missing");
    }

    if (binding_destination == nullptr) {
        return Fail("authoring sidecar binding destination missing");
    }

    const auto attachment = attachment_destination->Query(WORLD_OBJECT_MODEL, COMPONENT_TYPE_MESH);
    if (!attachment.Succeeded()) {
        return Fail("authoring sidecar attachment query failed");
    }

    if (attachment.component_slot_id.value != COMPONENT_SLOT_MESH.value) {
        return Fail("authoring sidecar attachment slot mismatch");
    }

    const auto binding =
        binding_destination->Query(WORLD_OBJECT_MODEL, COMPONENT_TYPE_MESH, COMPONENT_SLOT_MESH);
    if (!binding.Succeeded()) {
        return Fail("authoring sidecar binding query failed");
    }

    if (!ResourceHandlesMatch(binding.resource_handle, texture_handle)) {
        return Fail("authoring sidecar binding resource handle mismatch");
    }

    if (binding.expected_resource_type.value != RESOURCE_TYPE_TEXTURE.value) {
        return Fail("authoring sidecar binding resource type mismatch");
    }

    return 0;
}

int CommitExportedAuthoringDependencyEdge(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    ResourceRegistry *resource_registry,
    ResourceHandle document_handle) {
    if (resource_registry == nullptr) {
        return Fail("authoring dependency edge registry missing");
    }

    const WorldSceneAuthoringDependencyRecord &dependency =
        fixture.authoring_output_dependencies[0U];
    WorldSceneAuthoringDependencyRecord stable_id_only_dependency = dependency;
    stable_id_only_dependency.resource_handle = ResourceHandle{};

    const ResourceStatus stable_id_only_status =
        resource_registry->AddDependency(document_handle, stable_id_only_dependency.resource_handle);
    if (stable_id_only_status != ResourceStatus::DependencyMissing) {
        return Fail("authoring dependency edge accepted stable id without handle");
    }

    const ResourceSnapshot stable_id_only_snapshot = resource_registry->Snapshot();
    if (stable_id_only_snapshot.dependency_edge_count != 0U) {
        return Fail("authoring dependency edge stable id path changed edge count");
    }

    if (stable_id_only_snapshot.dependency_validation_count != 1U) {
        return Fail("authoring dependency edge stable id path validation count mismatch");
    }

    const ResourceStatus edge_status =
        resource_registry->AddDependency(document_handle, dependency.resource_handle);
    if (edge_status != ResourceStatus::Success) {
        return Fail("authoring dependency edge commit failed");
    }

    const ResourceSnapshot edge_snapshot = resource_registry->Snapshot();
    if (edge_snapshot.dependency_edge_count != 1U) {
        return Fail("authoring dependency edge commit count mismatch");
    }

    if (edge_snapshot.dependency_validation_count != 2U) {
        return Fail("authoring dependency edge validation count mismatch");
    }

    return 0;
}

int CommitExportedAuthoringAssetDependencyEdge(
    const AuthoringRuntimeExportHandoffFixture &fixture,
    AssetManager *asset_manager,
    AssetHandle document_asset_handle,
    AssetHandle dependency_asset_handle) {
    if (asset_manager == nullptr) {
        return Fail("authoring asset dependency edge manager missing");
    }

    const WorldSceneAuthoringDependencyRecord &dependency =
        fixture.authoring_output_dependencies[0U];

    const AssetStatus stable_id_only_status =
        asset_manager->AddDependency(document_asset_handle, AssetHandle{});
    if (stable_id_only_status != AssetStatus::InvalidHandle) {
        return Fail("authoring asset dependency edge accepted default handle");
    }

    const AssetSnapshot stable_id_only_snapshot = asset_manager->Snapshot();
    if (stable_id_only_snapshot.active_dependency_edge_count != 0U) {
        return Fail("authoring asset dependency edge default handle changed edge count");
    }

    AssetRecord dependency_asset{};
    const AssetStatus query_status =
        asset_manager->QueryAsset(dependency_asset_handle, &dependency_asset);
    if (query_status != AssetStatus::Success) {
        return Fail("authoring asset dependency edge dependency asset query failed");
    }

    if (!ResourceHandlesMatch(dependency_asset.resource, dependency.resource_handle)) {
        return Fail("authoring asset dependency edge resource handle mismatch");
    }

    if (dependency_asset.resource_type.value != dependency.expected_resource_type.value) {
        return Fail("authoring asset dependency edge resource type mismatch");
    }

    const AssetStatus edge_status =
        asset_manager->AddDependency(document_asset_handle, dependency_asset_handle);
    if (edge_status != AssetStatus::Success) {
        return Fail("authoring asset dependency edge commit failed");
    }

    const AssetSnapshot edge_snapshot = asset_manager->Snapshot();
    if (edge_snapshot.active_dependency_edge_count != 1U) {
        return Fail("authoring asset dependency edge commit count mismatch");
    }

    std::array<AssetHandle, 1U> dependencies{};
    std::uint32_t dependency_count = 0U;
    const AssetStatus traverse_status = asset_manager->TraverseDependencies(
        document_asset_handle,
        dependencies.data(),
        static_cast<std::uint32_t>(dependencies.size()),
        &dependency_count);
    if (traverse_status != AssetStatus::Success) {
        return Fail("authoring asset dependency edge traversal failed");
    }

    if (dependency_count != 1U) {
        return Fail("authoring asset dependency edge traversal count mismatch");
    }

    if (!AssetHandlesMatch(dependencies[0U], dependency_asset_handle)) {
        return Fail("authoring asset dependency edge traversal handle mismatch");
    }

    return 0;
}

int TestFeedAuthoringRuntimeExportThroughRecordStreamIntoRestoreHandoff() {
    AuthoringRuntimeExportHandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareCallerOwnedRegistries(&world, &object_registry, &fixture) != 0) {
        return 1;
    }

    if (ExportAuthoringRuntimeRecords(&fixture, 0U, 0U, 0U) != 0) {
        return 1;
    }

    if (VerifyAuthoringRuntimeOutputs(fixture, 0U, 0U, 0U, ResourceHandle{}) != 0) {
        return 1;
    }

    if (RoundTripWorldSceneRecordStream(&fixture, 0U, 0U) != 0) {
        return 1;
    }

    if (VerifyRecordStreamOutputs(fixture, 0U, 0U, ResourceHandle{}) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    WorldObjectIdentityBridge identity_destination(world, object_registry);
    WorldTransformBridge transform_destination(world);
    WorldComponentAttachmentBridge attachment_destination{};
    WorldComponentResourceBindingBridge binding_destination{};
    RuntimeAssetWorldObjectAdapterRequest adapter_request = MakeAdapterRequest(&fixture);
    RuntimeAssetWorldObjectRestoreHandoffRequest handoff_request = MakeHandoffRequest(
        &fixture,
        &adapter_request,
        &world,
        &object_registry,
        &resource_registry,
        &identity_destination,
        &transform_destination,
        &attachment_destination,
        &binding_destination,
        0U,
        0U);

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("authoring handoff failed");
    }

    if (result.state.input_mapping_count != DATA_RECORD_COUNT) {
        return Fail("authoring handoff input mapping count mismatch");
    }

    if (result.state.output_identity_count != DATA_RECORD_COUNT) {
        return Fail("authoring handoff identity count mismatch");
    }

    if (result.state.output_transform_count != DATA_RECORD_COUNT) {
        return Fail("authoring handoff transform count mismatch");
    }

    if (result.state.proof_record_count != STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring handoff proof count mismatch");
    }

    if (result.state.slice_record_count != STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring handoff slice count mismatch");
    }

    if (result.state.gate_record_count != STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring handoff gate count mismatch");
    }

    if (result.state.restored_attachment_count != 0U) {
        return Fail("authoring handoff restored attachment count mismatch");
    }

    if (result.state.restored_binding_count != 0U) {
        return Fail("authoring handoff restored binding count mismatch");
    }

    if (result.state.restored_identity_count != DATA_RECORD_COUNT) {
        return Fail("authoring handoff restored identity count mismatch");
    }

    if (result.state.restored_transform_count != DATA_RECORD_COUNT) {
        return Fail("authoring handoff restored transform count mismatch");
    }

    if (VerifyRecordStreamOutputs(fixture, 0U, 0U, ResourceHandle{}) != 0) {
        return 1;
    }

    if (VerifyDecodedRecordStreams(fixture, 0U, 0U, ResourceHandle{}) != 0) {
        return 1;
    }

    const WorldObjectIdentitySnapshot identity_snapshot = identity_destination.Snapshot();
    if (identity_snapshot.binding_count != DATA_RECORD_COUNT) {
        return Fail("authoring identity destination count mismatch");
    }

    const WorldTransformSnapshot transform_snapshot = transform_destination.Snapshot();
    if (transform_snapshot.record_count != DATA_RECORD_COUNT) {
        return Fail("authoring transform destination count mismatch");
    }

    if (VerifyRestoredTransforms(&transform_destination, fixture) != 0) {
        return 1;
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.accepted_handoff_count != 1U) {
        return Fail("authoring snapshot accepted count mismatch");
    }

    if (snapshot.emitted_gate_record_count != STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring snapshot gate count mismatch");
    }

    if (snapshot.restored_identity_count != DATA_RECORD_COUNT) {
        return Fail("authoring snapshot identity count mismatch");
    }

    if (snapshot.restored_transform_count != DATA_RECORD_COUNT) {
        return Fail("authoring snapshot transform count mismatch");
    }

    return 0;
}

int TestFeedAttachmentBindingDependencyExportThroughRecordStreamIntoRestoreHandoff() {
    AuthoringRuntimeExportHandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareCallerOwnedRegistries(&world, &object_registry, &fixture) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    ResourceHandle texture_handle{};
    if (PrepareAuthoringSidecarInputs(&fixture, &resource_registry, &texture_handle) != 0) {
        return 1;
    }

    const ResourceSnapshot resource_before_handoff = resource_registry.Snapshot();
    if (resource_before_handoff.dependency_edge_count != 0U) {
        return Fail("authoring dependency setup added resource dependency edge");
    }

    if (resource_before_handoff.dependency_validation_count != 0U) {
        return Fail("authoring dependency setup ran resource dependency validation");
    }

    if (ExportAuthoringRuntimeRecords(
            &fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            DEPENDENCY_RECORD_COUNT) != 0) {
        return 1;
    }

    if (VerifyAuthoringRuntimeOutputs(
            fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            DEPENDENCY_RECORD_COUNT,
            texture_handle) != 0) {
        return 1;
    }

    if (RoundTripWorldSceneRecordStream(&fixture, SIDECAR_RECORD_COUNT, SIDECAR_RECORD_COUNT) != 0) {
        return 1;
    }

    if (VerifyRecordStreamOutputs(
            fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            texture_handle) != 0) {
        return 1;
    }

    WorldObjectIdentityBridge identity_destination(world, object_registry);
    WorldTransformBridge transform_destination(world);
    WorldComponentAttachmentBridge attachment_destination{};
    WorldComponentResourceBindingBridge binding_destination{};
    RuntimeAssetWorldObjectAdapterRequest adapter_request = MakeAdapterRequest(&fixture);
    RuntimeAssetWorldObjectRestoreHandoffRequest handoff_request = MakeHandoffRequest(
        &fixture,
        &adapter_request,
        &world,
        &object_registry,
        &resource_registry,
        &identity_destination,
        &transform_destination,
        &attachment_destination,
        &binding_destination,
        SIDECAR_RECORD_COUNT,
        SIDECAR_RECORD_COUNT);

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("authoring sidecar handoff failed");
    }

    if (result.state.proof_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring sidecar handoff proof count mismatch");
    }

    if (result.state.slice_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring sidecar handoff slice count mismatch");
    }

    if (result.state.gate_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring sidecar handoff gate count mismatch");
    }

    if (result.state.restored_attachment_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring sidecar restored attachment count mismatch");
    }

    if (result.state.restored_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring sidecar restored binding count mismatch");
    }

    if (VerifyDecodedRecordStreams(
            fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            texture_handle) != 0) {
        return 1;
    }

    if (VerifyRestoredSidecarDestinations(
            &attachment_destination,
            &binding_destination,
            texture_handle) != 0) {
        return 1;
    }

    const auto attachment_snapshot = attachment_destination.Snapshot();
    if (attachment_snapshot.active_attachment_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring sidecar attachment destination count mismatch");
    }

    const auto binding_snapshot = binding_destination.Snapshot();
    if (binding_snapshot.active_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring sidecar binding destination count mismatch");
    }

    if (binding_snapshot.acquired_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring sidecar binding acquire count mismatch");
    }

    const ResourceSnapshot resource_after_handoff = resource_registry.Snapshot();
    if (resource_after_handoff.dependency_edge_count != resource_before_handoff.dependency_edge_count) {
        return Fail("authoring sidecar handoff mutated resource dependency edge count");
    }

    if (resource_after_handoff.dependency_validation_count != resource_before_handoff.dependency_validation_count) {
        return Fail("authoring sidecar handoff ran resource dependency validation");
    }

    if (resource_after_handoff.acquired_handle_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring sidecar handoff resource acquire count mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.emitted_gate_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring sidecar snapshot gate count mismatch");
    }

    return 0;
}

int TestCommitExportedDependencyAsCallerOwnedResourceEdge() {
    AuthoringRuntimeExportHandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareCallerOwnedRegistries(&world, &object_registry, &fixture) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    ResourceHandle document_handle{};
    ResourceHandle texture_handle{};
    if (PrepareAuthoringDependencyEdgeInputs(
            &fixture,
            &resource_registry,
            &document_handle,
            &texture_handle) != 0) {
        return 1;
    }

    const ResourceSnapshot resource_after_setup = resource_registry.Snapshot();
    if (resource_after_setup.dependency_edge_count != 0U) {
        return Fail("authoring dependency edge setup changed edge count");
    }

    if (resource_after_setup.dependency_validation_count != 0U) {
        return Fail("authoring dependency edge setup changed validation count");
    }

    if (ExportAuthoringRuntimeRecords(
            &fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            DEPENDENCY_RECORD_COUNT) != 0) {
        return 1;
    }

    if (VerifyAuthoringRuntimeOutputs(
            fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            DEPENDENCY_RECORD_COUNT,
            texture_handle) != 0) {
        return 1;
    }

    if (CommitExportedAuthoringDependencyEdge(
            fixture,
            &resource_registry,
            document_handle) != 0) {
        return 1;
    }

    const ResourceSnapshot resource_after_edge_commit = resource_registry.Snapshot();
    if (resource_after_edge_commit.dependency_edge_count != 1U) {
        return Fail("authoring dependency edge committed count missing");
    }

    if (resource_after_edge_commit.dependency_validation_count != 2U) {
        return Fail("authoring dependency edge committed validation count mismatch");
    }

    if (RoundTripWorldSceneRecordStream(&fixture, SIDECAR_RECORD_COUNT, SIDECAR_RECORD_COUNT) != 0) {
        return 1;
    }

    if (VerifyRecordStreamOutputs(
            fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            texture_handle) != 0) {
        return 1;
    }

    WorldObjectIdentityBridge identity_destination(world, object_registry);
    WorldTransformBridge transform_destination(world);
    WorldComponentAttachmentBridge attachment_destination{};
    WorldComponentResourceBindingBridge binding_destination{};
    RuntimeAssetWorldObjectAdapterRequest adapter_request = MakeAdapterRequest(&fixture);
    RuntimeAssetWorldObjectRestoreHandoffRequest handoff_request = MakeHandoffRequest(
        &fixture,
        &adapter_request,
        &world,
        &object_registry,
        &resource_registry,
        &identity_destination,
        &transform_destination,
        &attachment_destination,
        &binding_destination,
        SIDECAR_RECORD_COUNT,
        SIDECAR_RECORD_COUNT);

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("authoring dependency edge handoff failed");
    }

    if (result.state.proof_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring dependency edge handoff proof count mismatch");
    }

    if (result.state.slice_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring dependency edge handoff slice count mismatch");
    }

    if (result.state.gate_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring dependency edge handoff gate count mismatch");
    }

    if (result.state.restored_attachment_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring dependency edge restored attachment count mismatch");
    }

    if (result.state.restored_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring dependency edge restored binding count mismatch");
    }

    if (VerifyDecodedRecordStreams(
            fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            texture_handle) != 0) {
        return 1;
    }

    if (VerifyRestoredSidecarDestinations(
            &attachment_destination,
            &binding_destination,
            texture_handle) != 0) {
        return 1;
    }

    const auto attachment_snapshot = attachment_destination.Snapshot();
    if (attachment_snapshot.active_attachment_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring dependency edge attachment destination count mismatch");
    }

    const auto binding_snapshot = binding_destination.Snapshot();
    if (binding_snapshot.active_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring dependency edge binding destination count mismatch");
    }

    if (binding_snapshot.acquired_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring dependency edge binding acquire count mismatch");
    }

    const ResourceSnapshot resource_after_handoff = resource_registry.Snapshot();
    if (resource_after_handoff.dependency_edge_count != resource_after_edge_commit.dependency_edge_count) {
        return Fail("authoring dependency edge handoff changed edge count");
    }

    if (resource_after_handoff.dependency_validation_count !=
        resource_after_edge_commit.dependency_validation_count) {
        return Fail("authoring dependency edge handoff changed validation count");
    }

    if (resource_after_handoff.acquired_handle_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring dependency edge handoff resource acquire count mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.emitted_gate_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring dependency edge snapshot gate count mismatch");
    }

    return 0;
}

int TestCommitExportedDependencyAsCallerOwnedAssetEdge() {
    AuthoringRuntimeExportHandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareCallerOwnedRegistries(&world, &object_registry, &fixture) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    AssetManager asset_manager = MakeAssetManager();
    AssetHandle document_asset_handle{};
    AssetHandle dependency_asset_handle{};
    ResourceHandle texture_handle{};
    if (PrepareAuthoringAssetDependencyEdgeInputs(
            &fixture,
            &resource_registry,
            &asset_manager,
            &document_asset_handle,
            &dependency_asset_handle,
            &texture_handle) != 0) {
        return 1;
    }

    const AssetSnapshot asset_after_setup = asset_manager.Snapshot();
    if (asset_after_setup.active_dependency_edge_count != 0U) {
        return Fail("authoring asset dependency edge setup changed edge count");
    }

    if (ExportAuthoringRuntimeRecords(
            &fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            DEPENDENCY_RECORD_COUNT) != 0) {
        return 1;
    }

    if (VerifyAuthoringRuntimeOutputs(
            fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            DEPENDENCY_RECORD_COUNT,
            texture_handle) != 0) {
        return 1;
    }

    if (CommitExportedAuthoringAssetDependencyEdge(
            fixture,
            &asset_manager,
            document_asset_handle,
            dependency_asset_handle) != 0) {
        return 1;
    }

    const AssetSnapshot asset_after_edge_commit = asset_manager.Snapshot();
    if (asset_after_edge_commit.active_dependency_edge_count != 1U) {
        return Fail("authoring asset dependency edge committed count missing");
    }

    if (RoundTripWorldSceneRecordStream(&fixture, SIDECAR_RECORD_COUNT, SIDECAR_RECORD_COUNT) != 0) {
        return 1;
    }

    if (VerifyRecordStreamOutputs(
            fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            texture_handle) != 0) {
        return 1;
    }

    WorldObjectIdentityBridge identity_destination(world, object_registry);
    WorldTransformBridge transform_destination(world);
    WorldComponentAttachmentBridge attachment_destination{};
    WorldComponentResourceBindingBridge binding_destination{};
    RuntimeAssetWorldObjectAdapterRequest adapter_request = MakeAdapterRequest(&fixture);
    RuntimeAssetWorldObjectRestoreHandoffRequest handoff_request = MakeHandoffRequest(
        &fixture,
        &adapter_request,
        &world,
        &object_registry,
        &resource_registry,
        &identity_destination,
        &transform_destination,
        &attachment_destination,
        &binding_destination,
        SIDECAR_RECORD_COUNT,
        SIDECAR_RECORD_COUNT);

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("authoring asset dependency edge handoff failed");
    }

    if (result.state.proof_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring asset dependency edge handoff proof count mismatch");
    }

    if (result.state.slice_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring asset dependency edge handoff slice count mismatch");
    }

    if (result.state.gate_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring asset dependency edge handoff gate count mismatch");
    }

    if (result.state.restored_attachment_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring asset dependency edge restored attachment count mismatch");
    }

    if (result.state.restored_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring asset dependency edge restored binding count mismatch");
    }

    if (VerifyDecodedRecordStreams(
            fixture,
            SIDECAR_RECORD_COUNT,
            SIDECAR_RECORD_COUNT,
            texture_handle) != 0) {
        return 1;
    }

    if (VerifyRestoredSidecarDestinations(
            &attachment_destination,
            &binding_destination,
            texture_handle) != 0) {
        return 1;
    }

    const auto binding_snapshot = binding_destination.Snapshot();
    if (binding_snapshot.acquired_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("authoring asset dependency edge binding acquire count mismatch");
    }

    const AssetSnapshot asset_after_handoff = asset_manager.Snapshot();
    if (asset_after_handoff.active_dependency_edge_count !=
        asset_after_edge_commit.active_dependency_edge_count) {
        return Fail("authoring asset dependency edge handoff changed edge count");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.emitted_gate_record_count != SIDECAR_STREAM_PLAN_RECORD_COUNT) {
        return Fail("authoring asset dependency edge snapshot gate count mismatch");
    }

    return 0;
}

}

bool RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTestNameMatches(std::string_view test_name) {
    if (test_name == TEST_FEED_AUTHORING_RUNTIME_EXPORT) {
        return true;
    }

    if (test_name == TEST_FEED_AUTHORING_SIDECAR_RUNTIME_EXPORT) {
        return true;
    }

    if (test_name == TEST_COMMIT_AUTHORING_DEPENDENCY_EDGE_RUNTIME_EXPORT) {
        return true;
    }

    return test_name == TEST_COMMIT_AUTHORING_ASSET_DEPENDENCY_EDGE_RUNTIME_EXPORT;
}

int RunRuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest(std::string_view test_name) {
    if (RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTestNameMatches(test_name)) {
        if (test_name == TEST_COMMIT_AUTHORING_ASSET_DEPENDENCY_EDGE_RUNTIME_EXPORT) {
            return TestCommitExportedDependencyAsCallerOwnedAssetEdge();
        }

        if (test_name == TEST_COMMIT_AUTHORING_DEPENDENCY_EDGE_RUNTIME_EXPORT) {
            return TestCommitExportedDependencyAsCallerOwnedResourceEdge();
        }

        if (test_name == TEST_FEED_AUTHORING_SIDECAR_RUNTIME_EXPORT) {
            return TestFeedAttachmentBindingDependencyExportThroughRecordStreamIntoRestoreHandoff();
        }

        return TestFeedAuthoringRuntimeExportThroughRecordStreamIntoRestoreHandoff();
    }

    return Fail("unknown authoring runtime export handoff test name");
}
