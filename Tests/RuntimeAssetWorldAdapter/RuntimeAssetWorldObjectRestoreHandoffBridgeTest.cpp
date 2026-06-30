// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

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
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterIdentityRecord.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterStatus.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridge.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffResult.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffSnapshot.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffStatus.h"
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
#include "YuEngine/World/WorldSceneAssemblyBridgeDesc.h"
#include "YuEngine/World/WorldSceneAssemblyStatus.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformSnapshot.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldTransformStatus.h"

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
using yuengine::resource::ResourceTypeId;
using yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord;
using yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord;
using yuengine::runtimeasset::RuntimeAssetTargetIdentityKind;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterIdentityRecord;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterStatus;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffBridge;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffResult;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffSnapshot;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffStatus;
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
using yuengine::world::WorldSceneAssemblyBridgeDesc;
using yuengine::world::WorldSceneAssemblyStatus;
using yuengine::world::WorldSceneDecodedRestorePlanRecord;
using yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using yuengine::world::WorldSceneObjectTransformRestoreStatus;
using yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;
using yuengine::world::WorldTransformBridge;
using yuengine::world::WorldTransformResult;
using yuengine::world::WorldTransformSnapshot;
using yuengine::world::WorldTransformState;
using yuengine::world::WorldTransformStatus;

bool RuntimeAssetWorldObjectDataHandoffFixtureTestNameMatches(std::string_view test_name);
int RunRuntimeAssetWorldObjectDataHandoffFixtureTest(std::string_view test_name);

namespace {
constexpr std::uint32_t OUTPUT_RECORD_COUNT = 2U;
constexpr std::uint32_t SCRATCH_RECORD_COUNT = 8U;
constexpr std::uint32_t SIDECAR_RECORD_COUNT = 2U;
constexpr const char *TEST_APPLY_RESTORE =
    "RuntimeAssetWorldObjectRestoreHandoff_AppliesAdapterRecordsThroughWorldRestoreBridge";
constexpr const char *TEST_APPLY_TARGET_FAMILY_ALIASES =
    "RuntimeAssetWorldObjectRestoreHandoff_AppliesModelAndSkeletonTargetFamilyAliases";
constexpr const char *TEST_APPLY_ATTACHMENT_BINDING_GATE_RECORDS =
    "RuntimeAssetWorldObjectRestoreHandoff_CarriesAttachmentAndBindingGateRecordsForTargetAliases";
constexpr const char *TEST_APPLY_SIDECAR_ASSEMBLY_RESTORE =
    "RuntimeAssetWorldObjectRestoreHandoff_RestoresAttachmentAndBindingSidecarsThroughWorldAssembly";
constexpr const char *TEST_REJECT_SIDECAR_ASSEMBLY_FAILURE_STATUS =
    "RuntimeAssetWorldObjectRestoreHandoff_ExposesSidecarAssemblyFailureStatus";
constexpr const char *TEST_REJECT_ADAPTER_PREFLIGHT =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsAdapterPreflightWithoutWorldMutation";
constexpr const char *TEST_REJECT_WORLD_GATE =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsWorldGateWithoutRestoringSidecars";
constexpr const char *TEST_REJECT_NULL_ADAPTER =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsNullAdapterRequestWithoutMutation";
constexpr WorldObjectId WORLD_OBJECT_A{1U};
constexpr WorldObjectId WORLD_OBJECT_B{2U};
constexpr WorldObjectId SENTINEL_WORLD_OBJECT{99U};
constexpr ObjectHandle SENTINEL_OBJECT_HANDLE{9U, 9U};
constexpr ObjectTypeId OBJECT_TYPE_PLAYER{1U};
constexpr ObjectTypeId OBJECT_TYPE_CAMERA{2U};
constexpr ResourceTypeId RESOURCE_TYPE_TEXTURE{1U};
constexpr ResourceTypeId RESOURCE_TYPE_MATERIAL{2U};
constexpr WorldComponentTypeId COMPONENT_TYPE_MESH{1U};
constexpr WorldComponentTypeId COMPONENT_TYPE_MATERIAL{2U};
constexpr WorldComponentSlotId COMPONENT_SLOT_MESH{11U};
constexpr WorldComponentSlotId COMPONENT_SLOT_MATERIAL{12U};
constexpr std::uint64_t TARGET_A = 1001U;
constexpr std::uint64_t TARGET_B = 1002U;
constexpr std::uint32_t ENTITY_A = 11U;
constexpr std::uint32_t ENTITY_B = 12U;

using TestFunction = int (*)();

struct TestCase final {
    std::string_view name;
    TestFunction function = nullptr;
};

struct HandoffFixture final {
    std::array<RuntimeAssetRuntimeInstanceMappingRecord, OUTPUT_RECORD_COUNT> mappings{};
    std::array<RuntimeAssetSceneEntityRecord, OUTPUT_RECORD_COUNT> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, OUTPUT_RECORD_COUNT> scene_transforms{};
    std::array<RuntimeAssetWorldObjectAdapterIdentityRecord, OUTPUT_RECORD_COUNT> identity_records{};
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, OUTPUT_RECORD_COUNT> output_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, OUTPUT_RECORD_COUNT> output_transforms{};
    std::array<WorldComponentAttachmentSnapshotRecord, SIDECAR_RECORD_COUNT> attachments{};
    std::array<WorldComponentResourceBindingSnapshotRecord, SIDECAR_RECORD_COUNT> bindings{};
    std::array<WorldSceneDecodedRestorePlanRecord, SCRATCH_RECORD_COUNT> plan_scratch{};
    std::array<WorldSceneApplyTimeRestoreProofRecord, SCRATCH_RECORD_COUNT> proof_scratch{};
    std::array<WorldSceneApplyTimeRestoreProofSliceRecord, SCRATCH_RECORD_COUNT> slice_scratch{};
    std::array<WorldSceneActiveRestoreGateRecord, SCRATCH_RECORD_COUNT> gate_outputs{};
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
    std::uint64_t target_id,
    std::uint32_t entity_id,
    std::uint32_t scene_entity_index,
    std::uint32_t scene_transform_index) {
    RuntimeAssetRuntimeInstanceMappingRecord record{};
    record.target_kind = RuntimeAssetTargetIdentityKind::SceneNode;
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

ResourceDescriptor Resource(ResourceTypeId type, const char *key) {
    ResourceDescriptor descriptor{};
    descriptor.type = type;
    descriptor.logical_key = ResourceLogicalKey(key);
    return descriptor;
}

int PrepareSidecarInputs(
    HandoffFixture *fixture,
    ResourceRegistry *resource_registry,
    ResourceHandle *texture_handle,
    ResourceHandle *material_handle) {
    if (fixture == nullptr) {
        return Fail("sidecar fixture missing");
    }

    if (resource_registry == nullptr) {
        return Fail("sidecar registry missing");
    }

    ResourceDescriptor texture_descriptor = Resource(RESOURCE_TYPE_TEXTURE, "handoff_texture");
    const ResourceRegistrationResult texture_resource =
        resource_registry->RegisterSyntheticDescriptor(texture_descriptor);
    if (!texture_resource.Succeeded()) {
        return Fail("target family sidecar texture setup failed");
    }

    ResourceDescriptor material_descriptor = Resource(RESOURCE_TYPE_MATERIAL, "handoff_material");
    const ResourceRegistrationResult material_resource =
        resource_registry->RegisterSyntheticDescriptor(material_descriptor);
    if (!material_resource.Succeeded()) {
        return Fail("target family sidecar material setup failed");
    }

    fixture->attachments[0U].world_object_id = WORLD_OBJECT_A;
    fixture->attachments[0U].component_type_id = COMPONENT_TYPE_MESH;
    fixture->attachments[0U].component_slot_id = COMPONENT_SLOT_MESH;
    fixture->attachments[1U].world_object_id = WORLD_OBJECT_B;
    fixture->attachments[1U].component_type_id = COMPONENT_TYPE_MATERIAL;
    fixture->attachments[1U].component_slot_id = COMPONENT_SLOT_MATERIAL;

    fixture->bindings[0U].world_object_id = WORLD_OBJECT_A;
    fixture->bindings[0U].component_type_id = COMPONENT_TYPE_MESH;
    fixture->bindings[0U].component_slot_id = COMPONENT_SLOT_MESH;
    fixture->bindings[0U].resource_handle = texture_resource.handle;
    fixture->bindings[0U].expected_resource_type = RESOURCE_TYPE_TEXTURE;
    fixture->bindings[1U].world_object_id = WORLD_OBJECT_B;
    fixture->bindings[1U].component_type_id = COMPONENT_TYPE_MATERIAL;
    fixture->bindings[1U].component_slot_id = COMPONENT_SLOT_MATERIAL;
    fixture->bindings[1U].resource_handle = material_resource.handle;
    fixture->bindings[1U].expected_resource_type = RESOURCE_TYPE_MATERIAL;

    if (texture_handle != nullptr) {
        *texture_handle = texture_resource.handle;
    }

    if (material_handle != nullptr) {
        *material_handle = material_resource.handle;
    }

    return 0;
}

WorldSceneObjectTransformRestoreIdentityRecord SentinelIdentityOutput() {
    WorldSceneObjectTransformRestoreIdentityRecord record{};
    record.world_object_id = SENTINEL_WORLD_OBJECT;
    record.object_handle = SENTINEL_OBJECT_HANDLE;
    return record;
}

WorldSceneObjectTransformRestoreTransformRecord SentinelTransformOutput() {
    WorldSceneObjectTransformRestoreTransformRecord record{};
    record.world_object_id = SENTINEL_WORLD_OBJECT;
    record.transform_state = Transform(90.0F);
    return record;
}

HandoffFixture MakeFixture() {
    HandoffFixture fixture{};
    fixture.mappings[0U] = Mapping(TARGET_A, ENTITY_A, 0U, 0U);
    fixture.mappings[1U] = Mapping(TARGET_B, ENTITY_B, 1U, 1U);
    fixture.scene_entities[0U] = Entity(ENTITY_A, WORLD_OBJECT_A, Transform(10.0F));
    fixture.scene_entities[1U] = Entity(ENTITY_B, WORLD_OBJECT_B, Transform(20.0F));
    fixture.scene_transforms[0U] = TransformOutput(WORLD_OBJECT_A, Transform(30.0F));
    fixture.scene_transforms[1U] = TransformOutput(WORLD_OBJECT_B, Transform(40.0F));
    fixture.identity_records[0U] = Identity(TARGET_A, WORLD_OBJECT_A, ObjectHandle{});
    fixture.identity_records[1U] = Identity(TARGET_B, WORLD_OBJECT_B, ObjectHandle{});

    std::uint32_t index = 0U;
    while (index < OUTPUT_RECORD_COUNT) {
        fixture.output_identities[index] = SentinelIdentityOutput();
        fixture.output_transforms[index] = SentinelTransformOutput();
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

ObjectRegistrationResult CreateObject(ObjectRegistry *object_registry, ObjectTypeId type) {
    ObjectDescriptor descriptor{};
    descriptor.type = type;
    descriptor.initial_reference_count = 0U;
    return object_registry->CreateSyntheticObject(descriptor);
}

int RegisterWorldObject(WorldInstance *world, WorldObjectId world_object_id) {
    WorldObjectDesc object_desc{};
    object_desc.id = world_object_id;
    object_desc.is_enabled = true;
    const WorldRegistrationResult world_result = world->RegisterObject(object_desc);
    if (!world_result.Succeeded()) {
        return Fail("world object setup failed");
    }

    return 0;
}

int PrepareWorldAndObjects(
    WorldInstance *world,
    ObjectRegistry *object_registry,
    HandoffFixture *fixture,
    bool register_second_world_object) {
    if (world == nullptr) {
        return Fail("world fixture missing");
    }

    if (object_registry == nullptr) {
        return Fail("object registry fixture missing");
    }

    if (fixture == nullptr) {
        return Fail("handoff fixture missing");
    }

    if (RegisterWorldObject(world, WORLD_OBJECT_A) != 0) {
        return 1;
    }

    if (register_second_world_object) {
        if (RegisterWorldObject(world, WORLD_OBJECT_B) != 0) {
            return 1;
        }
    }

    const ObjectRegistrationResult object_a = CreateObject(object_registry, OBJECT_TYPE_PLAYER);
    if (!object_a.Succeeded()) {
        return Fail("first object setup failed");
    }

    const ObjectRegistrationResult object_b = CreateObject(object_registry, OBJECT_TYPE_CAMERA);
    if (!object_b.Succeeded()) {
        return Fail("second object setup failed");
    }

    fixture->identity_records[0U].object_handle = object_a.handle;
    fixture->identity_records[1U].object_handle = object_b.handle;
    return 0;
}

RuntimeAssetWorldObjectAdapterRequest MakeAdapterRequest(HandoffFixture *fixture) {
    RuntimeAssetWorldObjectAdapterRequest request{};
    if (fixture == nullptr) {
        return request;
    }

    request.runtime_instance_mappings = fixture->mappings.data();
    request.runtime_instance_mapping_count = OUTPUT_RECORD_COUNT;
    request.scene_entities = fixture->scene_entities.data();
    request.scene_entity_count = OUTPUT_RECORD_COUNT;
    request.scene_transforms = fixture->scene_transforms.data();
    request.scene_transform_count = OUTPUT_RECORD_COUNT;
    request.identity_records = fixture->identity_records.data();
    request.identity_record_count = OUTPUT_RECORD_COUNT;
    request.output_identities = fixture->output_identities.data();
    request.output_identity_capacity = OUTPUT_RECORD_COUNT;
    request.output_transforms = fixture->output_transforms.data();
    request.output_transform_capacity = OUTPUT_RECORD_COUNT;
    return request;
}

RuntimeAssetWorldObjectRestoreHandoffRequest MakeHandoffRequest(
    HandoffFixture *fixture,
    RuntimeAssetWorldObjectAdapterRequest *adapter_request,
    WorldInstance *world,
    ObjectRegistry *object_registry,
    ResourceRegistry *resource_registry,
    WorldObjectIdentityBridge *identity_destination,
    WorldTransformBridge *transform_destination,
    WorldComponentAttachmentBridge *attachment_destination,
    WorldComponentResourceBindingBridge *binding_destination) {
    RuntimeAssetWorldObjectRestoreHandoffRequest request{};
    request.adapter_request = adapter_request;
    request.world = world;
    request.object_registry = object_registry;
    request.resource_registry = resource_registry;
    request.identity_destination = identity_destination;
    request.transform_destination = transform_destination;
    request.attachment_destination = attachment_destination;
    request.binding_destination = binding_destination;
    request.input_attachments = fixture->attachments.data();
    request.input_attachment_count = 0U;
    request.input_bindings = fixture->bindings.data();
    request.input_binding_count = 0U;
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

bool ObjectIdentitySnapshotsMatch(
    const WorldObjectIdentitySnapshot &left,
    const WorldObjectIdentitySnapshot &right) {
    if (left.bridge_capacity != right.bridge_capacity) {
        return false;
    }

    if (left.binding_count != right.binding_count) {
        return false;
    }

    if (left.acquired_handle_count != right.acquired_handle_count) {
        return false;
    }

    if (left.released_handle_count != right.released_handle_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
    }

    if (left.last_object_status != right.last_object_status) {
        return false;
    }

    return left.last_status == right.last_status;
}

bool TransformSnapshotsMatch(
    const WorldTransformSnapshot &left,
    const WorldTransformSnapshot &right) {
    if (left.bridge_capacity != right.bridge_capacity) {
        return false;
    }

    if (left.record_count != right.record_count) {
        return false;
    }

    if (left.updated_record_count != right.updated_record_count) {
        return false;
    }

    if (left.removed_record_count != right.removed_record_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
    }

    return left.last_status == right.last_status;
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

int VerifyRestoredSidecarDestinations(
    WorldComponentAttachmentBridge *attachment_destination,
    WorldComponentResourceBindingBridge *binding_destination,
    ResourceHandle texture_handle,
    ResourceHandle material_handle) {
    if (attachment_destination == nullptr) {
        return Fail("sidecar assembly attachment destination missing");
    }

    if (binding_destination == nullptr) {
        return Fail("sidecar assembly binding destination missing");
    }

    const auto model_attachment = attachment_destination->Query(WORLD_OBJECT_A, COMPONENT_TYPE_MESH);
    if (!model_attachment.Succeeded()) {
        return Fail("model node assembly attachment query failed");
    }

    if (model_attachment.component_slot_id.value != COMPONENT_SLOT_MESH.value) {
        return Fail("model node assembly attachment slot mismatch");
    }

    const auto skeleton_attachment = attachment_destination->Query(WORLD_OBJECT_B, COMPONENT_TYPE_MATERIAL);
    if (!skeleton_attachment.Succeeded()) {
        return Fail("skeleton joint assembly attachment query failed");
    }

    if (skeleton_attachment.component_slot_id.value != COMPONENT_SLOT_MATERIAL.value) {
        return Fail("skeleton joint assembly attachment slot mismatch");
    }

    const auto model_binding =
        binding_destination->Query(WORLD_OBJECT_A, COMPONENT_TYPE_MESH, COMPONENT_SLOT_MESH);
    if (!model_binding.Succeeded()) {
        return Fail("model node assembly binding query failed");
    }

    if (!ResourceHandlesMatch(model_binding.resource_handle, texture_handle)) {
        return Fail("model node assembly binding resource handle mismatch");
    }

    if (model_binding.expected_resource_type.value != RESOURCE_TYPE_TEXTURE.value) {
        return Fail("model node assembly binding resource type mismatch");
    }

    const auto skeleton_binding =
        binding_destination->Query(WORLD_OBJECT_B, COMPONENT_TYPE_MATERIAL, COMPONENT_SLOT_MATERIAL);
    if (!skeleton_binding.Succeeded()) {
        return Fail("skeleton joint assembly binding query failed");
    }

    if (!ResourceHandlesMatch(skeleton_binding.resource_handle, material_handle)) {
        return Fail("skeleton joint assembly binding resource handle mismatch");
    }

    if (skeleton_binding.expected_resource_type.value != RESOURCE_TYPE_MATERIAL.value) {
        return Fail("skeleton joint assembly binding resource type mismatch");
    }

    return 0;
}

bool OutputRecordsHaveSentinelValues(const HandoffFixture &fixture) {
    std::uint32_t index = 0U;
    while (index < OUTPUT_RECORD_COUNT) {
        const WorldSceneObjectTransformRestoreIdentityRecord &identity = fixture.output_identities[index];
        if (identity.world_object_id.value != SENTINEL_WORLD_OBJECT.value) {
            return false;
        }

        if (identity.object_handle.slot != SENTINEL_OBJECT_HANDLE.slot) {
            return false;
        }

        const WorldSceneObjectTransformRestoreTransformRecord &transform = fixture.output_transforms[index];
        if (transform.world_object_id.value != SENTINEL_WORLD_OBJECT.value) {
            return false;
        }

        ++index;
    }

    return true;
}

int TestApplyRestore() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
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
        &binding_destination);

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("handoff apply failed");
    }

    if (result.state.output_identity_count != OUTPUT_RECORD_COUNT) {
        return Fail("handoff identity count mismatch");
    }

    if (result.state.output_transform_count != OUTPUT_RECORD_COUNT) {
        return Fail("handoff transform count mismatch");
    }

    if (result.state.gate_record_count != OUTPUT_RECORD_COUNT + OUTPUT_RECORD_COUNT) {
        return Fail("handoff gate count mismatch");
    }

    const WorldObjectIdentitySnapshot identity_snapshot = identity_destination.Snapshot();
    if (identity_snapshot.binding_count != OUTPUT_RECORD_COUNT) {
        return Fail("handoff identity destination count mismatch");
    }

    const WorldTransformSnapshot transform_snapshot = transform_destination.Snapshot();
    if (transform_snapshot.record_count != OUTPUT_RECORD_COUNT) {
        return Fail("handoff transform destination count mismatch");
    }

    const WorldTransformResult transform_result = transform_destination.Query(WORLD_OBJECT_B);
    if (!transform_result.Succeeded()) {
        return Fail("handoff transform query failed");
    }

    if (!TransformsMatch(transform_result.transform_state, fixture.scene_transforms[1U].transform)) {
        return Fail("handoff restored transform mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.accepted_handoff_count != 1U) {
        return Fail("handoff snapshot accepted count mismatch");
    }

    if (snapshot.restored_identity_count != OUTPUT_RECORD_COUNT) {
        return Fail("handoff snapshot restored identity count mismatch");
    }

    return 0;
}

int TestApplyTargetFamilyAliases() {
    HandoffFixture fixture = MakeFixture();
    fixture.mappings[0U].target_kind = RuntimeAssetTargetIdentityKind::ModelNode;
    fixture.mappings[1U].target_kind = RuntimeAssetTargetIdentityKind::SkeletonJoint;

    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
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
        &binding_destination);

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("target family handoff apply failed");
    }

    if (result.state.input_mapping_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family handoff input count mismatch");
    }

    if (result.state.output_identity_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family handoff identity count mismatch");
    }

    if (result.state.output_transform_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family handoff transform count mismatch");
    }

    if (result.state.gate_record_count != OUTPUT_RECORD_COUNT + OUTPUT_RECORD_COUNT) {
        return Fail("target family handoff gate count mismatch");
    }

    if (result.state.restored_identity_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family handoff restored identity count mismatch");
    }

    if (result.state.restored_transform_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family handoff restored transform count mismatch");
    }

    if (fixture.output_identities[0U].world_object_id.value != WORLD_OBJECT_A.value) {
        return Fail("model node handoff identity world object mismatch");
    }

    if (fixture.output_identities[1U].world_object_id.value != WORLD_OBJECT_B.value) {
        return Fail("skeleton joint handoff identity world object mismatch");
    }

    if (!ObjectHandlesMatch(fixture.output_identities[0U].object_handle, fixture.identity_records[0U].object_handle)) {
        return Fail("model node handoff object handle mismatch");
    }

    if (!ObjectHandlesMatch(fixture.output_identities[1U].object_handle, fixture.identity_records[1U].object_handle)) {
        return Fail("skeleton joint handoff object handle mismatch");
    }

    if (fixture.output_transforms[0U].world_object_id.value != WORLD_OBJECT_A.value) {
        return Fail("model node handoff transform world object mismatch");
    }

    if (fixture.output_transforms[1U].world_object_id.value != WORLD_OBJECT_B.value) {
        return Fail("skeleton joint handoff transform world object mismatch");
    }

    if (!TransformsMatch(fixture.output_transforms[0U].transform_state, fixture.scene_transforms[0U].transform)) {
        return Fail("model node handoff transform state mismatch");
    }

    if (!TransformsMatch(fixture.output_transforms[1U].transform_state, fixture.scene_transforms[1U].transform)) {
        return Fail("skeleton joint handoff transform state mismatch");
    }

    const WorldObjectIdentitySnapshot identity_snapshot = identity_destination.Snapshot();
    if (identity_snapshot.binding_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family handoff identity destination count mismatch");
    }

    const WorldTransformSnapshot transform_snapshot = transform_destination.Snapshot();
    if (transform_snapshot.record_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family handoff transform destination count mismatch");
    }

    const WorldTransformResult model_transform_result = transform_destination.Query(WORLD_OBJECT_A);
    if (!model_transform_result.Succeeded()) {
        return Fail("model node handoff transform query failed");
    }

    if (!TransformsMatch(model_transform_result.transform_state, fixture.scene_transforms[0U].transform)) {
        return Fail("model node handoff restored transform mismatch");
    }

    const WorldTransformResult skeleton_transform_result = transform_destination.Query(WORLD_OBJECT_B);
    if (!skeleton_transform_result.Succeeded()) {
        return Fail("skeleton joint handoff transform query failed");
    }

    if (!TransformsMatch(skeleton_transform_result.transform_state, fixture.scene_transforms[1U].transform)) {
        return Fail("skeleton joint handoff restored transform mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.accepted_handoff_count != 1U) {
        return Fail("target family handoff snapshot accepted count mismatch");
    }

    if (snapshot.restored_identity_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family handoff snapshot restored identity count mismatch");
    }

    return 0;
}

int TestApplyAttachmentAndBindingGateRecords() {
    HandoffFixture fixture = MakeFixture();
    fixture.mappings[0U].target_kind = RuntimeAssetTargetIdentityKind::ModelNode;
    fixture.mappings[1U].target_kind = RuntimeAssetTargetIdentityKind::SkeletonJoint;

    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    ResourceHandle texture_handle{};
    ResourceHandle material_handle{};
    if (PrepareSidecarInputs(&fixture, &resource_registry, &texture_handle, &material_handle) != 0) {
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
        &binding_destination);
    handoff_request.input_attachment_count = SIDECAR_RECORD_COUNT;
    handoff_request.input_binding_count = SIDECAR_RECORD_COUNT;

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("target family sidecar handoff apply failed");
    }

    if (result.state.proof_record_count != SCRATCH_RECORD_COUNT) {
        return Fail("target family sidecar proof count mismatch");
    }

    if (result.state.slice_record_count != SCRATCH_RECORD_COUNT) {
        return Fail("target family sidecar slice count mismatch");
    }

    if (result.state.gate_record_count != SCRATCH_RECORD_COUNT) {
        return Fail("target family sidecar gate count mismatch");
    }

    if (result.state.restored_identity_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family sidecar restored identity count mismatch");
    }

    if (result.state.restored_transform_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family sidecar restored transform count mismatch");
    }

    const WorldSceneActiveRestoreGateRecord &model_attachment_gate = fixture.gate_outputs[4U];
    if (model_attachment_gate.family != WorldSceneApplyTimeRestoreProofFamily::Attachment) {
        return Fail("model node attachment gate family mismatch");
    }

    if (model_attachment_gate.input_index != 0U) {
        return Fail("model node attachment gate input mismatch");
    }

    if (model_attachment_gate.world_object_id.value != WORLD_OBJECT_A.value) {
        return Fail("model node attachment gate world object mismatch");
    }

    if (model_attachment_gate.component_type_id.value != COMPONENT_TYPE_MESH.value) {
        return Fail("model node attachment gate component type mismatch");
    }

    if (model_attachment_gate.component_slot_id.value != COMPONENT_SLOT_MESH.value) {
        return Fail("model node attachment gate component slot mismatch");
    }

    const WorldSceneActiveRestoreGateRecord &skeleton_attachment_gate = fixture.gate_outputs[5U];
    if (skeleton_attachment_gate.family != WorldSceneApplyTimeRestoreProofFamily::Attachment) {
        return Fail("skeleton joint attachment gate family mismatch");
    }

    if (skeleton_attachment_gate.input_index != 1U) {
        return Fail("skeleton joint attachment gate input mismatch");
    }

    if (skeleton_attachment_gate.world_object_id.value != WORLD_OBJECT_B.value) {
        return Fail("skeleton joint attachment gate world object mismatch");
    }

    if (skeleton_attachment_gate.component_type_id.value != COMPONENT_TYPE_MATERIAL.value) {
        return Fail("skeleton joint attachment gate component type mismatch");
    }

    if (skeleton_attachment_gate.component_slot_id.value != COMPONENT_SLOT_MATERIAL.value) {
        return Fail("skeleton joint attachment gate component slot mismatch");
    }

    const WorldSceneActiveRestoreGateRecord &model_binding_gate = fixture.gate_outputs[6U];
    if (model_binding_gate.family != WorldSceneApplyTimeRestoreProofFamily::Binding) {
        return Fail("model node binding gate family mismatch");
    }

    if (model_binding_gate.input_index != 0U) {
        return Fail("model node binding gate input mismatch");
    }

    if (model_binding_gate.world_object_id.value != WORLD_OBJECT_A.value) {
        return Fail("model node binding gate world object mismatch");
    }

    if (model_binding_gate.component_type_id.value != COMPONENT_TYPE_MESH.value) {
        return Fail("model node binding gate component type mismatch");
    }

    if (model_binding_gate.component_slot_id.value != COMPONENT_SLOT_MESH.value) {
        return Fail("model node binding gate component slot mismatch");
    }

    if (!ResourceHandlesMatch(model_binding_gate.resource_handle, texture_handle)) {
        return Fail("model node binding gate resource handle mismatch");
    }

    if (model_binding_gate.expected_resource_type.value != RESOURCE_TYPE_TEXTURE.value) {
        return Fail("model node binding gate resource type mismatch");
    }

    const WorldSceneActiveRestoreGateRecord &skeleton_binding_gate = fixture.gate_outputs[7U];
    if (skeleton_binding_gate.family != WorldSceneApplyTimeRestoreProofFamily::Binding) {
        return Fail("skeleton joint binding gate family mismatch");
    }

    if (skeleton_binding_gate.input_index != 1U) {
        return Fail("skeleton joint binding gate input mismatch");
    }

    if (skeleton_binding_gate.world_object_id.value != WORLD_OBJECT_B.value) {
        return Fail("skeleton joint binding gate world object mismatch");
    }

    if (skeleton_binding_gate.component_type_id.value != COMPONENT_TYPE_MATERIAL.value) {
        return Fail("skeleton joint binding gate component type mismatch");
    }

    if (skeleton_binding_gate.component_slot_id.value != COMPONENT_SLOT_MATERIAL.value) {
        return Fail("skeleton joint binding gate component slot mismatch");
    }

    if (!ResourceHandlesMatch(skeleton_binding_gate.resource_handle, material_handle)) {
        return Fail("skeleton joint binding gate resource handle mismatch");
    }

    if (skeleton_binding_gate.expected_resource_type.value != RESOURCE_TYPE_MATERIAL.value) {
        return Fail("skeleton joint binding gate resource type mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.emitted_gate_record_count != SCRATCH_RECORD_COUNT) {
        return Fail("target family sidecar snapshot gate count mismatch");
    }

    return 0;
}

int TestApplySidecarAssemblyRestore() {
    HandoffFixture fixture = MakeFixture();
    fixture.mappings[0U].target_kind = RuntimeAssetTargetIdentityKind::ModelNode;
    fixture.mappings[1U].target_kind = RuntimeAssetTargetIdentityKind::SkeletonJoint;

    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    ResourceHandle texture_handle{};
    ResourceHandle material_handle{};
    if (PrepareSidecarInputs(&fixture, &resource_registry, &texture_handle, &material_handle) != 0) {
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
        &binding_destination);
    handoff_request.input_attachment_count = SIDECAR_RECORD_COUNT;
    handoff_request.input_binding_count = SIDECAR_RECORD_COUNT;

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("sidecar assembly handoff apply failed");
    }

    if (result.state.restored_attachment_count != SIDECAR_RECORD_COUNT) {
        return Fail("sidecar assembly restored attachment count mismatch");
    }

    if (result.state.restored_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("sidecar assembly restored binding count mismatch");
    }

    const auto attachment_snapshot = attachment_destination.Snapshot();
    if (attachment_snapshot.active_attachment_count != SIDECAR_RECORD_COUNT) {
        return Fail("sidecar assembly attachment destination count mismatch");
    }

    const auto binding_snapshot = binding_destination.Snapshot();
    if (binding_snapshot.active_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("sidecar assembly binding destination count mismatch");
    }

    if (binding_snapshot.acquired_binding_count != SIDECAR_RECORD_COUNT) {
        return Fail("sidecar assembly binding acquire count mismatch");
    }

    if (VerifyRestoredSidecarDestinations(
            &attachment_destination,
            &binding_destination,
            texture_handle,
            material_handle) != 0) {
        return 1;
    }

    if (result.state.restored_identity_count != OUTPUT_RECORD_COUNT) {
        return Fail("sidecar assembly restored identity count mismatch");
    }

    if (result.state.restored_transform_count != OUTPUT_RECORD_COUNT) {
        return Fail("sidecar assembly restored transform count mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.accepted_handoff_count != 1U) {
        return Fail("sidecar assembly snapshot accepted count mismatch");
    }

    if (snapshot.emitted_gate_record_count != SCRATCH_RECORD_COUNT) {
        return Fail("sidecar assembly snapshot gate count mismatch");
    }

    return 0;
}

int TestRejectSidecarAssemblyFailureStatus() {
    HandoffFixture fixture = MakeFixture();
    fixture.mappings[0U].target_kind = RuntimeAssetTargetIdentityKind::ModelNode;
    fixture.mappings[1U].target_kind = RuntimeAssetTargetIdentityKind::SkeletonJoint;

    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    ResourceHandle texture_handle{};
    ResourceHandle material_handle{};
    if (PrepareSidecarInputs(&fixture, &resource_registry, &texture_handle, &material_handle) != 0) {
        return 1;
    }

    WorldObjectIdentityBridge identity_destination(world, object_registry);
    WorldTransformBridge transform_destination(world);
    WorldComponentAttachmentBridge attachment_destination{};
    WorldComponentResourceBindingBridge binding_destination{};
    const WorldObjectIdentitySnapshot identity_before = identity_destination.Snapshot();
    const WorldTransformSnapshot transform_before = transform_destination.Snapshot();
    const auto attachment_before = attachment_destination.Snapshot();
    const auto binding_before = binding_destination.Snapshot();

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
        &binding_destination);
    handoff_request.input_attachment_count = SIDECAR_RECORD_COUNT;
    handoff_request.input_binding_count = SIDECAR_RECORD_COUNT;

    WorldSceneAssemblyBridgeDesc assembly_desc{};
    assembly_desc.attachment_capacity = 0U;
    RuntimeAssetWorldObjectRestoreHandoffBridge bridge(assembly_desc);
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (result.status != RuntimeAssetWorldObjectRestoreHandoffStatus::RestoreFailed) {
        return Fail("sidecar assembly failure returned wrong handoff status");
    }

    if (result.assembly_status != WorldSceneAssemblyStatus::InvalidBridgeCapacity) {
        return Fail("sidecar assembly failure did not expose assembly status");
    }

    if (result.state.assembly_status != WorldSceneAssemblyStatus::InvalidBridgeCapacity) {
        return Fail("sidecar assembly failure did not write state assembly status");
    }

    if (result.restore_status != WorldSceneObjectTransformRestoreStatus::Success) {
        return Fail("sidecar assembly failure wrote wrong transform restore status");
    }

    if (result.gate_status != WorldSceneActiveRestoreGateStatus::Success) {
        return Fail("sidecar assembly failure did not pass active gate first");
    }

    if (!ObjectIdentitySnapshotsMatch(identity_before, identity_destination.Snapshot())) {
        return Fail("sidecar assembly failure mutated identity destination");
    }

    if (!TransformSnapshotsMatch(transform_before, transform_destination.Snapshot())) {
        return Fail("sidecar assembly failure mutated transform destination");
    }

    const auto attachment_after = attachment_destination.Snapshot();
    if (attachment_after.active_attachment_count != attachment_before.active_attachment_count) {
        return Fail("sidecar assembly failure mutated attachment destination");
    }

    if (attachment_after.last_status != attachment_before.last_status) {
        return Fail("sidecar assembly failure touched attachment status");
    }

    const auto binding_after = binding_destination.Snapshot();
    if (binding_after.active_binding_count != binding_before.active_binding_count) {
        return Fail("sidecar assembly failure mutated binding destination");
    }

    if (binding_after.acquired_binding_count != binding_before.acquired_binding_count) {
        return Fail("sidecar assembly failure acquired binding resource");
    }

    if (binding_after.last_status != binding_before.last_status) {
        return Fail("sidecar assembly failure touched binding status");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.failed_operation_count != 1U) {
        return Fail("sidecar assembly failure snapshot failure count mismatch");
    }

    if (snapshot.rejected_operation_count != 0U) {
        return Fail("sidecar assembly failure snapshot rejection mismatch");
    }

    if (snapshot.last_status != RuntimeAssetWorldObjectRestoreHandoffStatus::RestoreFailed) {
        return Fail("sidecar assembly failure snapshot handoff status mismatch");
    }

    if (snapshot.last_assembly_status != WorldSceneAssemblyStatus::InvalidBridgeCapacity) {
        return Fail("sidecar assembly failure snapshot assembly status mismatch");
    }

    if (snapshot.last_restore_status != WorldSceneObjectTransformRestoreStatus::Success) {
        return Fail("sidecar assembly failure snapshot restore status mismatch");
    }

    return 0;
}

int TestRejectAdapterPreflight() {
    HandoffFixture fixture = MakeFixture();
    fixture.mappings[0U].target_kind = RuntimeAssetTargetIdentityKind::Unknown;
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    WorldObjectIdentityBridge identity_destination(world, object_registry);
    WorldTransformBridge transform_destination(world);
    WorldComponentAttachmentBridge attachment_destination{};
    WorldComponentResourceBindingBridge binding_destination{};
    const WorldObjectIdentitySnapshot identity_before = identity_destination.Snapshot();
    const WorldTransformSnapshot transform_before = transform_destination.Snapshot();

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
        &binding_destination);

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (result.status != RuntimeAssetWorldObjectRestoreHandoffStatus::AdapterBuildFailed) {
        return Fail("handoff adapter preflight returned wrong status");
    }

    if (result.adapter_status != RuntimeAssetWorldObjectAdapterStatus::UnsupportedTargetKind) {
        return Fail("handoff adapter preflight returned wrong adapter status");
    }

    if (!OutputRecordsHaveSentinelValues(fixture)) {
        return Fail("handoff adapter preflight mutated output records");
    }

    if (!ObjectIdentitySnapshotsMatch(identity_before, identity_destination.Snapshot())) {
        return Fail("handoff adapter preflight mutated identity destination");
    }

    if (!TransformSnapshotsMatch(transform_before, transform_destination.Snapshot())) {
        return Fail("handoff adapter preflight mutated transform destination");
    }

    return 0;
}

int TestRejectWorldGate() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, false) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    WorldObjectIdentityBridge identity_destination(world, object_registry);
    WorldTransformBridge transform_destination(world);
    WorldComponentAttachmentBridge attachment_destination{};
    WorldComponentResourceBindingBridge binding_destination{};
    const WorldObjectIdentitySnapshot identity_before = identity_destination.Snapshot();
    const WorldTransformSnapshot transform_before = transform_destination.Snapshot();

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
        &binding_destination);

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (result.status != RuntimeAssetWorldObjectRestoreHandoffStatus::GateFailed) {
        return Fail("handoff world gate returned wrong status");
    }

    if (result.gate_status == WorldSceneActiveRestoreGateStatus::Success) {
        return Fail("handoff world gate did not expose gate failure");
    }

    if (!ObjectIdentitySnapshotsMatch(identity_before, identity_destination.Snapshot())) {
        return Fail("handoff world gate mutated identity destination");
    }

    if (!TransformSnapshotsMatch(transform_before, transform_destination.Snapshot())) {
        return Fail("handoff world gate mutated transform destination");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.rejected_operation_count != 1U) {
        return Fail("handoff world gate snapshot rejection mismatch");
    }

    return 0;
}

int TestRejectNullAdapterRequest() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    WorldObjectIdentityBridge identity_destination(world, object_registry);
    WorldTransformBridge transform_destination(world);
    WorldComponentAttachmentBridge attachment_destination{};
    WorldComponentResourceBindingBridge binding_destination{};
    const WorldObjectIdentitySnapshot identity_before = identity_destination.Snapshot();
    const WorldTransformSnapshot transform_before = transform_destination.Snapshot();

    RuntimeAssetWorldObjectRestoreHandoffRequest handoff_request = MakeHandoffRequest(
        &fixture,
        nullptr,
        &world,
        &object_registry,
        &resource_registry,
        &identity_destination,
        &transform_destination,
        &attachment_destination,
        &binding_destination);

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (result.status != RuntimeAssetWorldObjectRestoreHandoffStatus::InvalidAdapterRequest) {
        return Fail("handoff null adapter returned wrong status");
    }

    if (!ObjectIdentitySnapshotsMatch(identity_before, identity_destination.Snapshot())) {
        return Fail("handoff null adapter mutated identity destination");
    }

    if (!TransformSnapshotsMatch(transform_before, transform_destination.Snapshot())) {
        return Fail("handoff null adapter mutated transform destination");
    }

    return 0;
}

int RunTest(std::string_view test_name) {
    constexpr std::array<TestCase, 8U> TESTS{{
        {TEST_APPLY_RESTORE, TestApplyRestore},
        {TEST_APPLY_TARGET_FAMILY_ALIASES, TestApplyTargetFamilyAliases},
        {TEST_APPLY_ATTACHMENT_BINDING_GATE_RECORDS, TestApplyAttachmentAndBindingGateRecords},
        {TEST_APPLY_SIDECAR_ASSEMBLY_RESTORE, TestApplySidecarAssemblyRestore},
        {TEST_REJECT_SIDECAR_ASSEMBLY_FAILURE_STATUS, TestRejectSidecarAssemblyFailureStatus},
        {TEST_REJECT_ADAPTER_PREFLIGHT, TestRejectAdapterPreflight},
        {TEST_REJECT_WORLD_GATE, TestRejectWorldGate},
        {TEST_REJECT_NULL_ADAPTER, TestRejectNullAdapterRequest},
    }};

    for (const TestCase &test : TESTS) {
        if (test.name == test_name) {
            return test.function();
        }
    }

    if (RuntimeAssetWorldObjectDataHandoffFixtureTestNameMatches(test_name)) {
        return RunRuntimeAssetWorldObjectDataHandoffFixtureTest(test_name);
    }

    return Fail("unknown test name");
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail("expected exactly one test name");
    }

    return RunTest(argv[1]);
}
