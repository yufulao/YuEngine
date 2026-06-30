// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectDataHandoffFixtureTest.cpp

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
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterIdentityRecord.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridge.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffResult.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffSnapshot.h"
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
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofRecord.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofSliceRecord.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformSnapshot.h"
#include "YuEngine/World/WorldTransformState.h"

using yuengine::object::ObjectDescriptor;
using yuengine::object::ObjectHandle;
using yuengine::object::ObjectRegistrationResult;
using yuengine::object::ObjectRegistry;
using yuengine::object::ObjectRegistryDesc;
using yuengine::object::ObjectTypeId;
using yuengine::resource::ResourceRegistry;
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
using yuengine::world::WorldComponentAttachmentBridge;
using yuengine::world::WorldComponentAttachmentSnapshotRecord;
using yuengine::world::WorldComponentResourceBindingBridge;
using yuengine::world::WorldComponentResourceBindingSnapshotRecord;
using yuengine::world::WorldDesc;
using yuengine::world::WorldInstance;
using yuengine::world::WorldObjectDesc;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldObjectIdentityBridge;
using yuengine::world::WorldObjectIdentitySnapshot;
using yuengine::world::WorldRegistrationResult;
using yuengine::world::WorldSceneActiveRestoreGateRecord;
using yuengine::world::WorldSceneApplyTimeRestoreProofRecord;
using yuengine::world::WorldSceneApplyTimeRestoreProofSliceRecord;
using yuengine::world::WorldSceneDecodedRestorePlanRecord;
using yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;
using yuengine::world::WorldTransformBridge;
using yuengine::world::WorldTransformResult;
using yuengine::world::WorldTransformSnapshot;
using yuengine::world::WorldTransformState;

namespace {
constexpr std::uint32_t DATA_RECORD_COUNT = 3U;
constexpr std::uint32_t SCRATCH_RECORD_COUNT = 8U;
constexpr std::uint32_t EMPTY_SIDECAR_COUNT = 1U;
constexpr const char *TEST_FEED_RUNTIME_ASSET_DATA =
    "RuntimeAssetWorldObjectDataHandoff_FeedsRuntimeAssetDataOutputsIntoRestoreHandoff";
constexpr WorldObjectId WORLD_OBJECT_SCENE{11U};
constexpr WorldObjectId WORLD_OBJECT_MODEL{12U};
constexpr WorldObjectId WORLD_OBJECT_SKELETON{13U};
constexpr WorldObjectId SENTINEL_WORLD_OBJECT{99U};
constexpr ObjectHandle SENTINEL_OBJECT_HANDLE{9U, 9U};
constexpr ObjectTypeId OBJECT_TYPE_SCENE{1U};
constexpr ObjectTypeId OBJECT_TYPE_MODEL{2U};
constexpr ObjectTypeId OBJECT_TYPE_SKELETON{3U};
constexpr std::uint64_t TARGET_SCENE = 7001U;
constexpr std::uint64_t TARGET_MODEL = 7002U;
constexpr std::uint64_t TARGET_SKELETON = 7003U;
constexpr std::uint32_t ENTITY_SCENE = 201U;
constexpr std::uint32_t ENTITY_MODEL = 202U;
constexpr std::uint32_t ENTITY_SKELETON = 203U;

struct RuntimeAssetDataHandoffFixture final {
    std::array<RuntimeAssetRuntimeInstanceMappingRecord, DATA_RECORD_COUNT> mappings{};
    std::array<RuntimeAssetSceneEntityRecord, DATA_RECORD_COUNT> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, DATA_RECORD_COUNT> scene_transforms{};
    std::array<RuntimeAssetWorldObjectAdapterIdentityRecord, DATA_RECORD_COUNT> identity_records{};
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, DATA_RECORD_COUNT> output_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, DATA_RECORD_COUNT> output_transforms{};
    std::array<WorldComponentAttachmentSnapshotRecord, EMPTY_SIDECAR_COUNT> attachments{};
    std::array<WorldComponentResourceBindingSnapshotRecord, EMPTY_SIDECAR_COUNT> bindings{};
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

RuntimeAssetDataHandoffFixture MakeFixture() {
    RuntimeAssetDataHandoffFixture fixture{};
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
    const WorldRegistrationResult result = world->RegisterObject(object_desc);
    if (!result.Succeeded()) {
        return Fail("world object setup failed");
    }

    return 0;
}

int PrepareCallerOwnedRegistries(
    WorldInstance *world,
    ObjectRegistry *object_registry,
    RuntimeAssetDataHandoffFixture *fixture) {
    if (world == nullptr) {
        return Fail("world fixture missing");
    }

    if (object_registry == nullptr) {
        return Fail("object registry fixture missing");
    }

    if (fixture == nullptr) {
        return Fail("handoff fixture missing");
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
        return Fail("scene object setup failed");
    }

    const ObjectRegistrationResult model_object = CreateObject(object_registry, OBJECT_TYPE_MODEL);
    if (!model_object.Succeeded()) {
        return Fail("model object setup failed");
    }

    const ObjectRegistrationResult skeleton_object = CreateObject(object_registry, OBJECT_TYPE_SKELETON);
    if (!skeleton_object.Succeeded()) {
        return Fail("skeleton object setup failed");
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
    return 0;
}

RuntimeAssetWorldObjectAdapterRequest MakeAdapterRequest(RuntimeAssetDataHandoffFixture *fixture) {
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
    request.output_identities = fixture->output_identities.data();
    request.output_identity_capacity = DATA_RECORD_COUNT;
    request.output_transforms = fixture->output_transforms.data();
    request.output_transform_capacity = DATA_RECORD_COUNT;
    return request;
}

RuntimeAssetWorldObjectRestoreHandoffRequest MakeHandoffRequest(
    RuntimeAssetDataHandoffFixture *fixture,
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

bool ObjectHandlesMatch(ObjectHandle left, ObjectHandle right) {
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

int VerifyAdapterOutputs(const RuntimeAssetDataHandoffFixture &fixture) {
    std::uint32_t index = 0U;
    while (index < DATA_RECORD_COUNT) {
        const WorldSceneObjectTransformRestoreIdentityRecord &identity =
            fixture.output_identities[index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord &expected_identity =
            fixture.identity_records[index];
        if (identity.world_object_id.value != expected_identity.world_object_id.value) {
            return Fail("runtime asset handoff identity world object mismatch");
        }

        if (!ObjectHandlesMatch(identity.object_handle, expected_identity.object_handle)) {
            return Fail("runtime asset handoff identity object handle mismatch");
        }

        const WorldSceneObjectTransformRestoreTransformRecord &transform =
            fixture.output_transforms[index];
        const RuntimeAssetSceneTransformOutputRecord &expected_transform =
            fixture.scene_transforms[index];
        if (transform.world_object_id.value != expected_transform.world_object_id.value) {
            return Fail("runtime asset handoff transform world object mismatch");
        }

        if (!TransformsMatch(transform.transform_state, expected_transform.transform)) {
            return Fail("runtime asset handoff transform state mismatch");
        }

        ++index;
    }

    return 0;
}

int VerifyRestoredTransforms(
    WorldTransformBridge *transform_destination,
    const RuntimeAssetDataHandoffFixture &fixture) {
    if (transform_destination == nullptr) {
        return Fail("runtime asset handoff transform destination missing");
    }

    std::uint32_t index = 0U;
    while (index < DATA_RECORD_COUNT) {
        const RuntimeAssetSceneTransformOutputRecord &expected_transform =
            fixture.scene_transforms[index];
        const WorldTransformResult result =
            transform_destination->Query(expected_transform.world_object_id);
        if (!result.Succeeded()) {
            return Fail("runtime asset handoff restored transform query failed");
        }

        if (!TransformsMatch(result.transform_state, expected_transform.transform)) {
            return Fail("runtime asset handoff restored transform mismatch");
        }

        ++index;
    }

    return 0;
}

int TestFeedRuntimeAssetDataOutputs() {
    RuntimeAssetDataHandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareCallerOwnedRegistries(&world, &object_registry, &fixture) != 0) {
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
        return Fail("runtime asset data handoff failed");
    }

    if (result.state.input_mapping_count != DATA_RECORD_COUNT) {
        return Fail("runtime asset data handoff input mapping count mismatch");
    }

    if (result.state.output_identity_count != DATA_RECORD_COUNT) {
        return Fail("runtime asset data handoff identity count mismatch");
    }

    if (result.state.output_transform_count != DATA_RECORD_COUNT) {
        return Fail("runtime asset data handoff transform count mismatch");
    }

    if (result.state.gate_record_count != DATA_RECORD_COUNT + DATA_RECORD_COUNT) {
        return Fail("runtime asset data handoff gate count mismatch");
    }

    if (result.state.restored_identity_count != DATA_RECORD_COUNT) {
        return Fail("runtime asset data handoff restored identity count mismatch");
    }

    if (result.state.restored_transform_count != DATA_RECORD_COUNT) {
        return Fail("runtime asset data handoff restored transform count mismatch");
    }

    if (VerifyAdapterOutputs(fixture) != 0) {
        return 1;
    }

    const WorldObjectIdentitySnapshot identity_snapshot = identity_destination.Snapshot();
    if (identity_snapshot.binding_count != DATA_RECORD_COUNT) {
        return Fail("runtime asset data handoff identity destination count mismatch");
    }

    const WorldTransformSnapshot transform_snapshot = transform_destination.Snapshot();
    if (transform_snapshot.record_count != DATA_RECORD_COUNT) {
        return Fail("runtime asset data handoff transform destination count mismatch");
    }

    if (VerifyRestoredTransforms(&transform_destination, fixture) != 0) {
        return 1;
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.accepted_handoff_count != 1U) {
        return Fail("runtime asset data handoff snapshot accepted count mismatch");
    }

    if (snapshot.restored_identity_count != DATA_RECORD_COUNT) {
        return Fail("runtime asset data handoff snapshot identity count mismatch");
    }

    if (snapshot.restored_transform_count != DATA_RECORD_COUNT) {
        return Fail("runtime asset data handoff snapshot transform count mismatch");
    }

    return 0;
}

}

bool RuntimeAssetWorldObjectDataHandoffFixtureTestNameMatches(std::string_view test_name) {
    return test_name == TEST_FEED_RUNTIME_ASSET_DATA;
}

int RunRuntimeAssetWorldObjectDataHandoffFixtureTest(std::string_view test_name) {
    if (RuntimeAssetWorldObjectDataHandoffFixtureTestNameMatches(test_name)) {
        return TestFeedRuntimeAssetDataOutputs();
    }

    return Fail("unknown data handoff test name");
}
