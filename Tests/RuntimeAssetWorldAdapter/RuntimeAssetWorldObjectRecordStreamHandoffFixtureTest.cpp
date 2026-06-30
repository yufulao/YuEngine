// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRecordStreamHandoffFixtureTest.cpp

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
using yuengine::serialize::MAX_STREAM_BYTE_COUNT;
using yuengine::serialize::SerializeReader;
using yuengine::serialize::SerializeWriter;
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
using yuengine::world::WorldSceneActiveRestoreGateStatus;
using yuengine::world::WorldSceneApplyTimeRestoreProofFamily;
using yuengine::world::WorldSceneApplyTimeRestoreProofRecord;
using yuengine::world::WorldSceneApplyTimeRestoreProofSliceRecord;
using yuengine::world::WorldSceneApplyTimeRestoreProofStatus;
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
constexpr const char *TEST_FEED_WORLD_SCENE_RECORD_STREAMS =
    "RuntimeAssetWorldObjectRecordStreamHandoff_FeedsWorldSceneRecordStreamsIntoRestoreHandoff";
constexpr WorldObjectId WORLD_OBJECT_SCENE{21U};
constexpr WorldObjectId WORLD_OBJECT_MODEL{22U};
constexpr WorldObjectId WORLD_OBJECT_SKELETON{23U};
constexpr WorldObjectId SENTINEL_WORLD_OBJECT{99U};
constexpr ObjectHandle SENTINEL_OBJECT_HANDLE{9U, 9U};
constexpr ObjectTypeId OBJECT_TYPE_SCENE{1U};
constexpr ObjectTypeId OBJECT_TYPE_MODEL{2U};
constexpr ObjectTypeId OBJECT_TYPE_SKELETON{3U};
constexpr std::uint64_t TARGET_SCENE = 8001U;
constexpr std::uint64_t TARGET_MODEL = 8002U;
constexpr std::uint64_t TARGET_SKELETON = 8003U;
constexpr std::uint32_t ENTITY_SCENE = 301U;
constexpr std::uint32_t ENTITY_MODEL = 302U;
constexpr std::uint32_t ENTITY_SKELETON = 303U;

using SerializeBuffer = std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT>;

struct RecordStreamHandoffFixture final {
    std::array<RuntimeAssetRuntimeInstanceMappingRecord, DATA_RECORD_COUNT> mappings{};
    std::array<RuntimeAssetSceneEntityRecord, DATA_RECORD_COUNT> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, DATA_RECORD_COUNT> scene_transforms{};
    std::array<RuntimeAssetWorldObjectAdapterIdentityRecord, DATA_RECORD_COUNT> identity_records{};
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, DATA_RECORD_COUNT> stream_input_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, DATA_RECORD_COUNT> stream_input_transforms{};
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

RecordStreamHandoffFixture MakeFixture() {
    RecordStreamHandoffFixture fixture{};
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
        return Fail("record stream world object setup failed");
    }

    return 0;
}

int PrepareCallerOwnedRegistries(
    WorldInstance *world,
    ObjectRegistry *object_registry,
    RecordStreamHandoffFixture *fixture) {
    if (world == nullptr) {
        return Fail("record stream world fixture missing");
    }

    if (object_registry == nullptr) {
        return Fail("record stream object registry fixture missing");
    }

    if (fixture == nullptr) {
        return Fail("record stream fixture missing");
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
        return Fail("record stream scene object setup failed");
    }

    const ObjectRegistrationResult model_object = CreateObject(object_registry, OBJECT_TYPE_MODEL);
    if (!model_object.Succeeded()) {
        return Fail("record stream model object setup failed");
    }

    const ObjectRegistrationResult skeleton_object = CreateObject(object_registry, OBJECT_TYPE_SKELETON);
    if (!skeleton_object.Succeeded()) {
        return Fail("record stream skeleton object setup failed");
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
        fixture->stream_input_identities[index] = RestoreIdentity(
            identity.world_object_id,
            identity.object_handle);
        fixture->stream_input_transforms[index] = RestoreTransform(
            transform.world_object_id,
            transform.transform);
        ++index;
    }

    return 0;
}

int RoundTripWorldSceneRecordStream(RecordStreamHandoffFixture *fixture) {
    if (fixture == nullptr) {
        return Fail("record stream roundtrip fixture missing");
    }

    WorldSceneRecordValueStreamBridge stream_bridge{};
    const std::uint32_t stream_capacity =
        static_cast<std::uint32_t>(fixture->stream_buffer.size());
    SerializeWriter writer(fixture->stream_buffer.data(), stream_capacity);
    const WorldSceneRecordValueStreamResult write_result = stream_bridge.WriteSceneRecords(
        &writer,
        fixture->stream_input_identities.data(),
        DATA_RECORD_COUNT,
        fixture->stream_input_transforms.data(),
        DATA_RECORD_COUNT,
        fixture->empty_attachments.data(),
        0U,
        fixture->empty_bindings.data(),
        0U);
    if (!write_result.Succeeded()) {
        return Fail("record stream write failed");
    }

    if (write_result.state.committed_byte_count == 0U) {
        return Fail("record stream write byte count missing");
    }

    std::uint32_t identity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t attachment_count = 0U;
    std::uint32_t binding_count = 0U;
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
        &attachment_count,
        fixture->stream_output_bindings.data(),
        EMPTY_SIDECAR_COUNT,
        &binding_count);
    if (!read_result.Succeeded()) {
        return Fail("record stream read failed");
    }

    if (identity_count != DATA_RECORD_COUNT) {
        return Fail("record stream identity count mismatch");
    }

    if (transform_count != DATA_RECORD_COUNT) {
        return Fail("record stream transform count mismatch");
    }

    if (attachment_count != 0U) {
        return Fail("record stream attachment count mismatch");
    }

    if (binding_count != 0U) {
        return Fail("record stream binding count mismatch");
    }

    return 0;
}

RuntimeAssetWorldObjectAdapterRequest MakeAdapterRequest(RecordStreamHandoffFixture *fixture) {
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
    RecordStreamHandoffFixture *fixture,
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
    request.input_attachments = fixture->stream_output_attachments.data();
    request.input_attachment_count = 0U;
    request.input_bindings = fixture->stream_output_bindings.data();
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

int VerifyRecordStreamOutputs(const RecordStreamHandoffFixture &fixture) {
    std::uint32_t index = 0U;
    while (index < DATA_RECORD_COUNT) {
        const WorldSceneObjectTransformRestoreIdentityRecord &stream_identity =
            fixture.stream_output_identities[index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord &expected_identity =
            fixture.identity_records[index];
        if (stream_identity.world_object_id.value != expected_identity.world_object_id.value) {
            return Fail("record stream identity world object mismatch");
        }

        if (!ObjectHandlesMatch(stream_identity.object_handle, expected_identity.object_handle)) {
            return Fail("record stream identity object handle mismatch");
        }

        const WorldSceneObjectTransformRestoreTransformRecord &stream_transform =
            fixture.stream_output_transforms[index];
        const RuntimeAssetSceneTransformOutputRecord &expected_transform =
            fixture.scene_transforms[index];
        if (stream_transform.world_object_id.value != expected_transform.world_object_id.value) {
            return Fail("record stream transform world object mismatch");
        }

        if (!TransformsMatch(stream_transform.transform_state, expected_transform.transform)) {
            return Fail("record stream transform state mismatch");
        }

        ++index;
    }

    return 0;
}

int VerifyIdentityRecordChain(
    const RecordStreamHandoffFixture &fixture,
    std::uint32_t record_index) {
    const WorldSceneDecodedRestorePlanRecord &plan = fixture.plan_scratch[record_index];
    if (plan.family != WorldSceneDecodedRestorePlanRecordFamily::Identity) {
        return Fail("record stream decoded identity family mismatch");
    }

    if (plan.input_index != record_index) {
        return Fail("record stream decoded identity input mismatch");
    }

    const RuntimeAssetWorldObjectAdapterIdentityRecord &expected_identity =
        fixture.identity_records[record_index];
    if (plan.world_object_id.value != expected_identity.world_object_id.value) {
        return Fail("record stream decoded identity world object mismatch");
    }

    if (!ObjectHandlesMatch(plan.object_handle, expected_identity.object_handle)) {
        return Fail("record stream decoded identity object handle mismatch");
    }

    if (plan.projected_object_acquire_count != 1U) {
        return Fail("record stream decoded identity acquire count mismatch");
    }

    if (plan.status != WorldSceneDecodedRestorePlanStatus::Success) {
        return Fail("record stream decoded identity status mismatch");
    }

    const WorldSceneApplyTimeRestoreProofRecord &proof = fixture.proof_scratch[record_index];
    if (proof.family != WorldSceneApplyTimeRestoreProofFamily::Identity) {
        return Fail("record stream proof identity family mismatch");
    }

    if (proof.plan_index != record_index) {
        return Fail("record stream proof identity plan index mismatch");
    }

    if (proof.input_index != record_index) {
        return Fail("record stream proof identity input mismatch");
    }

    if (proof.plan_status != WorldSceneDecodedRestorePlanStatus::Success) {
        return Fail("record stream proof identity plan status mismatch");
    }

    if (proof.status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return Fail("record stream proof identity status mismatch");
    }

    const WorldSceneApplyTimeRestoreProofSliceRecord &slice = fixture.slice_scratch[record_index];
    if (slice.family != WorldSceneApplyTimeRestoreProofFamily::Identity) {
        return Fail("record stream slice identity family mismatch");
    }

    if (slice.plan_index != record_index) {
        return Fail("record stream slice identity plan index mismatch");
    }

    const WorldSceneActiveRestoreGateRecord &gate = fixture.gate_outputs[record_index];
    if (gate.family != WorldSceneApplyTimeRestoreProofFamily::Identity) {
        return Fail("record stream gate identity family mismatch");
    }

    if (gate.gate_index != record_index) {
        return Fail("record stream gate identity index mismatch");
    }

    if (gate.plan_index != record_index) {
        return Fail("record stream gate identity plan index mismatch");
    }

    if (gate.status != WorldSceneActiveRestoreGateStatus::Success) {
        return Fail("record stream gate identity status mismatch");
    }

    return 0;
}

int VerifyTransformRecordChain(
    const RecordStreamHandoffFixture &fixture,
    std::uint32_t transform_index) {
    const std::uint32_t plan_index = DATA_RECORD_COUNT + transform_index;
    const WorldSceneDecodedRestorePlanRecord &plan = fixture.plan_scratch[plan_index];
    if (plan.family != WorldSceneDecodedRestorePlanRecordFamily::Transform) {
        return Fail("record stream decoded transform family mismatch");
    }

    if (plan.input_index != transform_index) {
        return Fail("record stream decoded transform input mismatch");
    }

    const RuntimeAssetSceneTransformOutputRecord &expected_transform =
        fixture.scene_transforms[transform_index];
    if (plan.world_object_id.value != expected_transform.world_object_id.value) {
        return Fail("record stream decoded transform world object mismatch");
    }

    if (plan.status != WorldSceneDecodedRestorePlanStatus::Success) {
        return Fail("record stream decoded transform status mismatch");
    }

    const WorldSceneApplyTimeRestoreProofRecord &proof = fixture.proof_scratch[plan_index];
    if (proof.family != WorldSceneApplyTimeRestoreProofFamily::Transform) {
        return Fail("record stream proof transform family mismatch");
    }

    if (proof.plan_index != plan_index) {
        return Fail("record stream proof transform plan index mismatch");
    }

    if (proof.input_index != transform_index) {
        return Fail("record stream proof transform input mismatch");
    }

    if (proof.status != WorldSceneApplyTimeRestoreProofStatus::Success) {
        return Fail("record stream proof transform status mismatch");
    }

    const WorldSceneApplyTimeRestoreProofSliceRecord &slice = fixture.slice_scratch[plan_index];
    if (slice.family != WorldSceneApplyTimeRestoreProofFamily::Transform) {
        return Fail("record stream slice transform family mismatch");
    }

    if (slice.plan_index != plan_index) {
        return Fail("record stream slice transform plan index mismatch");
    }

    const WorldSceneActiveRestoreGateRecord &gate = fixture.gate_outputs[plan_index];
    if (gate.family != WorldSceneApplyTimeRestoreProofFamily::Transform) {
        return Fail("record stream gate transform family mismatch");
    }

    if (gate.gate_index != plan_index) {
        return Fail("record stream gate transform index mismatch");
    }

    if (gate.input_index != transform_index) {
        return Fail("record stream gate transform input mismatch");
    }

    if (gate.status != WorldSceneActiveRestoreGateStatus::Success) {
        return Fail("record stream gate transform status mismatch");
    }

    return 0;
}

int VerifyDecodedRecordStreams(const RecordStreamHandoffFixture &fixture) {
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

    return 0;
}

int VerifyRestoredTransforms(
    WorldTransformBridge *transform_destination,
    const RecordStreamHandoffFixture &fixture) {
    if (transform_destination == nullptr) {
        return Fail("record stream transform destination missing");
    }

    std::uint32_t index = 0U;
    while (index < DATA_RECORD_COUNT) {
        const RuntimeAssetSceneTransformOutputRecord &expected_transform =
            fixture.scene_transforms[index];
        const WorldTransformResult result =
            transform_destination->Query(expected_transform.world_object_id);
        if (!result.Succeeded()) {
            return Fail("record stream restored transform query failed");
        }

        if (!TransformsMatch(result.transform_state, expected_transform.transform)) {
            return Fail("record stream restored transform mismatch");
        }

        ++index;
    }

    return 0;
}

int TestFeedWorldSceneRecordStreamsIntoRestoreHandoff() {
    RecordStreamHandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareCallerOwnedRegistries(&world, &object_registry, &fixture) != 0) {
        return 1;
    }

    if (RoundTripWorldSceneRecordStream(&fixture) != 0) {
        return 1;
    }

    if (VerifyRecordStreamOutputs(fixture) != 0) {
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
        return Fail("record stream handoff failed");
    }

    if (result.state.input_mapping_count != DATA_RECORD_COUNT) {
        return Fail("record stream handoff input mapping count mismatch");
    }

    if (result.state.output_identity_count != DATA_RECORD_COUNT) {
        return Fail("record stream handoff identity count mismatch");
    }

    if (result.state.output_transform_count != DATA_RECORD_COUNT) {
        return Fail("record stream handoff transform count mismatch");
    }

    if (result.state.proof_record_count != STREAM_PLAN_RECORD_COUNT) {
        return Fail("record stream handoff proof count mismatch");
    }

    if (result.state.slice_record_count != STREAM_PLAN_RECORD_COUNT) {
        return Fail("record stream handoff slice count mismatch");
    }

    if (result.state.gate_record_count != STREAM_PLAN_RECORD_COUNT) {
        return Fail("record stream handoff gate count mismatch");
    }

    if (result.state.restored_identity_count != DATA_RECORD_COUNT) {
        return Fail("record stream handoff restored identity count mismatch");
    }

    if (result.state.restored_transform_count != DATA_RECORD_COUNT) {
        return Fail("record stream handoff restored transform count mismatch");
    }

    if (VerifyRecordStreamOutputs(fixture) != 0) {
        return 1;
    }

    if (VerifyDecodedRecordStreams(fixture) != 0) {
        return 1;
    }

    const WorldObjectIdentitySnapshot identity_snapshot = identity_destination.Snapshot();
    if (identity_snapshot.binding_count != DATA_RECORD_COUNT) {
        return Fail("record stream identity destination count mismatch");
    }

    const WorldTransformSnapshot transform_snapshot = transform_destination.Snapshot();
    if (transform_snapshot.record_count != DATA_RECORD_COUNT) {
        return Fail("record stream transform destination count mismatch");
    }

    if (VerifyRestoredTransforms(&transform_destination, fixture) != 0) {
        return 1;
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.accepted_handoff_count != 1U) {
        return Fail("record stream snapshot accepted count mismatch");
    }

    if (snapshot.emitted_gate_record_count != STREAM_PLAN_RECORD_COUNT) {
        return Fail("record stream snapshot gate count mismatch");
    }

    if (snapshot.restored_identity_count != DATA_RECORD_COUNT) {
        return Fail("record stream snapshot identity count mismatch");
    }

    if (snapshot.restored_transform_count != DATA_RECORD_COUNT) {
        return Fail("record stream snapshot transform count mismatch");
    }

    return 0;
}

}

bool RuntimeAssetWorldObjectRecordStreamHandoffFixtureTestNameMatches(std::string_view test_name) {
    return test_name == TEST_FEED_WORLD_SCENE_RECORD_STREAMS;
}

int RunRuntimeAssetWorldObjectRecordStreamHandoffFixtureTest(std::string_view test_name) {
    if (RuntimeAssetWorldObjectRecordStreamHandoffFixtureTestNameMatches(test_name)) {
        return TestFeedWorldSceneRecordStreamsIntoRestoreHandoff();
    }

    return Fail("unknown record stream handoff test name");
}
