// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterBridgeTest.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/Object/ObjectDescriptor.h"
#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Object/ObjectRegistrationResult.h"
#include "YuEngine/Object/ObjectRegistry.h"
#include "YuEngine/Object/ObjectSnapshot.h"
#include "YuEngine/Object/ObjectTypeId.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterBridge.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterIdentityRecord.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterResult.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterSnapshot.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterStatus.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectTransformApplicationRequest.h"
#include "YuEngine/World/WorldDesc.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectDesc.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldRegistrationResult.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldSnapshot.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformBridgeDesc.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldTransformStatus.h"

using yuengine::animation::AnimationRuntimeChannel;
using yuengine::animation::AnimationRuntimeSampledValue;
using yuengine::object::ObjectDescriptor;
using yuengine::object::ObjectHandle;
using yuengine::object::ObjectRegistrationResult;
using yuengine::object::ObjectRegistry;
using yuengine::object::ObjectSnapshot;
using yuengine::object::ObjectTypeId;
using yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord;
using yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord;
using yuengine::runtimeasset::RuntimeAssetTargetIdentityKind;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterBridge;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterIdentityRecord;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterResult;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterSnapshot;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterStatus;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectTransformApplicationRequest;
using yuengine::world::WorldDesc;
using yuengine::world::WorldInstance;
using yuengine::world::WorldObjectDesc;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldRegistrationResult;
using yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;
using yuengine::world::WorldSnapshot;
using yuengine::world::WorldTransformBridge;
using yuengine::world::WorldTransformBridgeDesc;
using yuengine::world::WorldTransformResult;
using yuengine::world::WorldTransformState;
using yuengine::world::WorldTransformStatus;

namespace {
constexpr std::uint32_t OUTPUT_RECORD_COUNT = 2U;
constexpr const char *TEST_BUILD_RESTORE_RECORDS =
    "RuntimeAssetWorldObjectAdapter_BuildsRestoreRecordsFromRuntimeInstanceMapping";
constexpr const char *TEST_BUILD_TARGET_FAMILY_ALIAS_RESTORE_RECORDS =
    "RuntimeAssetWorldObjectAdapter_BuildsRestoreRecordsForModelAndSkeletonTargetFamilyAliases";
constexpr const char *TEST_REJECT_MISSING_MAPPING =
    "RuntimeAssetWorldObjectAdapter_RejectsMissingRuntimeInstanceMappingWithoutMutation";
constexpr const char *TEST_REJECT_TARGET_KIND =
    "RuntimeAssetWorldObjectAdapter_RejectsInvalidTargetKindWithoutMutation";
constexpr const char *TEST_REJECT_SCENE_ENTITY_INDEX =
    "RuntimeAssetWorldObjectAdapter_RejectsSceneEntityIndexOverflowWithoutMutation";
constexpr const char *TEST_REJECT_WORLD_OBJECT =
    "RuntimeAssetWorldObjectAdapter_RejectsInvalidWorldObjectIdWithoutMutation";
constexpr const char *TEST_REJECT_OBJECT_HANDLE =
    "RuntimeAssetWorldObjectAdapter_RejectsInvalidObjectHandleWithoutMutation";
constexpr const char *TEST_REJECT_DUPLICATE_TARGET =
    "RuntimeAssetWorldObjectAdapter_RejectsDuplicateTargetIdWithoutMutation";
constexpr const char *TEST_REJECT_DUPLICATE_WORLD_OBJECT =
    "RuntimeAssetWorldObjectAdapter_RejectsDuplicateWorldObjectIdWithoutMutation";
constexpr const char *TEST_REJECT_DUPLICATE_OBJECT_HANDLE =
    "RuntimeAssetWorldObjectAdapter_RejectsDuplicateObjectHandleWithoutMutation";
constexpr const char *TEST_REJECT_SMALL_IDENTITY_OUTPUT =
    "RuntimeAssetWorldObjectAdapter_RejectsSmallIdentityOutputWithoutMutation";
constexpr const char *TEST_REJECT_SMALL_TRANSFORM_OUTPUT =
    "RuntimeAssetWorldObjectAdapter_RejectsSmallTransformOutputWithoutMutation";
constexpr const char *TEST_NO_WORLD_OBJECT_REGISTRY_MUTATION =
    "RuntimeAssetWorldObjectAdapter_DoesNotMutateWorldOrObjectRegistryDuringBuild";
constexpr const char *TEST_APPLY_SAMPLED_TRANSFORM =
    "RuntimeAssetWorldObjectAdapter_AppliesRuntimeAssetSampledTransformToCallerOwnedWorldObject";
constexpr const char *TEST_REJECT_UNMAPPED_SAMPLED_TRANSFORM =
    "RuntimeAssetWorldObjectAdapter_RejectsUnmappedSampledTransformWithoutMutation";
constexpr WorldObjectId WORLD_OBJECT_A{1U};
constexpr WorldObjectId WORLD_OBJECT_B{2U};
constexpr WorldObjectId WORLD_OBJECT_UNMAPPED{77U};
constexpr WorldObjectId SENTINEL_WORLD_OBJECT{99U};
constexpr ObjectHandle OBJECT_HANDLE_A{1U, 1U};
constexpr ObjectHandle OBJECT_HANDLE_B{2U, 1U};
constexpr ObjectHandle SENTINEL_OBJECT_HANDLE{9U, 9U};
constexpr ObjectTypeId OBJECT_TYPE_PLAYER{1U};
constexpr std::uint64_t TARGET_A = 1001U;
constexpr std::uint64_t TARGET_B = 1002U;
constexpr std::uint32_t ENTITY_A = 11U;
constexpr std::uint32_t ENTITY_B = 12U;

using TestFunction = int (*)();

struct TestCase final {
    std::string_view name;
    TestFunction function = nullptr;
};

struct AdapterFixture final {
    std::array<RuntimeAssetRuntimeInstanceMappingRecord, OUTPUT_RECORD_COUNT> mappings{};
    std::array<RuntimeAssetSceneEntityRecord, OUTPUT_RECORD_COUNT> scene_entities{};
    std::array<RuntimeAssetSceneTransformOutputRecord, OUTPUT_RECORD_COUNT> scene_transforms{};
    std::array<RuntimeAssetWorldObjectAdapterIdentityRecord, OUTPUT_RECORD_COUNT> identity_records{};
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, OUTPUT_RECORD_COUNT> output_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, OUTPUT_RECORD_COUNT> output_transforms{};

    RuntimeAssetWorldObjectAdapterRequest Request() {
        RuntimeAssetWorldObjectAdapterRequest request{};
        request.runtime_instance_mappings = mappings.data();
        request.runtime_instance_mapping_count = OUTPUT_RECORD_COUNT;
        request.scene_entities = scene_entities.data();
        request.scene_entity_count = OUTPUT_RECORD_COUNT;
        request.scene_transforms = scene_transforms.data();
        request.scene_transform_count = OUTPUT_RECORD_COUNT;
        request.identity_records = identity_records.data();
        request.identity_record_count = OUTPUT_RECORD_COUNT;
        request.output_identities = output_identities.data();
        request.output_identity_capacity = OUTPUT_RECORD_COUNT;
        request.output_transforms = output_transforms.data();
        request.output_transform_capacity = OUTPUT_RECORD_COUNT;
        return request;
    }
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

AdapterFixture MakeFixture() {
    AdapterFixture fixture{};
    fixture.mappings[0U] = Mapping(TARGET_A, ENTITY_A, 0U, 0U);
    fixture.mappings[1U] = Mapping(TARGET_B, ENTITY_B, 1U, 1U);
    fixture.scene_entities[0U] = Entity(ENTITY_A, WORLD_OBJECT_A, Transform(10.0F));
    fixture.scene_entities[1U] = Entity(ENTITY_B, WORLD_OBJECT_B, Transform(20.0F));
    fixture.scene_transforms[0U] = TransformOutput(WORLD_OBJECT_A, Transform(30.0F));
    fixture.scene_transforms[1U] = TransformOutput(WORLD_OBJECT_B, Transform(40.0F));
    fixture.identity_records[0U] = Identity(TARGET_A, WORLD_OBJECT_A, OBJECT_HANDLE_A);
    fixture.identity_records[1U] = Identity(TARGET_B, WORLD_OBJECT_B, OBJECT_HANDLE_B);

    std::uint32_t index = 0U;
    while (index < OUTPUT_RECORD_COUNT) {
        fixture.output_identities[index] = SentinelIdentityOutput();
        fixture.output_transforms[index] = SentinelTransformOutput();
        ++index;
    }

    return fixture;
}

RuntimeAssetWorldObjectTransformApplicationRequest MakeTransformApplicationRequest(
    AdapterFixture *fixture,
    WorldTransformBridge *transform_destination,
    const AnimationRuntimeSampledValue *sampled_values,
    std::uint32_t sampled_value_count) {
    RuntimeAssetWorldObjectTransformApplicationRequest request{};
    if (fixture == nullptr) {
        return request;
    }

    request.runtime_instance_mappings = fixture->mappings.data();
    request.runtime_instance_mapping_count = OUTPUT_RECORD_COUNT;
    request.identity_records = fixture->identity_records.data();
    request.identity_record_count = OUTPUT_RECORD_COUNT;
    request.transform_destination = transform_destination;
    request.sampled_values = sampled_values;
    request.sampled_value_count = sampled_value_count;
    return request;
}

int RegisterWorldObject(WorldInstance *world, WorldObjectId world_object_id) {
    if (world == nullptr) {
        return Fail("world fixture missing");
    }

    WorldObjectDesc object_desc{};
    object_desc.id = world_object_id;
    object_desc.is_enabled = true;
    const WorldRegistrationResult result = world->RegisterObject(object_desc);
    if (!result.Succeeded()) {
        return Fail("world object setup failed");
    }

    return 0;
}

int RegisterTransform(
    WorldTransformBridge *transform_destination,
    WorldObjectId world_object_id,
    WorldTransformState transform) {
    if (transform_destination == nullptr) {
        return Fail("transform destination missing");
    }

    const WorldTransformResult result = transform_destination->Register(world_object_id, transform);
    if (result.status != WorldTransformStatus::Success) {
        return Fail("transform setup failed");
    }

    return 0;
}

bool ObjectHandlesMatch(ObjectHandle left, ObjectHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool IdentityOutputsMatch(
    const WorldSceneObjectTransformRestoreIdentityRecord &left,
    const WorldSceneObjectTransformRestoreIdentityRecord &right) {
    if (left.world_object_id.value != right.world_object_id.value) {
        return false;
    }

    return ObjectHandlesMatch(left.object_handle, right.object_handle);
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

bool TransformOutputsMatch(
    const WorldSceneObjectTransformRestoreTransformRecord &left,
    const WorldSceneObjectTransformRestoreTransformRecord &right) {
    if (left.world_object_id.value != right.world_object_id.value) {
        return false;
    }

    return TransformsMatch(left.transform_state, right.transform_state);
}

bool OutputIdentitiesMatch(
    const std::array<WorldSceneObjectTransformRestoreIdentityRecord, OUTPUT_RECORD_COUNT> &left,
    const std::array<WorldSceneObjectTransformRestoreIdentityRecord, OUTPUT_RECORD_COUNT> &right) {
    std::uint32_t index = 0U;
    while (index < OUTPUT_RECORD_COUNT) {
        if (!IdentityOutputsMatch(left[index], right[index])) {
            return false;
        }

        ++index;
    }

    return true;
}

bool OutputTransformsMatch(
    const std::array<WorldSceneObjectTransformRestoreTransformRecord, OUTPUT_RECORD_COUNT> &left,
    const std::array<WorldSceneObjectTransformRestoreTransformRecord, OUTPUT_RECORD_COUNT> &right) {
    std::uint32_t index = 0U;
    while (index < OUTPUT_RECORD_COUNT) {
        if (!TransformOutputsMatch(left[index], right[index])) {
            return false;
        }

        ++index;
    }

    return true;
}

bool WorldSnapshotsMatch(const WorldSnapshot &left, const WorldSnapshot &right) {
    if (left.object_capacity != right.object_capacity) {
        return false;
    }

    if (left.phase_trace_capacity != right.phase_trace_capacity) {
        return false;
    }

    if (left.registered_object_count != right.registered_object_count) {
        return false;
    }

    if (left.active_object_count != right.active_object_count) {
        return false;
    }

    if (left.frame_count != right.frame_count) {
        return false;
    }

    if (left.phase_execution_count != right.phase_execution_count) {
        return false;
    }

    if (left.skipped_object_count != right.skipped_object_count) {
        return false;
    }

    if (left.phase_trace_count != right.phase_trace_count) {
        return false;
    }

    if (left.lifecycle_state != right.lifecycle_state) {
        return false;
    }

    return left.last_status == right.last_status;
}

bool ObjectSnapshotsMatch(const ObjectSnapshot &left, const ObjectSnapshot &right) {
    if (left.object_capacity != right.object_capacity) {
        return false;
    }

    if (left.type_capacity != right.type_capacity) {
        return false;
    }

    if (left.type_count != right.type_count) {
        return false;
    }

    if (left.alive_object_count != right.alive_object_count) {
        return false;
    }

    if (left.destroyed_object_count != right.destroyed_object_count) {
        return false;
    }

    if (left.created_object_count != right.created_object_count) {
        return false;
    }

    if (left.referenced_object_count != right.referenced_object_count) {
        return false;
    }

    if (left.released_reference_count != right.released_reference_count) {
        return false;
    }

    if (left.accepted_operation_count != right.accepted_operation_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    return left.last_status == right.last_status;
}

int ExpectFailureWithoutOutputMutation(
    AdapterFixture &fixture,
    RuntimeAssetWorldObjectAdapterRequest request,
    RuntimeAssetWorldObjectAdapterStatus expected_status,
    std::uint32_t expected_required_identity_output_count=0U,
    std::uint32_t expected_required_transform_output_count=0U) {
    const std::array<WorldSceneObjectTransformRestoreIdentityRecord, OUTPUT_RECORD_COUNT> identity_before =
        fixture.output_identities;
    const std::array<WorldSceneObjectTransformRestoreTransformRecord, OUTPUT_RECORD_COUNT> transform_before =
        fixture.output_transforms;
    const std::uint64_t expected_required_identity_output_count_64 =
        static_cast<std::uint64_t>(expected_required_identity_output_count);
    const std::uint64_t expected_required_transform_output_count_64 =
        static_cast<std::uint64_t>(expected_required_transform_output_count);

    RuntimeAssetWorldObjectAdapterBridge bridge{};
    const RuntimeAssetWorldObjectAdapterResult result = bridge.BuildRestoreRecords(request);
    if (result.status != expected_status) {
        return Fail("unexpected failure status");
    }

    if (result.required_identity_output_count != expected_required_identity_output_count) {
        return Fail("required identity output count mismatch");
    }

    if (result.required_transform_output_count != expected_required_transform_output_count) {
        return Fail("required transform output count mismatch");
    }

    if (!OutputIdentitiesMatch(fixture.output_identities, identity_before)) {
        return Fail("identity outputs changed during failed preflight");
    }

    if (!OutputTransformsMatch(fixture.output_transforms, transform_before)) {
        return Fail("transform outputs changed during failed preflight");
    }

    const RuntimeAssetWorldObjectAdapterSnapshot snapshot = bridge.Snapshot();
    if (snapshot.failed_operation_count != 1U) {
        return Fail("failed operation count mismatch");
    }

    if (snapshot.rejected_record_count != 1U) {
        return Fail("rejected record count mismatch");
    }

    if (snapshot.required_identity_output_count != expected_required_identity_output_count_64) {
        return Fail("snapshot required identity output count mismatch");
    }

    if (snapshot.required_transform_output_count != expected_required_transform_output_count_64) {
        return Fail("snapshot required transform output count mismatch");
    }

    return 0;
}

int TestBuildRestoreRecords() {
    AdapterFixture fixture = MakeFixture();
    RuntimeAssetWorldObjectAdapterBridge bridge{};
    const RuntimeAssetWorldObjectAdapterResult result = bridge.BuildRestoreRecords(fixture.Request());
    if (!result.Succeeded()) {
        return Fail("build restore records failed");
    }

    if (result.state.output_identity_count != OUTPUT_RECORD_COUNT) {
        return Fail("identity output count mismatch");
    }

    if (result.state.output_transform_count != OUTPUT_RECORD_COUNT) {
        return Fail("transform output count mismatch");
    }

    if (result.required_identity_output_count != 0U) {
        return Fail("success required identity output count mismatch");
    }

    if (result.required_transform_output_count != 0U) {
        return Fail("success required transform output count mismatch");
    }

    if (fixture.output_identities[0U].world_object_id.value != WORLD_OBJECT_A.value) {
        return Fail("first identity world object mismatch");
    }

    if (!ObjectHandlesMatch(fixture.output_identities[0U].object_handle, OBJECT_HANDLE_A)) {
        return Fail("first identity object handle mismatch");
    }

    if (fixture.output_transforms[1U].world_object_id.value != WORLD_OBJECT_B.value) {
        return Fail("second transform world object mismatch");
    }

    if (!TransformsMatch(fixture.output_transforms[1U].transform_state, fixture.scene_transforms[1U].transform)) {
        return Fail("second transform state mismatch");
    }

    const RuntimeAssetWorldObjectAdapterSnapshot snapshot = bridge.Snapshot();
    if (snapshot.built_identity_count != OUTPUT_RECORD_COUNT) {
        return Fail("built identity snapshot mismatch");
    }

    if (snapshot.built_transform_count != OUTPUT_RECORD_COUNT) {
        return Fail("built transform snapshot mismatch");
    }

    if (snapshot.required_identity_output_count != 0U) {
        return Fail("success snapshot required identity output count mismatch");
    }

    if (snapshot.required_transform_output_count != 0U) {
        return Fail("success snapshot required transform output count mismatch");
    }

    return 0;
}

int TestBuildTargetFamilyAliasRestoreRecords() {
    AdapterFixture fixture = MakeFixture();
    fixture.mappings[0U].target_kind = RuntimeAssetTargetIdentityKind::ModelNode;
    fixture.mappings[1U].target_kind = RuntimeAssetTargetIdentityKind::SkeletonJoint;

    RuntimeAssetWorldObjectAdapterBridge bridge{};
    const RuntimeAssetWorldObjectAdapterResult result = bridge.BuildRestoreRecords(fixture.Request());
    if (!result.Succeeded()) {
        return Fail("target family alias restore records failed");
    }

    if (result.state.output_identity_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family identity output count mismatch");
    }

    if (result.state.output_transform_count != OUTPUT_RECORD_COUNT) {
        return Fail("target family transform output count mismatch");
    }

    if (fixture.output_identities[0U].world_object_id.value != WORLD_OBJECT_A.value) {
        return Fail("model node identity world object mismatch");
    }

    if (fixture.output_identities[1U].world_object_id.value != WORLD_OBJECT_B.value) {
        return Fail("skeleton joint identity world object mismatch");
    }

    if (!ObjectHandlesMatch(fixture.output_identities[0U].object_handle, OBJECT_HANDLE_A)) {
        return Fail("model node object handle mismatch");
    }

    if (!ObjectHandlesMatch(fixture.output_identities[1U].object_handle, OBJECT_HANDLE_B)) {
        return Fail("skeleton joint object handle mismatch");
    }

    if (fixture.output_transforms[0U].world_object_id.value != WORLD_OBJECT_A.value) {
        return Fail("model node transform world object mismatch");
    }

    if (fixture.output_transforms[1U].world_object_id.value != WORLD_OBJECT_B.value) {
        return Fail("skeleton joint transform world object mismatch");
    }

    if (!TransformsMatch(fixture.output_transforms[0U].transform_state, fixture.scene_transforms[0U].transform)) {
        return Fail("model node transform state mismatch");
    }

    if (!TransformsMatch(fixture.output_transforms[1U].transform_state, fixture.scene_transforms[1U].transform)) {
        return Fail("skeleton joint transform state mismatch");
    }

    return 0;
}

int TestRejectMissingRuntimeInstanceMapping() {
    AdapterFixture fixture = MakeFixture();
    RuntimeAssetWorldObjectAdapterRequest request = fixture.Request();
    request.runtime_instance_mappings = nullptr;
    return ExpectFailureWithoutOutputMutation(
        fixture,
        request,
        RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput);
}

int TestRejectInvalidTargetKind() {
    AdapterFixture fixture = MakeFixture();
    fixture.mappings[0U].target_kind = RuntimeAssetTargetIdentityKind::Unknown;
    return ExpectFailureWithoutOutputMutation(
        fixture,
        fixture.Request(),
        RuntimeAssetWorldObjectAdapterStatus::UnsupportedTargetKind);
}

int TestRejectSceneEntityIndexOverflow() {
    AdapterFixture fixture = MakeFixture();
    fixture.mappings[0U].scene_entity_index = 99U;
    return ExpectFailureWithoutOutputMutation(
        fixture,
        fixture.Request(),
        RuntimeAssetWorldObjectAdapterStatus::InvalidSceneEntityIndex);
}

int TestRejectInvalidWorldObjectId() {
    AdapterFixture fixture = MakeFixture();
    fixture.scene_entities[0U].world_object_id = WorldObjectId{};
    return ExpectFailureWithoutOutputMutation(
        fixture,
        fixture.Request(),
        RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId);
}

int TestRejectInvalidObjectHandle() {
    AdapterFixture fixture = MakeFixture();
    fixture.identity_records[0U].object_handle = ObjectHandle{};
    return ExpectFailureWithoutOutputMutation(
        fixture,
        fixture.Request(),
        RuntimeAssetWorldObjectAdapterStatus::InvalidObjectHandle);
}

int TestRejectDuplicateTargetId() {
    AdapterFixture fixture = MakeFixture();
    fixture.mappings[1U].target_id = fixture.mappings[0U].target_id;
    return ExpectFailureWithoutOutputMutation(
        fixture,
        fixture.Request(),
        RuntimeAssetWorldObjectAdapterStatus::DuplicateTargetId);
}

int TestRejectDuplicateWorldObjectId() {
    AdapterFixture fixture = MakeFixture();
    fixture.identity_records[1U].world_object_id = WORLD_OBJECT_A;
    fixture.scene_entities[1U].world_object_id = WORLD_OBJECT_A;
    fixture.scene_transforms[1U].world_object_id = WORLD_OBJECT_A;
    return ExpectFailureWithoutOutputMutation(
        fixture,
        fixture.Request(),
        RuntimeAssetWorldObjectAdapterStatus::DuplicateWorldObjectId);
}

int TestRejectDuplicateObjectHandle() {
    AdapterFixture fixture = MakeFixture();
    fixture.identity_records[1U].object_handle = fixture.identity_records[0U].object_handle;
    return ExpectFailureWithoutOutputMutation(
        fixture,
        fixture.Request(),
        RuntimeAssetWorldObjectAdapterStatus::DuplicateObjectHandle);
}

int TestRejectSmallIdentityOutput() {
    AdapterFixture fixture = MakeFixture();
    RuntimeAssetWorldObjectAdapterRequest request = fixture.Request();
    request.output_identity_capacity = 1U;
    return ExpectFailureWithoutOutputMutation(
        fixture,
        request,
        RuntimeAssetWorldObjectAdapterStatus::IdentityOutputCapacityExceeded,
        OUTPUT_RECORD_COUNT,
        0U);
}

int TestRejectSmallTransformOutput() {
    AdapterFixture fixture = MakeFixture();
    RuntimeAssetWorldObjectAdapterRequest request = fixture.Request();
    request.output_transform_capacity = 1U;
    return ExpectFailureWithoutOutputMutation(
        fixture,
        request,
        RuntimeAssetWorldObjectAdapterStatus::TransformOutputCapacityExceeded,
        0U,
        OUTPUT_RECORD_COUNT);
}

int TestDoesNotMutateWorldOrObjectRegistryDuringBuild() {
    WorldDesc world_desc{};
    world_desc.object_capacity = 4U;
    WorldInstance world(world_desc);
    WorldObjectDesc object_desc{};
    object_desc.id = WORLD_OBJECT_A;
    const WorldRegistrationResult world_result = world.RegisterObject(object_desc);
    if (!world_result.Succeeded()) {
        return Fail("world setup failed");
    }

    ObjectRegistry object_registry{};
    ObjectDescriptor descriptor{OBJECT_TYPE_PLAYER, 1U};
    const ObjectRegistrationResult object_result = object_registry.CreateSyntheticObject(descriptor);
    if (!object_result.Succeeded()) {
        return Fail("object setup failed");
    }

    AdapterFixture fixture = MakeFixture();
    fixture.identity_records[0U].object_handle = object_result.handle;
    const WorldSnapshot world_before = world.Snapshot();
    const ObjectSnapshot object_before = object_registry.Snapshot();

    RuntimeAssetWorldObjectAdapterBridge bridge{};
    const RuntimeAssetWorldObjectAdapterResult adapter_result = bridge.BuildRestoreRecords(fixture.Request());
    if (!adapter_result.Succeeded()) {
        return Fail("adapter build failed");
    }

    const WorldSnapshot world_after = world.Snapshot();
    if (!WorldSnapshotsMatch(world_before, world_after)) {
        return Fail("world snapshot changed during adapter build");
    }

    const ObjectSnapshot object_after = object_registry.Snapshot();
    if (!ObjectSnapshotsMatch(object_before, object_after)) {
        return Fail("object snapshot changed during adapter build");
    }

    return 0;
}

int TestApplySampledTransformToCallerOwnedWorldObject() {
    AdapterFixture fixture = MakeFixture();
    const std::array<WorldSceneObjectTransformRestoreIdentityRecord, OUTPUT_RECORD_COUNT> identity_before =
        fixture.output_identities;
    const std::array<WorldSceneObjectTransformRestoreTransformRecord, OUTPUT_RECORD_COUNT> transform_before =
        fixture.output_transforms;

    WorldDesc world_desc{};
    world_desc.object_capacity = 4U;
    WorldInstance world(world_desc);
    if (RegisterWorldObject(&world, WORLD_OBJECT_A) != 0) {
        return 1;
    }

    if (RegisterWorldObject(&world, WORLD_OBJECT_B) != 0) {
        return 1;
    }

    WorldTransformBridgeDesc transform_desc{};
    transform_desc.bridge_capacity = 4U;
    WorldTransformBridge transform_destination(world, transform_desc);
    const WorldTransformState initial_a = Transform(-10.0F);
    const WorldTransformState initial_b = Transform(-20.0F);
    if (RegisterTransform(&transform_destination, WORLD_OBJECT_A, initial_a) != 0) {
        return 1;
    }

    if (RegisterTransform(&transform_destination, WORLD_OBJECT_B, initial_b) != 0) {
        return 1;
    }

    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    sampled_values[0U].target = WORLD_OBJECT_A;
    sampled_values[0U].channel = AnimationRuntimeChannel::TranslationX;
    sampled_values[0U].value = 31.0F;
    sampled_values[1U].target = WORLD_OBJECT_A;
    sampled_values[1U].channel = AnimationRuntimeChannel::RotationY;
    sampled_values[1U].value = 0.25F;
    sampled_values[2U].target = WORLD_OBJECT_B;
    sampled_values[2U].channel = AnimationRuntimeChannel::ScaleZ;
    sampled_values[2U].value = 3.5F;

    RuntimeAssetWorldObjectTransformApplicationRequest request = MakeTransformApplicationRequest(
        &fixture,
        &transform_destination,
        sampled_values.data(),
        static_cast<std::uint32_t>(sampled_values.size()));
    RuntimeAssetWorldObjectAdapterBridge bridge{};
    const RuntimeAssetWorldObjectAdapterResult result = bridge.ApplySampledTransforms(request);
    if (!result.Succeeded()) {
        return Fail("sampled transform application failed");
    }

    if (result.state.input_mapping_count != OUTPUT_RECORD_COUNT) {
        return Fail("sampled transform application mapping count mismatch");
    }

    if (result.state.applied_transform_value_count != 3U) {
        return Fail("sampled transform applied value count mismatch");
    }

    if (result.state.updated_world_object_count != 2U) {
        return Fail("sampled transform updated object count mismatch");
    }

    const WorldTransformResult transform_a = transform_destination.Query(WORLD_OBJECT_A);
    if (transform_a.status != WorldTransformStatus::Success) {
        return Fail("sampled transform query a failed");
    }

    if (transform_a.transform_state.translation_x != 31.0F ||
        transform_a.transform_state.rotation_y != 0.25F ||
        transform_a.transform_state.translation_y != initial_a.translation_y) {
        return Fail("sampled transform application a mismatch");
    }

    const WorldTransformResult transform_b = transform_destination.Query(WORLD_OBJECT_B);
    if (transform_b.status != WorldTransformStatus::Success) {
        return Fail("sampled transform query b failed");
    }

    if (transform_b.transform_state.scale_z != 3.5F ||
        transform_b.transform_state.translation_x != initial_b.translation_x) {
        return Fail("sampled transform application b mismatch");
    }

    if (!OutputIdentitiesMatch(fixture.output_identities, identity_before)) {
        return Fail("sampled transform application mutated identity outputs");
    }

    if (!OutputTransformsMatch(fixture.output_transforms, transform_before)) {
        return Fail("sampled transform application mutated transform outputs");
    }

    const RuntimeAssetWorldObjectAdapterSnapshot snapshot = bridge.Snapshot();
    if (snapshot.transform_application_attempt_count != 1U) {
        return Fail("sampled transform application attempt count mismatch");
    }

    if (snapshot.applied_transform_value_count != 3U) {
        return Fail("sampled transform snapshot applied value count mismatch");
    }

    if (snapshot.updated_world_object_count != 2U) {
        return Fail("sampled transform snapshot updated object count mismatch");
    }

    if (snapshot.built_identity_count != 0U || snapshot.built_transform_count != 0U) {
        return Fail("sampled transform application should not build restore records");
    }

    return 0;
}

int TestRejectUnmappedSampledTransformWithoutMutation() {
    AdapterFixture fixture = MakeFixture();

    WorldDesc world_desc{};
    world_desc.object_capacity = 4U;
    WorldInstance world(world_desc);
    if (RegisterWorldObject(&world, WORLD_OBJECT_A) != 0) {
        return 1;
    }

    if (RegisterWorldObject(&world, WORLD_OBJECT_B) != 0) {
        return 1;
    }

    if (RegisterWorldObject(&world, WORLD_OBJECT_UNMAPPED) != 0) {
        return 1;
    }

    WorldTransformBridgeDesc transform_desc{};
    transform_desc.bridge_capacity = 4U;
    WorldTransformBridge transform_destination(world, transform_desc);
    const WorldTransformState initial_a = Transform(-10.0F);
    const WorldTransformState initial_unmapped = Transform(70.0F);
    const WorldTransformState initial_b = Transform(-20.0F);
    if (RegisterTransform(&transform_destination, WORLD_OBJECT_A, initial_a) != 0) {
        return 1;
    }

    if (RegisterTransform(&transform_destination, WORLD_OBJECT_B, initial_b) != 0) {
        return 1;
    }

    if (RegisterTransform(&transform_destination, WORLD_OBJECT_UNMAPPED, initial_unmapped) != 0) {
        return 1;
    }

    std::array<AnimationRuntimeSampledValue, 1U> sampled_values{};
    sampled_values[0U].target = WORLD_OBJECT_UNMAPPED;
    sampled_values[0U].channel = AnimationRuntimeChannel::TranslationX;
    sampled_values[0U].value = 99.0F;

    RuntimeAssetWorldObjectTransformApplicationRequest request = MakeTransformApplicationRequest(
        &fixture,
        &transform_destination,
        sampled_values.data(),
        static_cast<std::uint32_t>(sampled_values.size()));
    RuntimeAssetWorldObjectAdapterBridge bridge{};
    const RuntimeAssetWorldObjectAdapterResult result = bridge.ApplySampledTransforms(request);
    if (result.status != RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound) {
        return Fail("unmapped sampled transform status mismatch");
    }

    const WorldTransformResult unmapped_result = transform_destination.Query(WORLD_OBJECT_UNMAPPED);
    if (unmapped_result.status != WorldTransformStatus::Success) {
        return Fail("unmapped sampled transform query failed");
    }

    if (!TransformsMatch(unmapped_result.transform_state, initial_unmapped)) {
        return Fail("unmapped sampled transform mutated target");
    }

    const WorldTransformResult mapped_result = transform_destination.Query(WORLD_OBJECT_A);
    if (mapped_result.status != WorldTransformStatus::Success) {
        return Fail("mapped transform query failed");
    }

    if (!TransformsMatch(mapped_result.transform_state, initial_a)) {
        return Fail("unmapped sampled transform mutated mapped target");
    }

    const RuntimeAssetWorldObjectAdapterSnapshot snapshot = bridge.Snapshot();
    if (snapshot.failed_operation_count != 1U) {
        return Fail("unmapped sampled transform failure count mismatch");
    }

    if (snapshot.applied_transform_value_count != 0U || snapshot.updated_world_object_count != 0U) {
        return Fail("unmapped sampled transform snapshot mutation mismatch");
    }

    return 0;
}

int RunTest(std::string_view test_name) {
    constexpr std::array<TestCase, 15U> TESTS{{
        {TEST_BUILD_RESTORE_RECORDS, TestBuildRestoreRecords},
        {TEST_BUILD_TARGET_FAMILY_ALIAS_RESTORE_RECORDS, TestBuildTargetFamilyAliasRestoreRecords},
        {TEST_REJECT_MISSING_MAPPING, TestRejectMissingRuntimeInstanceMapping},
        {TEST_REJECT_TARGET_KIND, TestRejectInvalidTargetKind},
        {TEST_REJECT_SCENE_ENTITY_INDEX, TestRejectSceneEntityIndexOverflow},
        {TEST_REJECT_WORLD_OBJECT, TestRejectInvalidWorldObjectId},
        {TEST_REJECT_OBJECT_HANDLE, TestRejectInvalidObjectHandle},
        {TEST_REJECT_DUPLICATE_TARGET, TestRejectDuplicateTargetId},
        {TEST_REJECT_DUPLICATE_WORLD_OBJECT, TestRejectDuplicateWorldObjectId},
        {TEST_REJECT_DUPLICATE_OBJECT_HANDLE, TestRejectDuplicateObjectHandle},
        {TEST_REJECT_SMALL_IDENTITY_OUTPUT, TestRejectSmallIdentityOutput},
        {TEST_REJECT_SMALL_TRANSFORM_OUTPUT, TestRejectSmallTransformOutput},
        {TEST_NO_WORLD_OBJECT_REGISTRY_MUTATION, TestDoesNotMutateWorldOrObjectRegistryDuringBuild},
        {TEST_APPLY_SAMPLED_TRANSFORM, TestApplySampledTransformToCallerOwnedWorldObject},
        {TEST_REJECT_UNMAPPED_SAMPLED_TRANSFORM, TestRejectUnmappedSampledTransformWithoutMutation},
    }};

    for (const TestCase &test : TESTS) {
        if (test.name == test_name) {
            return test.function();
        }
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
