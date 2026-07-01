// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"
#include "YuEngine/Kernel/RuntimeFrameMode.h"
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
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterBridge.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterIdentityRecord.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterResult.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterStatus.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectProducerPlaybackRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridge.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffResult.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffSnapshot.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffStatus.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectTimelineTransformSampleRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectTransformApplicationRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectTransformSamplerBridgeRequest.h"
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

using yuengine::animation::AnimationRuntimeChannel;
using yuengine::animation::AnimationRuntimeClipRecord;
using yuengine::animation::AnimationRuntimeInterpolation;
using yuengine::animation::AnimationRuntimeKeyframeRecord;
using yuengine::animation::AnimationRuntimeSampleRequest;
using yuengine::animation::AnimationRuntimeSampledValue;
using yuengine::animation::AnimationRuntimeTrackRecord;
using yuengine::kernel::RuntimeFrameContext;
using yuengine::kernel::RuntimeFrameMode;
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
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterBridge;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterIdentityRecord;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterResult;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectAdapterStatus;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectProducerPlaybackBatchRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectProducerPlaybackRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffBridge;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffResult;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffSnapshot;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectRestoreHandoffStatus;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectTimelineTransformSampleRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectTransformApplicationRequest;
using yuengine::runtimeassetworldadapter::RuntimeAssetWorldObjectTransformSamplerBridgeRequest;
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
bool RuntimeAssetWorldObjectRecordStreamHandoffFixtureTestNameMatches(std::string_view test_name);
int RunRuntimeAssetWorldObjectRecordStreamHandoffFixtureTest(std::string_view test_name);
bool RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTestNameMatches(std::string_view test_name);
int RunRuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest(std::string_view test_name);

namespace {
constexpr std::uint32_t OUTPUT_RECORD_COUNT = 2U;
constexpr std::uint32_t SCRATCH_RECORD_COUNT = 8U;
constexpr std::uint32_t SIDECAR_RECORD_COUNT = 2U;
constexpr const char *TEST_APPLY_RESTORE =
    "RuntimeAssetWorldObjectRestoreHandoff_AppliesAdapterRecordsThroughWorldRestoreBridge";
constexpr const char *TEST_APPLY_SAMPLED_TRANSFORMS =
    "RuntimeAssetWorldObjectRestoreHandoff_AppliesSampledTransformsThroughAdapterBridge";
constexpr const char *TEST_APPLY_RUNTIME_ANIMATION_SAMPLES =
    "RuntimeAssetWorldObjectRestoreHandoff_AppliesRuntimeAnimationSamplesThroughSamplerBridge";
constexpr const char *TEST_APPLY_TIMELINE_SAMPLES =
    "RuntimeAssetWorldObjectRestoreHandoff_AppliesTimelineSamplesThroughSamplerRequest";
constexpr const char *TEST_SNAPSHOT_TIMELINE_APPLICATION_COUNT =
    "RuntimeAssetWorldObjectRestoreHandoff_SnapshotsTimelineTransformApplicationCountWithoutMutation";
constexpr const char *TEST_APPLY_PRODUCER_PLAYBACK =
    "RuntimeAssetWorldObjectRestoreHandoff_AppliesProducerPlaybackThroughScratchHandoff";
constexpr const char *TEST_APPLY_PRODUCER_PLAYBACK_BATCH =
    "RuntimeAssetWorldObjectRestoreHandoff_AppliesProducerPlaybackBatchThroughScratchHandoff";
constexpr const char *TEST_SNAPSHOT_PRODUCER_PLAYBACK_BATCH_COUNT =
    "RuntimeAssetWorldObjectRestoreHandoff_SnapshotsProducerPlaybackBatchCountWithoutMutation";
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
constexpr const char *TEST_REJECT_UNMAPPED_SAMPLED_TRANSFORM =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsUnmappedSampledTransformBeforeWorldRestore";
constexpr const char *TEST_REJECT_TIMELINE_OUTPUT_ATOMIC =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsTimelineSamplerOutputFailureWithoutMutation";
constexpr const char *TEST_REJECT_TIMELINE_APPLICATION_COUNT =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsTimelineTransformApplicationCountFailureWithoutMutation";
constexpr const char *TEST_REJECT_PRODUCER_PLAYBACK_ATOMIC =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsProducerPlaybackFailureWithoutMutation";
constexpr const char *TEST_REJECT_PRODUCER_PLAYBACK_BATCH_ATOMIC =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsProducerPlaybackBatchFailureWithoutMutation";
constexpr const char *TEST_REJECT_PRODUCER_PLAYBACK_BATCH_COUNT =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsProducerPlaybackBatchCountFailureWithoutMutation";
constexpr const char *TEST_REJECT_TIMELINE_SAMPLE_TARGETS =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsTimelineSampleTargetsBeforeWorldRestore";
constexpr const char *TEST_REJECT_WORLD_GATE =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsWorldGateWithoutRestoringSidecars";
constexpr const char *TEST_REJECT_NULL_ADAPTER =
    "RuntimeAssetWorldObjectRestoreHandoff_RejectsNullAdapterRequestWithoutMutation";
constexpr WorldObjectId WORLD_OBJECT_A{1U};
constexpr WorldObjectId WORLD_OBJECT_B{2U};
constexpr WorldObjectId WORLD_OBJECT_UNMAPPED{77U};
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
constexpr std::uint32_t CLIP_ID = 2101U;
constexpr std::uint32_t TRACK_ID = 2201U;
constexpr std::uint64_t HALF_SECOND_NANOSECONDS = 500000000ULL;

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

RuntimeFrameContext FrameContext(std::uint64_t fixed_time_nanoseconds) {
    RuntimeFrameContext context{};
    context.frame_index = 1U;
    context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    context.fixed_time_nanoseconds = fixed_time_nanoseconds;
    context.frame_mode = RuntimeFrameMode::Fixed;
    return context;
}

AnimationRuntimeClipRecord ClipRecord(std::size_t track_count) {
    AnimationRuntimeClipRecord clip{};
    clip.clip_id = CLIP_ID;
    clip.duration_seconds = 1.0F;
    clip.first_track_index = 0U;
    clip.track_count = track_count;
    clip.layer_count = 1U;
    clip.is_valid = true;
    return clip;
}

AnimationRuntimeTrackRecord TrackRecord(
    WorldObjectId target,
    AnimationRuntimeChannel channel,
    std::size_t first_keyframe_index) {
    AnimationRuntimeTrackRecord track{};
    track.track_id = TRACK_ID + static_cast<std::uint32_t>(first_keyframe_index);
    track.target = target;
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

AnimationRuntimeSampleRequest MakeSampleRequest(
    std::span<const AnimationRuntimeClipRecord> clips,
    std::span<const AnimationRuntimeTrackRecord> tracks,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes) {
    AnimationRuntimeSampleRequest request{};
    request.clip_id = CLIP_ID;
    request.clips = clips;
    request.tracks = tracks;
    request.keyframes = keyframes;
    request.frame_context = FrameContext(HALF_SECOND_NANOSECONDS);
    request.clip_start_time_nanoseconds = 0U;
    return request;
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

RuntimeAssetWorldObjectTransformApplicationRequest MakeTransformApplicationRequest(
    HandoffFixture *fixture,
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

RuntimeAssetWorldObjectTransformSamplerBridgeRequest MakeTransformSamplerBridgeRequest(
    HandoffFixture *fixture,
    WorldTransformBridge *transform_destination,
    const AnimationRuntimeSampleRequest *sample_request,
    AnimationRuntimeSampledValue *sampled_value_output,
    std::uint32_t sampled_value_output_capacity) {
    RuntimeAssetWorldObjectTransformSamplerBridgeRequest request{};
    if (fixture == nullptr) {
        return request;
    }

    request.runtime_instance_mappings = fixture->mappings.data();
    request.runtime_instance_mapping_count = OUTPUT_RECORD_COUNT;
    request.identity_records = fixture->identity_records.data();
    request.identity_record_count = OUTPUT_RECORD_COUNT;
    request.transform_destination = transform_destination;
    request.sample_request = sample_request;
    request.sampled_value_output = sampled_value_output;
    request.sampled_value_output_capacity = sampled_value_output_capacity;
    return request;
}

RuntimeAssetWorldObjectTimelineTransformSampleRequest MakeTimelineTransformSampleRequest(
    HandoffFixture *fixture,
    WorldTransformBridge *transform_destination,
    std::span<const AnimationRuntimeClipRecord> clips,
    std::span<const AnimationRuntimeTrackRecord> tracks,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes,
    AnimationRuntimeSampledValue *sampled_value_scratch,
    std::uint32_t sampled_value_scratch_capacity,
    AnimationRuntimeSampledValue *sampled_value_output,
    std::uint32_t sampled_value_output_capacity) {
    RuntimeAssetWorldObjectTimelineTransformSampleRequest request{};
    if (fixture == nullptr) {
        return request;
    }

    request.runtime_instance_mappings = fixture->mappings.data();
    request.runtime_instance_mapping_count = OUTPUT_RECORD_COUNT;
    request.identity_records = fixture->identity_records.data();
    request.identity_record_count = OUTPUT_RECORD_COUNT;
    request.transform_destination = transform_destination;
    request.clip_id = CLIP_ID;
    request.clips = clips;
    request.tracks = tracks;
    request.keyframes = keyframes;
    request.frame_context = FrameContext(HALF_SECOND_NANOSECONDS);
    request.clip_start_time_nanoseconds = 0U;
    request.sampled_value_scratch = sampled_value_scratch;
    request.sampled_value_scratch_capacity = sampled_value_scratch_capacity;
    request.sampled_value_output = sampled_value_output;
    request.sampled_value_output_capacity = sampled_value_output_capacity;
    return request;
}

RuntimeAssetWorldObjectProducerPlaybackRequest MakeProducerPlaybackRequest(
    HandoffFixture *fixture,
    WorldTransformBridge *transform_destination,
    std::span<const AnimationRuntimeClipRecord> clips,
    std::span<const AnimationRuntimeTrackRecord> tracks,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes,
    AnimationRuntimeSampledValue *sampled_value_scratch,
    std::uint32_t sampled_value_scratch_capacity,
    AnimationRuntimeSampledValue *sampled_value_output,
    std::uint32_t sampled_value_output_capacity) {
    RuntimeAssetWorldObjectProducerPlaybackRequest request{};
    if (fixture == nullptr) {
        return request;
    }

    request.runtime_instance_mappings = fixture->mappings.data();
    request.runtime_instance_mapping_count = OUTPUT_RECORD_COUNT;
    request.identity_records = fixture->identity_records.data();
    request.identity_record_count = OUTPUT_RECORD_COUNT;
    request.transform_destination = transform_destination;
    request.export_clip_id = CLIP_ID;
    request.export_clips = clips;
    request.export_tracks = tracks;
    request.export_keyframes = keyframes;
    request.playback_frame_context = FrameContext(HALF_SECOND_NANOSECONDS);
    request.export_clip_start_time_nanoseconds = 0U;
    request.sampled_value_scratch = sampled_value_scratch;
    request.sampled_value_scratch_capacity = sampled_value_scratch_capacity;
    request.sampled_value_output = sampled_value_output;
    request.sampled_value_output_capacity = sampled_value_output_capacity;
    return request;
}

RuntimeAssetWorldObjectProducerPlaybackBatchRequest MakeProducerPlaybackBatchRequest(
    const RuntimeAssetWorldObjectProducerPlaybackRequest *playback_requests,
    std::uint32_t playback_request_count,
    AnimationRuntimeSampledValue *sampled_value_scratch,
    std::uint32_t sampled_value_scratch_capacity,
    AnimationRuntimeSampledValue *sampled_value_output,
    std::uint32_t sampled_value_output_capacity) {
    RuntimeAssetWorldObjectProducerPlaybackBatchRequest request{};
    request.producer_playback_requests = playback_requests;
    request.producer_playback_request_count = playback_request_count;
    request.sampled_value_scratch = sampled_value_scratch;
    request.sampled_value_scratch_capacity = sampled_value_scratch_capacity;
    request.sampled_value_output = sampled_value_output;
    request.sampled_value_output_capacity = sampled_value_output_capacity;
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

bool SampledValuesMatch(AnimationRuntimeSampledValue left, AnimationRuntimeSampledValue right) {
    if (left.target.value != right.target.value) {
        return false;
    }

    if (left.channel != right.channel) {
        return false;
    }

    return left.value == right.value;
}

bool SampledValueArraysMatch(
    std::span<const AnimationRuntimeSampledValue> left,
    std::span<const AnimationRuntimeSampledValue> right) {
    if (left.size() != right.size()) {
        return false;
    }

    std::size_t value_index = 0U;
    while (value_index < left.size()) {
        if (!SampledValuesMatch(left[value_index], right[value_index])) {
            return false;
        }

        ++value_index;
    }

    return true;
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

int TestApplySampledTransformsThroughAdapterBridge() {
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
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request =
        MakeTransformApplicationRequest(
            &fixture,
            &transform_destination,
            sampled_values.data(),
            static_cast<std::uint32_t>(sampled_values.size()));
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
    handoff_request.transform_application_request = &transform_application_request;

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("sampled transform handoff apply failed");
    }

    if (result.state.applied_transform_value_count != 3U) {
        return Fail("sampled transform handoff applied value count mismatch");
    }

    if (result.state.updated_world_object_count != 2U) {
        return Fail("sampled transform handoff updated object count mismatch");
    }

    const WorldTransformResult transform_a = transform_destination.Query(WORLD_OBJECT_A);
    if (!transform_a.Succeeded()) {
        return Fail("sampled transform handoff query a failed");
    }

    if (transform_a.transform_state.translation_x != 31.0F ||
        transform_a.transform_state.rotation_y != 0.25F ||
        transform_a.transform_state.translation_y != fixture.scene_transforms[0U].transform.translation_y) {
        return Fail("sampled transform handoff transform a mismatch");
    }

    const WorldTransformResult transform_b = transform_destination.Query(WORLD_OBJECT_B);
    if (!transform_b.Succeeded()) {
        return Fail("sampled transform handoff query b failed");
    }

    if (transform_b.transform_state.scale_z != 3.5F ||
        transform_b.transform_state.translation_x != fixture.scene_transforms[1U].transform.translation_x) {
        return Fail("sampled transform handoff transform b mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.applied_transform_value_count != 3U) {
        return Fail("sampled transform handoff snapshot applied value count mismatch");
    }

    if (snapshot.updated_world_object_count != 2U) {
        return Fail("sampled transform handoff snapshot updated object count mismatch");
    }

    return 0;
}

int TestApplyRuntimeAnimationSamplesThroughSamplerBridge() {
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

    std::array<AnimationRuntimeClipRecord, 1U> clips{};
    clips[0U] = ClipRecord(3U);
    std::array<AnimationRuntimeTrackRecord, 3U> tracks{};
    tracks[0U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 0U);
    tracks[1U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 2U);
    tracks[2U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 4U);
    std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{};
    keyframes[0U] = Keyframe(0.0F, 30.0F);
    keyframes[1U] = Keyframe(1.0F, 32.0F);
    keyframes[2U] = Keyframe(0.0F, 0.0F);
    keyframes[3U] = Keyframe(1.0F, 0.5F);
    keyframes[4U] = Keyframe(0.0F, 3.0F);
    keyframes[5U] = Keyframe(1.0F, 4.0F);
    const AnimationRuntimeSampleRequest sample_request = MakeSampleRequest(
        std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()));

    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const std::uint64_t sampled_value_count_snapshot = static_cast<std::uint64_t>(sampled_value_count);
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectTransformSamplerBridgeRequest sampler_bridge_request =
        MakeTransformSamplerBridgeRequest(
            &fixture,
            &transform_destination,
            &sample_request,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    const RuntimeAssetWorldObjectAdapterResult sampler_result =
        adapter_bridge.BuildTransformApplicationRequest(
            sampler_bridge_request,
            &transform_application_request);
    if (!sampler_result.Succeeded()) {
        return Fail("runtime animation sampler bridge request failed");
    }

    if (sampler_result.state.sampled_transform_value_count != sampled_value_count) {
        return Fail("runtime animation sampler bridge sampled count mismatch");
    }

    if (transform_application_request.sampled_values != sampled_values.data()) {
        return Fail("runtime animation sampler bridge output pointer mismatch");
    }

    if (transform_application_request.sampled_value_count != sampled_value_count) {
        return Fail("runtime animation sampler bridge output count mismatch");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 1U) {
        return Fail("runtime animation sampler bridge attempt count mismatch");
    }

    if (adapter_snapshot.sampled_transform_value_count != sampled_value_count_snapshot) {
        return Fail("runtime animation sampler bridge snapshot count mismatch");
    }

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
    handoff_request.transform_application_request = &transform_application_request;

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("runtime animation sampler handoff apply failed");
    }

    if (result.state.applied_transform_value_count != sampled_value_count) {
        return Fail("runtime animation sampler handoff applied value count mismatch");
    }

    if (result.state.updated_world_object_count != 2U) {
        return Fail("runtime animation sampler handoff updated object count mismatch");
    }

    const WorldTransformResult transform_a = transform_destination.Query(WORLD_OBJECT_A);
    if (!transform_a.Succeeded()) {
        return Fail("runtime animation sampler handoff query a failed");
    }

    if (transform_a.transform_state.translation_x != 31.0F ||
        transform_a.transform_state.rotation_y != 0.25F ||
        transform_a.transform_state.translation_y != fixture.scene_transforms[0U].transform.translation_y) {
        return Fail("runtime animation sampler handoff transform a mismatch");
    }

    const WorldTransformResult transform_b = transform_destination.Query(WORLD_OBJECT_B);
    if (!transform_b.Succeeded()) {
        return Fail("runtime animation sampler handoff query b failed");
    }

    if (transform_b.transform_state.scale_z != 3.5F ||
        transform_b.transform_state.translation_x != fixture.scene_transforms[1U].transform.translation_x) {
        return Fail("runtime animation sampler handoff transform b mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.applied_transform_value_count != sampled_value_count_snapshot) {
        return Fail("runtime animation sampler handoff snapshot applied count mismatch");
    }

    return 0;
}

int TestApplyTimelineSamplesThroughSamplerRequest() {
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

    std::array<AnimationRuntimeClipRecord, 1U> clips{};
    clips[0U] = ClipRecord(3U);
    std::array<AnimationRuntimeTrackRecord, 3U> tracks{};
    tracks[0U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 0U);
    tracks[1U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 2U);
    tracks[2U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 4U);
    std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{};
    keyframes[0U] = Keyframe(0.0F, 3.0F);
    keyframes[1U] = Keyframe(1.0F, 4.0F);
    keyframes[2U] = Keyframe(0.0F, 40.0F);
    keyframes[3U] = Keyframe(1.0F, 42.0F);
    keyframes[4U] = Keyframe(0.0F, 0.0F);
    keyframes[5U] = Keyframe(1.0F, 0.5F);

    std::array<AnimationRuntimeSampledValue, 3U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const std::uint64_t sampled_value_count_snapshot = static_cast<std::uint64_t>(sampled_value_count);
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest timeline_request =
        MakeTimelineTransformSampleRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult sampler_result =
        adapter_bridge.BuildTimelineTransformApplicationRequest(
            timeline_request,
            &transform_application_request);
    if (!sampler_result.Succeeded()) {
        return Fail("timeline sampler request failed");
    }

    if (sampler_result.state.sampled_transform_value_count != sampled_value_count) {
        return Fail("timeline sampler request sampled count mismatch");
    }

    if (sampled_values[0U].target.value != WORLD_OBJECT_B.value ||
        sampled_values[0U].channel != AnimationRuntimeChannel::ScaleZ ||
        sampled_values[0U].value != 3.5F) {
        return Fail("timeline sampler request first value mismatch");
    }

    if (sampled_values[1U].target.value != WORLD_OBJECT_A.value ||
        sampled_values[1U].channel != AnimationRuntimeChannel::TranslationX ||
        sampled_values[1U].value != 41.0F) {
        return Fail("timeline sampler request second value mismatch");
    }

    if (sampled_values[2U].target.value != WORLD_OBJECT_A.value ||
        sampled_values[2U].channel != AnimationRuntimeChannel::RotationY ||
        sampled_values[2U].value != 0.25F) {
        return Fail("timeline sampler request third value mismatch");
    }

    const RuntimeAssetWorldObjectAdapterResult repeated_sampler_result =
        adapter_bridge.BuildTimelineTransformApplicationRequest(
            timeline_request,
            &transform_application_request);
    if (!repeated_sampler_result.Succeeded()) {
        return Fail("timeline sampler repeated request failed");
    }

    if (transform_application_request.sampled_values != sampled_values.data()) {
        return Fail("timeline sampler request output pointer mismatch");
    }

    if (transform_application_request.sampled_value_count != sampled_value_count) {
        return Fail("timeline sampler request output count mismatch");
    }

    const RuntimeAssetWorldObjectAdapterResult preflight_result =
        adapter_bridge.PreflightSampledTransforms(transform_application_request);
    if (!preflight_result.Succeeded()) {
        return Fail("timeline sampler request preflight failed");
    }

    const RuntimeAssetWorldObjectAdapterResult repeated_preflight_result =
        adapter_bridge.PreflightSampledTransforms(transform_application_request);
    if (!repeated_preflight_result.Succeeded()) {
        return Fail("timeline sampler request repeated preflight failed");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 2U) {
        return Fail("timeline sampler request attempt count mismatch");
    }

    if (adapter_snapshot.sampled_transform_value_count != sampled_value_count_snapshot * 2U) {
        return Fail("timeline sampler request snapshot count mismatch");
    }

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
    handoff_request.transform_application_request = &transform_application_request;

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("timeline sampler handoff apply failed");
    }

    if (result.state.applied_transform_value_count != sampled_value_count) {
        return Fail("timeline sampler handoff applied count mismatch");
    }

    if (result.state.updated_world_object_count != 2U) {
        return Fail("timeline sampler handoff updated object count mismatch");
    }

    const WorldTransformResult transform_a = transform_destination.Query(WORLD_OBJECT_A);
    if (!transform_a.Succeeded()) {
        return Fail("timeline sampler handoff query a failed");
    }

    if (transform_a.transform_state.translation_x != 41.0F ||
        transform_a.transform_state.rotation_y != 0.25F ||
        transform_a.transform_state.translation_y != fixture.scene_transforms[0U].transform.translation_y) {
        return Fail("timeline sampler handoff transform a mismatch");
    }

    const WorldTransformResult transform_b = transform_destination.Query(WORLD_OBJECT_B);
    if (!transform_b.Succeeded()) {
        return Fail("timeline sampler handoff query b failed");
    }

    if (transform_b.transform_state.scale_z != 3.5F ||
        transform_b.transform_state.translation_x != fixture.scene_transforms[1U].transform.translation_x) {
        return Fail("timeline sampler handoff transform b mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.applied_transform_value_count != sampled_value_count_snapshot) {
        return Fail("timeline sampler handoff snapshot applied count mismatch");
    }

    return 0;
}

int TestSnapshotTimelineTransformApplicationCountWithoutMutation() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    WorldTransformBridge transform_destination(world);
    std::array<AnimationRuntimeClipRecord, 1U> clips{};
    clips[0U] = ClipRecord(3U);
    std::array<AnimationRuntimeTrackRecord, 3U> tracks{};
    tracks[0U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 0U);
    tracks[1U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 2U);
    tracks[2U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 4U);
    std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{};
    keyframes[0U] = Keyframe(0.0F, 3.0F);
    keyframes[1U] = Keyframe(1.0F, 4.0F);
    keyframes[2U] = Keyframe(0.0F, 40.0F);
    keyframes[3U] = Keyframe(1.0F, 42.0F);
    keyframes[4U] = Keyframe(0.0F, 0.0F);
    keyframes[5U] = Keyframe(1.0F, 0.5F);

    std::array<AnimationRuntimeSampledValue, 3U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const std::uint64_t sampled_value_count_snapshot = static_cast<std::uint64_t>(sampled_value_count);
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest timeline_request =
        MakeTimelineTransformSampleRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult build_result =
        adapter_bridge.BuildTimelineTransformApplicationRequest(
            timeline_request,
            &transform_application_request);
    if (!build_result.Succeeded()) {
        return Fail("timeline count snapshot setup failed");
    }

    const std::array<AnimationRuntimeSampledValue, 3U> previous_values = sampled_values;
    const RuntimeAssetWorldObjectTransformApplicationRequest previous_request = transform_application_request;
    const RuntimeAssetWorldObjectAdapterResult count_result =
        adapter_bridge.SnapshotTimelineTransformApplicationCount(timeline_request);
    if (!count_result.Succeeded()) {
        return Fail("timeline count snapshot failed");
    }

    if (count_result.required_sampled_transform_value_count != sampled_value_count) {
        return Fail("timeline count snapshot required count mismatch");
    }

    if (count_result.state.sampled_transform_value_count != sampled_value_count) {
        return Fail("timeline count snapshot state mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count ||
        transform_application_request.transform_destination != previous_request.transform_destination) {
        return Fail("timeline count snapshot mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("timeline count snapshot mutated values");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 2U) {
        return Fail("timeline count snapshot attempt count mismatch");
    }

    if (adapter_snapshot.required_sampled_transform_value_count != sampled_value_count_snapshot) {
        return Fail("timeline count snapshot required snapshot mismatch");
    }

    if (adapter_snapshot.sampled_transform_value_count != sampled_value_count_snapshot) {
        return Fail("timeline count snapshot sampled counter mismatch");
    }

    return 0;
}

int TestApplyProducerPlaybackThroughScratchHandoff() {
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

    std::array<AnimationRuntimeClipRecord, 1U> clips{};
    clips[0U] = ClipRecord(3U);
    std::array<AnimationRuntimeTrackRecord, 3U> tracks{};
    tracks[0U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 0U);
    tracks[1U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 2U);
    tracks[2U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 4U);
    std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{};
    keyframes[0U] = Keyframe(0.0F, 3.0F);
    keyframes[1U] = Keyframe(1.0F, 4.0F);
    keyframes[2U] = Keyframe(0.0F, 40.0F);
    keyframes[3U] = Keyframe(1.0F, 42.0F);
    keyframes[4U] = Keyframe(0.0F, 0.0F);
    keyframes[5U] = Keyframe(1.0F, 0.5F);

    std::array<AnimationRuntimeSampledValue, 3U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const std::uint64_t sampled_value_count_snapshot = static_cast<std::uint64_t>(sampled_value_count);
    const RuntimeAssetWorldObjectProducerPlaybackRequest playback_request =
        MakeProducerPlaybackRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult sampler_result =
        adapter_bridge.BuildProducerPlaybackTransformApplicationRequest(
            playback_request,
            &transform_application_request);
    if (!sampler_result.Succeeded()) {
        return Fail("producer playback sampler request failed");
    }

    if (sampler_result.state.sampled_transform_value_count != sampled_value_count) {
        return Fail("producer playback sampled count mismatch");
    }

    if (sampled_values[0U].target.value != WORLD_OBJECT_B.value ||
        sampled_values[0U].channel != AnimationRuntimeChannel::ScaleZ ||
        sampled_values[0U].value != 3.5F) {
        return Fail("producer playback first value mismatch");
    }

    if (sampled_values[1U].target.value != WORLD_OBJECT_A.value ||
        sampled_values[1U].channel != AnimationRuntimeChannel::TranslationX ||
        sampled_values[1U].value != 41.0F) {
        return Fail("producer playback second value mismatch");
    }

    if (sampled_values[2U].target.value != WORLD_OBJECT_A.value ||
        sampled_values[2U].channel != AnimationRuntimeChannel::RotationY ||
        sampled_values[2U].value != 0.25F) {
        return Fail("producer playback third value mismatch");
    }

    if (transform_application_request.sampled_values != sampled_values.data()) {
        return Fail("producer playback request output pointer mismatch");
    }

    if (transform_application_request.sampled_value_count != sampled_value_count) {
        return Fail("producer playback request output count mismatch");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 1U) {
        return Fail("producer playback attempt count mismatch");
    }

    if (adapter_snapshot.sampled_transform_value_count != sampled_value_count_snapshot) {
        return Fail("producer playback snapshot count mismatch");
    }

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
    handoff_request.transform_application_request = &transform_application_request;

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("producer playback handoff apply failed");
    }

    if (result.state.applied_transform_value_count != sampled_value_count) {
        return Fail("producer playback handoff applied count mismatch");
    }

    const WorldTransformResult transform_a = transform_destination.Query(WORLD_OBJECT_A);
    if (!transform_a.Succeeded()) {
        return Fail("producer playback handoff query a failed");
    }

    if (transform_a.transform_state.translation_x != 41.0F ||
        transform_a.transform_state.rotation_y != 0.25F ||
        transform_a.transform_state.translation_y != fixture.scene_transforms[0U].transform.translation_y) {
        return Fail("producer playback handoff transform a mismatch");
    }

    const WorldTransformResult transform_b = transform_destination.Query(WORLD_OBJECT_B);
    if (!transform_b.Succeeded()) {
        return Fail("producer playback handoff query b failed");
    }

    if (transform_b.transform_state.scale_z != 3.5F ||
        transform_b.transform_state.translation_x != fixture.scene_transforms[1U].transform.translation_x) {
        return Fail("producer playback handoff transform b mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.applied_transform_value_count != sampled_value_count_snapshot) {
        return Fail("producer playback handoff snapshot applied count mismatch");
    }

    return 0;
}

int TestApplyProducerPlaybackBatchThroughScratchHandoff() {
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

    std::array<AnimationRuntimeClipRecord, 1U> first_clips{};
    first_clips[0U] = ClipRecord(2U);
    std::array<AnimationRuntimeTrackRecord, 2U> first_tracks{};
    first_tracks[0U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 0U);
    first_tracks[1U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 2U);
    std::array<AnimationRuntimeKeyframeRecord, 4U> first_keyframes{};
    first_keyframes[0U] = Keyframe(0.0F, 40.0F);
    first_keyframes[1U] = Keyframe(1.0F, 42.0F);
    first_keyframes[2U] = Keyframe(0.0F, 3.0F);
    first_keyframes[3U] = Keyframe(1.0F, 4.0F);

    std::array<AnimationRuntimeClipRecord, 1U> second_clips{};
    second_clips[0U] = ClipRecord(1U);
    std::array<AnimationRuntimeTrackRecord, 1U> second_tracks{};
    second_tracks[0U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 0U);
    std::array<AnimationRuntimeKeyframeRecord, 2U> second_keyframes{};
    second_keyframes[0U] = Keyframe(0.0F, 0.0F);
    second_keyframes[1U] = Keyframe(1.0F, 0.5F);

    std::array<RuntimeAssetWorldObjectProducerPlaybackRequest, 2U> playback_requests{};
    playback_requests[0U] = MakeProducerPlaybackRequest(
        &fixture,
        &transform_destination,
        std::span<const AnimationRuntimeClipRecord>(first_clips.data(), first_clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(first_tracks.data(), first_tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(first_keyframes.data(), first_keyframes.size()),
        nullptr,
        0U,
        nullptr,
        0U);
    playback_requests[1U] = MakeProducerPlaybackRequest(
        &fixture,
        &transform_destination,
        std::span<const AnimationRuntimeClipRecord>(second_clips.data(), second_clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(second_tracks.data(), second_tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(second_keyframes.data(), second_keyframes.size()),
        nullptr,
        0U,
        nullptr,
        0U);

    std::array<AnimationRuntimeSampledValue, 3U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const std::uint64_t sampled_value_count_snapshot = static_cast<std::uint64_t>(sampled_value_count);
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest batch_request =
        MakeProducerPlaybackBatchRequest(
            playback_requests.data(),
            static_cast<std::uint32_t>(playback_requests.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult sampler_result =
        adapter_bridge.BuildProducerPlaybackBatchTransformApplicationRequest(
            batch_request,
            &transform_application_request);
    if (!sampler_result.Succeeded()) {
        return Fail("producer playback batch sampler request failed");
    }

    if (sampler_result.state.sampled_transform_value_count != sampled_value_count) {
        return Fail("producer playback batch sampled count mismatch");
    }

    if (sampled_values[0U].target.value != WORLD_OBJECT_A.value ||
        sampled_values[0U].channel != AnimationRuntimeChannel::TranslationX ||
        sampled_values[0U].value != 41.0F) {
        return Fail("producer playback batch first value mismatch");
    }

    if (sampled_values[1U].target.value != WORLD_OBJECT_B.value ||
        sampled_values[1U].channel != AnimationRuntimeChannel::ScaleZ ||
        sampled_values[1U].value != 3.5F) {
        return Fail("producer playback batch second value mismatch");
    }

    if (sampled_values[2U].target.value != WORLD_OBJECT_A.value ||
        sampled_values[2U].channel != AnimationRuntimeChannel::RotationY ||
        sampled_values[2U].value != 0.25F) {
        return Fail("producer playback batch third value mismatch");
    }

    if (transform_application_request.sampled_values != sampled_values.data()) {
        return Fail("producer playback batch request output pointer mismatch");
    }

    if (transform_application_request.sampled_value_count != sampled_value_count) {
        return Fail("producer playback batch request output count mismatch");
    }

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
    handoff_request.transform_application_request = &transform_application_request;

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (!result.Succeeded()) {
        return Fail("producer playback batch handoff apply failed");
    }

    if (result.state.applied_transform_value_count != sampled_value_count) {
        return Fail("producer playback batch handoff applied count mismatch");
    }

    const WorldTransformResult transform_a = transform_destination.Query(WORLD_OBJECT_A);
    if (!transform_a.Succeeded()) {
        return Fail("producer playback batch handoff query a failed");
    }

    if (transform_a.transform_state.translation_x != 41.0F ||
        transform_a.transform_state.rotation_y != 0.25F ||
        transform_a.transform_state.translation_y != fixture.scene_transforms[0U].transform.translation_y) {
        return Fail("producer playback batch handoff transform a mismatch");
    }

    const WorldTransformResult transform_b = transform_destination.Query(WORLD_OBJECT_B);
    if (!transform_b.Succeeded()) {
        return Fail("producer playback batch handoff query b failed");
    }

    if (transform_b.transform_state.scale_z != 3.5F ||
        transform_b.transform_state.translation_x != fixture.scene_transforms[1U].transform.translation_x) {
        return Fail("producer playback batch handoff transform b mismatch");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.applied_transform_value_count != sampled_value_count_snapshot) {
        return Fail("producer playback batch handoff snapshot applied count mismatch");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 1U) {
        return Fail("producer playback batch attempt count mismatch");
    }

    if (adapter_snapshot.sampled_transform_value_count != sampled_value_count_snapshot) {
        return Fail("producer playback batch snapshot count mismatch");
    }

    return 0;
}

int TestSnapshotProducerPlaybackBatchCountWithoutMutation() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    WorldTransformBridge transform_destination(world);
    std::array<AnimationRuntimeClipRecord, 1U> first_clips{};
    first_clips[0U] = ClipRecord(2U);
    std::array<AnimationRuntimeTrackRecord, 2U> first_tracks{};
    first_tracks[0U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 0U);
    first_tracks[1U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 2U);
    std::array<AnimationRuntimeKeyframeRecord, 4U> first_keyframes{};
    first_keyframes[0U] = Keyframe(0.0F, 40.0F);
    first_keyframes[1U] = Keyframe(1.0F, 42.0F);
    first_keyframes[2U] = Keyframe(0.0F, 3.0F);
    first_keyframes[3U] = Keyframe(1.0F, 4.0F);

    std::array<AnimationRuntimeClipRecord, 1U> second_clips{};
    second_clips[0U] = ClipRecord(1U);
    std::array<AnimationRuntimeTrackRecord, 1U> second_tracks{};
    second_tracks[0U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 0U);
    std::array<AnimationRuntimeKeyframeRecord, 2U> second_keyframes{};
    second_keyframes[0U] = Keyframe(0.0F, 0.0F);
    second_keyframes[1U] = Keyframe(1.0F, 0.5F);

    std::array<RuntimeAssetWorldObjectProducerPlaybackRequest, 2U> playback_requests{};
    playback_requests[0U] = MakeProducerPlaybackRequest(
        &fixture,
        &transform_destination,
        std::span<const AnimationRuntimeClipRecord>(first_clips.data(), first_clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(first_tracks.data(), first_tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(first_keyframes.data(), first_keyframes.size()),
        nullptr,
        0U,
        nullptr,
        0U);
    playback_requests[1U] = MakeProducerPlaybackRequest(
        &fixture,
        &transform_destination,
        std::span<const AnimationRuntimeClipRecord>(second_clips.data(), second_clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(second_tracks.data(), second_tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(second_keyframes.data(), second_keyframes.size()),
        nullptr,
        0U,
        nullptr,
        0U);

    std::array<AnimationRuntimeSampledValue, 3U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const std::uint64_t sampled_value_count_snapshot = static_cast<std::uint64_t>(sampled_value_count);
    const std::uint32_t playback_request_count = static_cast<std::uint32_t>(playback_requests.size());
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest batch_request =
        MakeProducerPlaybackBatchRequest(
            playback_requests.data(),
            playback_request_count,
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult build_result =
        adapter_bridge.BuildProducerPlaybackBatchTransformApplicationRequest(
            batch_request,
            &transform_application_request);
    if (!build_result.Succeeded()) {
        return Fail("producer playback batch count setup failed");
    }

    const std::array<AnimationRuntimeSampledValue, 3U> previous_values = sampled_values;
    const RuntimeAssetWorldObjectTransformApplicationRequest previous_request = transform_application_request;
    const RuntimeAssetWorldObjectAdapterResult count_result =
        adapter_bridge.SnapshotProducerPlaybackBatchTransformApplicationCount(batch_request);
    if (!count_result.Succeeded()) {
        return Fail("producer playback batch count snapshot failed");
    }

    if (count_result.required_sampled_transform_value_count != sampled_value_count) {
        return Fail("producer playback batch count required count mismatch");
    }

    if (count_result.state.sampled_transform_value_count != sampled_value_count) {
        return Fail("producer playback batch count state mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("producer playback batch count mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback batch count mutated values");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 2U) {
        return Fail("producer playback batch count attempt count mismatch");
    }

    if (adapter_snapshot.required_sampled_transform_value_count != sampled_value_count_snapshot) {
        return Fail("producer playback batch count snapshot required count mismatch");
    }

    if (adapter_snapshot.sampled_transform_value_count != sampled_value_count_snapshot) {
        return Fail("producer playback batch count sampled counter mismatch");
    }

    return 0;
}

int TestRejectTimelineSamplerOutputFailureWithoutMutation() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    WorldTransformBridge transform_destination(world);
    std::array<AnimationRuntimeClipRecord, 1U> clips{};
    clips[0U] = ClipRecord(3U);
    std::array<AnimationRuntimeTrackRecord, 3U> tracks{};
    tracks[0U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 0U);
    tracks[1U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 2U);
    tracks[2U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 4U);
    std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{};
    keyframes[0U] = Keyframe(0.0F, 3.0F);
    keyframes[1U] = Keyframe(1.0F, 4.0F);
    keyframes[2U] = Keyframe(0.0F, 40.0F);
    keyframes[3U] = Keyframe(1.0F, 42.0F);
    keyframes[4U] = Keyframe(0.0F, 0.0F);
    keyframes[5U] = Keyframe(1.0F, 0.5F);

    std::array<AnimationRuntimeSampledValue, 3U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const std::uint64_t sampled_value_count_snapshot = static_cast<std::uint64_t>(sampled_value_count);
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest full_capacity_request =
        MakeTimelineTransformSampleRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult success_result =
        adapter_bridge.BuildTimelineTransformApplicationRequest(
            full_capacity_request,
            &transform_application_request);
    if (!success_result.Succeeded()) {
        return Fail("timeline sampler output atomic setup failed");
    }

    const std::array<AnimationRuntimeSampledValue, 3U> previous_values = sampled_values;
    const RuntimeAssetWorldObjectTransformApplicationRequest previous_request = transform_application_request;
    constexpr std::uint32_t SMALL_OUTPUT_CAPACITY = 1U;
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest small_output_request =
        MakeTimelineTransformSampleRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            SMALL_OUTPUT_CAPACITY);

    const RuntimeAssetWorldObjectAdapterResult capacity_result =
        adapter_bridge.BuildTimelineTransformApplicationRequest(
            small_output_request,
            &transform_application_request);
    if (capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("timeline sampler output capacity status mismatch");
    }

    if (capacity_result.required_sampled_transform_value_count != sampled_value_count) {
        return Fail("timeline sampler output capacity required count mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("timeline sampler output capacity mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("timeline sampler output capacity mutated values");
    }

    const RuntimeAssetWorldObjectAdapterResult repeated_capacity_result =
        adapter_bridge.BuildTimelineTransformApplicationRequest(
            small_output_request,
            &transform_application_request);
    if (repeated_capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("timeline sampler repeated output capacity status mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("timeline sampler repeated output capacity mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("timeline sampler repeated output capacity mutated values");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 3U) {
        return Fail("timeline sampler output atomic attempt count mismatch");
    }

    if (adapter_snapshot.failed_operation_count != 2U) {
        return Fail("timeline sampler output atomic failure count mismatch");
    }

    if (adapter_snapshot.required_sampled_transform_value_count != sampled_value_count_snapshot) {
        return Fail("timeline sampler output atomic snapshot required count mismatch");
    }

    return 0;
}

int TestRejectTimelineTransformApplicationCountFailureWithoutMutation() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    WorldTransformBridge transform_destination(world);
    std::array<AnimationRuntimeClipRecord, 1U> clips{};
    clips[0U] = ClipRecord(3U);
    std::array<AnimationRuntimeTrackRecord, 3U> tracks{};
    tracks[0U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 0U);
    tracks[1U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 2U);
    tracks[2U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 4U);
    std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{};
    keyframes[0U] = Keyframe(0.0F, 3.0F);
    keyframes[1U] = Keyframe(1.0F, 4.0F);
    keyframes[2U] = Keyframe(0.0F, 40.0F);
    keyframes[3U] = Keyframe(1.0F, 42.0F);
    keyframes[4U] = Keyframe(0.0F, 0.0F);
    keyframes[5U] = Keyframe(1.0F, 0.5F);

    std::array<AnimationRuntimeSampledValue, 3U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest full_capacity_request =
        MakeTimelineTransformSampleRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult success_result =
        adapter_bridge.BuildTimelineTransformApplicationRequest(
            full_capacity_request,
            &transform_application_request);
    if (!success_result.Succeeded()) {
        return Fail("timeline count failure setup failed");
    }

    const std::array<AnimationRuntimeSampledValue, 3U> previous_values = sampled_values;
    const RuntimeAssetWorldObjectTransformApplicationRequest previous_request = transform_application_request;
    RuntimeAssetWorldObjectTimelineTransformSampleRequest invalid_clip_request = full_capacity_request;
    invalid_clip_request.clip_id = 0U;
    const RuntimeAssetWorldObjectAdapterResult invalid_clip_result =
        adapter_bridge.SnapshotTimelineTransformApplicationCount(invalid_clip_request);
    if (invalid_clip_result.status != RuntimeAssetWorldObjectAdapterStatus::TransformSamplingFailed) {
        return Fail("timeline count invalid clip status mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("timeline count invalid clip mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("timeline count invalid clip mutated values");
    }

    constexpr std::uint32_t SMALL_SCRATCH_CAPACITY = 1U;
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest small_scratch_request =
        MakeTimelineTransformSampleRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            SMALL_SCRATCH_CAPACITY,
            sampled_values.data(),
            sampled_value_count);
    const RuntimeAssetWorldObjectAdapterResult scratch_capacity_result =
        adapter_bridge.SnapshotTimelineTransformApplicationCount(small_scratch_request);
    if (scratch_capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("timeline count scratch capacity status mismatch");
    }

    if (scratch_capacity_result.required_sampled_transform_value_count != sampled_value_count) {
        return Fail("timeline count scratch capacity required mismatch");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("timeline count scratch capacity mutated values");
    }

    constexpr std::uint32_t SMALL_OUTPUT_CAPACITY = 1U;
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest small_output_request =
        MakeTimelineTransformSampleRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            SMALL_OUTPUT_CAPACITY);
    const RuntimeAssetWorldObjectAdapterResult output_capacity_result =
        adapter_bridge.SnapshotTimelineTransformApplicationCount(small_output_request);
    if (output_capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("timeline count output capacity status mismatch");
    }

    if (output_capacity_result.required_sampled_transform_value_count != sampled_value_count) {
        return Fail("timeline count output capacity required mismatch");
    }

    const RuntimeAssetWorldObjectAdapterResult repeated_output_result =
        adapter_bridge.SnapshotTimelineTransformApplicationCount(small_output_request);
    if (repeated_output_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("timeline count repeated output status mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("timeline count repeated output mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("timeline count repeated output mutated values");
    }

    RuntimeAssetWorldObjectTimelineTransformSampleRequest null_scratch_request = full_capacity_request;
    null_scratch_request.sampled_value_scratch = nullptr;
    const RuntimeAssetWorldObjectAdapterResult null_scratch_result =
        adapter_bridge.SnapshotTimelineTransformApplicationCount(null_scratch_request);
    if (null_scratch_result.status != RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput) {
        return Fail("timeline count null scratch status mismatch");
    }

    std::array<AnimationRuntimeTrackRecord, 3U> unmapped_tracks = tracks;
    unmapped_tracks[0U] = TrackRecord(WORLD_OBJECT_UNMAPPED, AnimationRuntimeChannel::ScaleZ, 0U);
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest unmapped_target_request =
        MakeTimelineTransformSampleRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(unmapped_tracks.data(), unmapped_tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);
    const RuntimeAssetWorldObjectAdapterResult unmapped_target_result =
        adapter_bridge.SnapshotTimelineTransformApplicationCount(unmapped_target_request);
    if (unmapped_target_result.status != RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound) {
        return Fail("timeline count target status mismatch");
    }

    if (unmapped_target_result.failed_target_id != WORLD_OBJECT_UNMAPPED.value) {
        return Fail("timeline count target diagnostic mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("timeline count target mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("timeline count target mutated values");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 7U) {
        return Fail("timeline count failure attempt count mismatch");
    }

    if (adapter_snapshot.failed_operation_count != 6U) {
        return Fail("timeline count failure count mismatch");
    }

    if (adapter_snapshot.required_sampled_transform_value_count != 0U) {
        return Fail("timeline count final required count mismatch");
    }

    return 0;
}

int TestRejectProducerPlaybackFailureWithoutMutation() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    WorldTransformBridge transform_destination(world);
    std::array<AnimationRuntimeClipRecord, 1U> clips{};
    clips[0U] = ClipRecord(3U);
    std::array<AnimationRuntimeTrackRecord, 3U> tracks{};
    tracks[0U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 0U);
    tracks[1U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 2U);
    tracks[2U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 4U);
    std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{};
    keyframes[0U] = Keyframe(0.0F, 3.0F);
    keyframes[1U] = Keyframe(1.0F, 4.0F);
    keyframes[2U] = Keyframe(0.0F, 40.0F);
    keyframes[3U] = Keyframe(1.0F, 42.0F);
    keyframes[4U] = Keyframe(0.0F, 0.0F);
    keyframes[5U] = Keyframe(1.0F, 0.5F);

    std::array<AnimationRuntimeSampledValue, 3U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const RuntimeAssetWorldObjectProducerPlaybackRequest full_capacity_request =
        MakeProducerPlaybackRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult success_result =
        adapter_bridge.BuildProducerPlaybackTransformApplicationRequest(
            full_capacity_request,
            &transform_application_request);
    if (!success_result.Succeeded()) {
        return Fail("producer playback atomic setup failed");
    }

    const std::array<AnimationRuntimeSampledValue, 3U> previous_values = sampled_values;
    const RuntimeAssetWorldObjectTransformApplicationRequest previous_request = transform_application_request;
    constexpr std::uint32_t SMALL_CAPACITY = 1U;
    const RuntimeAssetWorldObjectProducerPlaybackRequest small_output_request =
        MakeProducerPlaybackRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            SMALL_CAPACITY);

    const RuntimeAssetWorldObjectAdapterResult output_capacity_result =
        adapter_bridge.BuildProducerPlaybackTransformApplicationRequest(
            small_output_request,
            &transform_application_request);
    if (output_capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("producer playback output capacity status mismatch");
    }

    if (output_capacity_result.required_sampled_transform_value_count != sampled_value_count) {
        return Fail("producer playback output capacity required count mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("producer playback output capacity mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback output capacity mutated values");
    }

    const RuntimeAssetWorldObjectProducerPlaybackRequest small_scratch_request =
        MakeProducerPlaybackRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            SMALL_CAPACITY,
            sampled_values.data(),
            SMALL_CAPACITY);

    const RuntimeAssetWorldObjectAdapterResult scratch_capacity_result =
        adapter_bridge.BuildProducerPlaybackTransformApplicationRequest(
            small_scratch_request,
            &transform_application_request);
    if (scratch_capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("producer playback scratch capacity status mismatch");
    }

    if (scratch_capacity_result.required_sampled_transform_value_count != sampled_value_count) {
        return Fail("producer playback scratch capacity required count mismatch");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback scratch capacity mutated values");
    }

    const RuntimeAssetWorldObjectAdapterResult repeated_capacity_result =
        adapter_bridge.BuildProducerPlaybackTransformApplicationRequest(
            small_output_request,
            &transform_application_request);
    if (repeated_capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("producer playback repeated capacity status mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("producer playback repeated capacity mutated request");
    }

    RuntimeAssetWorldObjectProducerPlaybackRequest default_clip_request = full_capacity_request;
    default_clip_request.export_clip_id = 0U;
    const RuntimeAssetWorldObjectAdapterResult default_clip_result =
        adapter_bridge.BuildProducerPlaybackTransformApplicationRequest(
            default_clip_request,
            &transform_application_request);
    if (default_clip_result.status != RuntimeAssetWorldObjectAdapterStatus::TransformSamplingFailed) {
        return Fail("producer playback default clip status mismatch");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback default clip mutated values");
    }

    tracks[0U] = TrackRecord(WORLD_OBJECT_UNMAPPED, AnimationRuntimeChannel::TranslationX, 0U);
    const RuntimeAssetWorldObjectProducerPlaybackRequest unmapped_target_request =
        MakeProducerPlaybackRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);
    const RuntimeAssetWorldObjectAdapterResult unmapped_target_result =
        adapter_bridge.BuildProducerPlaybackTransformApplicationRequest(
            unmapped_target_request,
            &transform_application_request);
    if (unmapped_target_result.status != RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound) {
        return Fail("producer playback unmapped target status mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("producer playback target failure mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback target failure mutated values");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 6U) {
        return Fail("producer playback atomic attempt count mismatch");
    }

    if (adapter_snapshot.failed_operation_count != 5U) {
        return Fail("producer playback atomic failure count mismatch");
    }

    if (adapter_snapshot.required_sampled_transform_value_count != 0U) {
        return Fail("producer playback final required count mismatch");
    }

    return 0;
}

int TestRejectProducerPlaybackBatchFailureWithoutMutation() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    WorldTransformBridge transform_destination(world);
    std::array<AnimationRuntimeClipRecord, 1U> first_clips{};
    first_clips[0U] = ClipRecord(2U);
    std::array<AnimationRuntimeTrackRecord, 2U> first_tracks{};
    first_tracks[0U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 0U);
    first_tracks[1U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 2U);
    std::array<AnimationRuntimeKeyframeRecord, 4U> first_keyframes{};
    first_keyframes[0U] = Keyframe(0.0F, 40.0F);
    first_keyframes[1U] = Keyframe(1.0F, 42.0F);
    first_keyframes[2U] = Keyframe(0.0F, 3.0F);
    first_keyframes[3U] = Keyframe(1.0F, 4.0F);

    std::array<AnimationRuntimeClipRecord, 1U> second_clips{};
    second_clips[0U] = ClipRecord(1U);
    std::array<AnimationRuntimeTrackRecord, 1U> second_tracks{};
    second_tracks[0U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 0U);
    std::array<AnimationRuntimeKeyframeRecord, 2U> second_keyframes{};
    second_keyframes[0U] = Keyframe(0.0F, 0.0F);
    second_keyframes[1U] = Keyframe(1.0F, 0.5F);

    std::array<RuntimeAssetWorldObjectProducerPlaybackRequest, 2U> playback_requests{};
    playback_requests[0U] = MakeProducerPlaybackRequest(
        &fixture,
        &transform_destination,
        std::span<const AnimationRuntimeClipRecord>(first_clips.data(), first_clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(first_tracks.data(), first_tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(first_keyframes.data(), first_keyframes.size()),
        nullptr,
        0U,
        nullptr,
        0U);
    playback_requests[1U] = MakeProducerPlaybackRequest(
        &fixture,
        &transform_destination,
        std::span<const AnimationRuntimeClipRecord>(second_clips.data(), second_clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(second_tracks.data(), second_tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(second_keyframes.data(), second_keyframes.size()),
        nullptr,
        0U,
        nullptr,
        0U);

    std::array<AnimationRuntimeSampledValue, 3U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const std::uint32_t playback_request_count = static_cast<std::uint32_t>(playback_requests.size());
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest full_capacity_request =
        MakeProducerPlaybackBatchRequest(
            playback_requests.data(),
            playback_request_count,
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult success_result =
        adapter_bridge.BuildProducerPlaybackBatchTransformApplicationRequest(
            full_capacity_request,
            &transform_application_request);
    if (!success_result.Succeeded()) {
        return Fail("producer playback batch atomic setup failed");
    }

    const std::array<AnimationRuntimeSampledValue, 3U> previous_values = sampled_values;
    const RuntimeAssetWorldObjectTransformApplicationRequest previous_request = transform_application_request;
    std::array<RuntimeAssetWorldObjectProducerPlaybackRequest, 2U> invalid_clip_requests = playback_requests;
    invalid_clip_requests[1U].export_clip_id = 0U;
    RuntimeAssetWorldObjectProducerPlaybackBatchRequest invalid_clip_request =
        MakeProducerPlaybackBatchRequest(
            invalid_clip_requests.data(),
            playback_request_count,
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);
    const RuntimeAssetWorldObjectAdapterResult invalid_clip_result =
        adapter_bridge.BuildProducerPlaybackBatchTransformApplicationRequest(
            invalid_clip_request,
            &transform_application_request);
    if (invalid_clip_result.status != RuntimeAssetWorldObjectAdapterStatus::TransformSamplingFailed) {
        return Fail("producer playback batch invalid clip status mismatch");
    }

    if (invalid_clip_result.failed_playback_request_index != 1U) {
        return Fail("producer playback batch invalid clip index mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("producer playback batch invalid clip mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback batch invalid clip mutated values");
    }

    constexpr std::uint32_t SMALL_CAPACITY = 2U;
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest small_scratch_request =
        MakeProducerPlaybackBatchRequest(
            playback_requests.data(),
            playback_request_count,
            sampled_scratch.data(),
            SMALL_CAPACITY,
            sampled_values.data(),
            sampled_value_count);
    const RuntimeAssetWorldObjectAdapterResult scratch_capacity_result =
        adapter_bridge.BuildProducerPlaybackBatchTransformApplicationRequest(
            small_scratch_request,
            &transform_application_request);
    if (scratch_capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("producer playback batch scratch capacity status mismatch");
    }

    if (scratch_capacity_result.failed_playback_request_index != 1U) {
        return Fail("producer playback batch scratch capacity index mismatch");
    }

    if (scratch_capacity_result.required_sampled_transform_value_count != sampled_value_count) {
        return Fail("producer playback batch scratch capacity required count mismatch");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback batch scratch capacity mutated values");
    }

    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest small_output_request =
        MakeProducerPlaybackBatchRequest(
            playback_requests.data(),
            playback_request_count,
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            SMALL_CAPACITY);
    const RuntimeAssetWorldObjectAdapterResult output_capacity_result =
        adapter_bridge.BuildProducerPlaybackBatchTransformApplicationRequest(
            small_output_request,
            &transform_application_request);
    if (output_capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("producer playback batch output capacity status mismatch");
    }

    if (output_capacity_result.failed_playback_request_index != 1U) {
        return Fail("producer playback batch output capacity index mismatch");
    }

    if (output_capacity_result.required_sampled_transform_value_count != sampled_value_count) {
        return Fail("producer playback batch output capacity required count mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("producer playback batch output capacity mutated request");
    }

    const RuntimeAssetWorldObjectAdapterResult repeated_output_capacity_result =
        adapter_bridge.BuildProducerPlaybackBatchTransformApplicationRequest(
            small_output_request,
            &transform_application_request);
    if (repeated_output_capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("producer playback batch repeated output capacity status mismatch");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback batch repeated output capacity mutated values");
    }

    std::array<AnimationRuntimeTrackRecord, 1U> unmapped_tracks = second_tracks;
    unmapped_tracks[0U] = TrackRecord(WORLD_OBJECT_UNMAPPED, AnimationRuntimeChannel::RotationY, 0U);
    std::array<RuntimeAssetWorldObjectProducerPlaybackRequest, 2U> unmapped_target_requests = playback_requests;
    unmapped_target_requests[1U] = MakeProducerPlaybackRequest(
        &fixture,
        &transform_destination,
        std::span<const AnimationRuntimeClipRecord>(second_clips.data(), second_clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(unmapped_tracks.data(), unmapped_tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(second_keyframes.data(), second_keyframes.size()),
        nullptr,
        0U,
        nullptr,
        0U);
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest unmapped_target_request =
        MakeProducerPlaybackBatchRequest(
            unmapped_target_requests.data(),
            playback_request_count,
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);
    const RuntimeAssetWorldObjectAdapterResult unmapped_target_result =
        adapter_bridge.BuildProducerPlaybackBatchTransformApplicationRequest(
            unmapped_target_request,
            &transform_application_request);
    if (unmapped_target_result.status != RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound) {
        return Fail("producer playback batch target status mismatch");
    }

    if (unmapped_target_result.failed_playback_request_index != 1U) {
        return Fail("producer playback batch target index mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("producer playback batch target failure mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback batch target failure mutated values");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 6U) {
        return Fail("producer playback batch atomic attempt count mismatch");
    }

    if (adapter_snapshot.failed_operation_count != 5U) {
        return Fail("producer playback batch atomic failure count mismatch");
    }

    if (adapter_snapshot.required_sampled_transform_value_count != 0U) {
        return Fail("producer playback batch final required count mismatch");
    }

    return 0;
}

int TestRejectProducerPlaybackBatchCountFailureWithoutMutation() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    WorldTransformBridge transform_destination(world);
    std::array<AnimationRuntimeClipRecord, 1U> first_clips{};
    first_clips[0U] = ClipRecord(2U);
    std::array<AnimationRuntimeTrackRecord, 2U> first_tracks{};
    first_tracks[0U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::TranslationX, 0U);
    first_tracks[1U] = TrackRecord(WORLD_OBJECT_B, AnimationRuntimeChannel::ScaleZ, 2U);
    std::array<AnimationRuntimeKeyframeRecord, 4U> first_keyframes{};
    first_keyframes[0U] = Keyframe(0.0F, 40.0F);
    first_keyframes[1U] = Keyframe(1.0F, 42.0F);
    first_keyframes[2U] = Keyframe(0.0F, 3.0F);
    first_keyframes[3U] = Keyframe(1.0F, 4.0F);

    std::array<AnimationRuntimeClipRecord, 1U> second_clips{};
    second_clips[0U] = ClipRecord(1U);
    std::array<AnimationRuntimeTrackRecord, 1U> second_tracks{};
    second_tracks[0U] = TrackRecord(WORLD_OBJECT_A, AnimationRuntimeChannel::RotationY, 0U);
    std::array<AnimationRuntimeKeyframeRecord, 2U> second_keyframes{};
    second_keyframes[0U] = Keyframe(0.0F, 0.0F);
    second_keyframes[1U] = Keyframe(1.0F, 0.5F);

    std::array<RuntimeAssetWorldObjectProducerPlaybackRequest, 2U> playback_requests{};
    playback_requests[0U] = MakeProducerPlaybackRequest(
        &fixture,
        &transform_destination,
        std::span<const AnimationRuntimeClipRecord>(first_clips.data(), first_clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(first_tracks.data(), first_tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(first_keyframes.data(), first_keyframes.size()),
        nullptr,
        0U,
        nullptr,
        0U);
    playback_requests[1U] = MakeProducerPlaybackRequest(
        &fixture,
        &transform_destination,
        std::span<const AnimationRuntimeClipRecord>(second_clips.data(), second_clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(second_tracks.data(), second_tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(second_keyframes.data(), second_keyframes.size()),
        nullptr,
        0U,
        nullptr,
        0U);

    std::array<AnimationRuntimeSampledValue, 3U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 3U> sampled_values{};
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const std::uint32_t playback_request_count = static_cast<std::uint32_t>(playback_requests.size());
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest full_capacity_request =
        MakeProducerPlaybackBatchRequest(
            playback_requests.data(),
            playback_request_count,
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult success_result =
        adapter_bridge.BuildProducerPlaybackBatchTransformApplicationRequest(
            full_capacity_request,
            &transform_application_request);
    if (!success_result.Succeeded()) {
        return Fail("producer playback batch count failure setup failed");
    }

    const std::array<AnimationRuntimeSampledValue, 3U> previous_values = sampled_values;
    const RuntimeAssetWorldObjectTransformApplicationRequest previous_request = transform_application_request;
    std::array<RuntimeAssetWorldObjectProducerPlaybackRequest, 2U> invalid_clip_requests = playback_requests;
    invalid_clip_requests[1U].export_clip_id = 0U;
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest invalid_clip_request =
        MakeProducerPlaybackBatchRequest(
            invalid_clip_requests.data(),
            playback_request_count,
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);
    const RuntimeAssetWorldObjectAdapterResult invalid_clip_result =
        adapter_bridge.SnapshotProducerPlaybackBatchTransformApplicationCount(invalid_clip_request);
    if (invalid_clip_result.status != RuntimeAssetWorldObjectAdapterStatus::TransformSamplingFailed) {
        return Fail("producer playback batch count invalid clip status mismatch");
    }

    if (invalid_clip_result.failed_playback_request_index != 1U) {
        return Fail("producer playback batch count invalid clip index mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("producer playback batch count invalid clip mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback batch count invalid clip mutated values");
    }

    constexpr std::uint32_t SMALL_CAPACITY = 2U;
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest small_scratch_request =
        MakeProducerPlaybackBatchRequest(
            playback_requests.data(),
            playback_request_count,
            sampled_scratch.data(),
            SMALL_CAPACITY,
            sampled_values.data(),
            sampled_value_count);
    const RuntimeAssetWorldObjectAdapterResult scratch_capacity_result =
        adapter_bridge.SnapshotProducerPlaybackBatchTransformApplicationCount(small_scratch_request);
    if (scratch_capacity_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("producer playback batch count scratch capacity status mismatch");
    }

    if (scratch_capacity_result.failed_playback_request_index != 1U) {
        return Fail("producer playback batch count scratch capacity index mismatch");
    }

    if (scratch_capacity_result.required_sampled_transform_value_count != sampled_value_count) {
        return Fail("producer playback batch count scratch capacity required mismatch");
    }

    const RuntimeAssetWorldObjectAdapterResult repeated_scratch_result =
        adapter_bridge.SnapshotProducerPlaybackBatchTransformApplicationCount(small_scratch_request);
    if (repeated_scratch_result.status != RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded) {
        return Fail("producer playback batch count repeated scratch status mismatch");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback batch count repeated scratch mutated values");
    }

    std::array<AnimationRuntimeTrackRecord, 1U> unmapped_tracks = second_tracks;
    unmapped_tracks[0U] = TrackRecord(WORLD_OBJECT_UNMAPPED, AnimationRuntimeChannel::RotationY, 0U);
    std::array<RuntimeAssetWorldObjectProducerPlaybackRequest, 2U> unmapped_target_requests = playback_requests;
    unmapped_target_requests[1U] = MakeProducerPlaybackRequest(
        &fixture,
        &transform_destination,
        std::span<const AnimationRuntimeClipRecord>(second_clips.data(), second_clips.size()),
        std::span<const AnimationRuntimeTrackRecord>(unmapped_tracks.data(), unmapped_tracks.size()),
        std::span<const AnimationRuntimeKeyframeRecord>(second_keyframes.data(), second_keyframes.size()),
        nullptr,
        0U,
        nullptr,
        0U);
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest unmapped_target_request =
        MakeProducerPlaybackBatchRequest(
            unmapped_target_requests.data(),
            playback_request_count,
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);
    const RuntimeAssetWorldObjectAdapterResult unmapped_target_result =
        adapter_bridge.SnapshotProducerPlaybackBatchTransformApplicationCount(unmapped_target_request);
    if (unmapped_target_result.status != RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound) {
        return Fail("producer playback batch count target status mismatch");
    }

    if (unmapped_target_result.failed_playback_request_index != 1U) {
        return Fail("producer playback batch count target index mismatch");
    }

    if (transform_application_request.sampled_values != previous_request.sampled_values ||
        transform_application_request.sampled_value_count != previous_request.sampled_value_count) {
        return Fail("producer playback batch count target mutated request");
    }

    if (!SampledValueArraysMatch(
            std::span<const AnimationRuntimeSampledValue>(sampled_values.data(), sampled_values.size()),
            std::span<const AnimationRuntimeSampledValue>(previous_values.data(), previous_values.size()))) {
        return Fail("producer playback batch count target mutated values");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 5U) {
        return Fail("producer playback batch count failure attempt count mismatch");
    }

    if (adapter_snapshot.failed_operation_count != 4U) {
        return Fail("producer playback batch count failure count mismatch");
    }

    if (adapter_snapshot.required_sampled_transform_value_count != 0U) {
        return Fail("producer playback batch count final required count mismatch");
    }

    return 0;
}

int TestRejectTimelineSampleTargetsBeforeWorldRestore() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    if (RegisterWorldObject(&world, WORLD_OBJECT_UNMAPPED) != 0) {
        return 1;
    }

    WorldObjectIdentityBridge identity_destination(world, object_registry);
    WorldTransformBridge transform_destination(world);
    const WorldTransformState unmapped_transform = Transform(70.0F);
    if (RegisterTransform(&transform_destination, WORLD_OBJECT_UNMAPPED, unmapped_transform) != 0) {
        return 1;
    }

    const WorldObjectIdentitySnapshot identity_before = identity_destination.Snapshot();
    const WorldTransformSnapshot transform_before = transform_destination.Snapshot();
    std::array<AnimationRuntimeClipRecord, 1U> clips{};
    clips[0U] = ClipRecord(1U);
    std::array<AnimationRuntimeTrackRecord, 1U> tracks{};
    tracks[0U] = TrackRecord(WorldObjectId{}, AnimationRuntimeChannel::TranslationX, 0U);
    std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{};
    keyframes[0U] = Keyframe(0.0F, 90.0F);
    keyframes[1U] = Keyframe(1.0F, 100.0F);
    std::array<AnimationRuntimeSampledValue, 1U> sampled_scratch{};
    std::array<AnimationRuntimeSampledValue, 1U> sampled_values{};
    sampled_values[0U].target = SENTINEL_WORLD_OBJECT;
    sampled_values[0U].channel = AnimationRuntimeChannel::ScaleZ;
    sampled_values[0U].value = 123.0F;
    const std::uint32_t sampled_value_count = static_cast<std::uint32_t>(sampled_values.size());
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest default_target_request =
        MakeTimelineTransformSampleRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request{};
    const RuntimeAssetWorldObjectAdapterResult default_result =
        adapter_bridge.BuildTimelineTransformApplicationRequest(
            default_target_request,
            &transform_application_request);
    if (default_result.status != RuntimeAssetWorldObjectAdapterStatus::TransformSamplingFailed) {
        return Fail("timeline sampler default target status mismatch");
    }

    if (transform_application_request.sampled_value_count != 0U) {
        return Fail("timeline sampler default target exposed request");
    }

    if (sampled_values[0U].target.value != SENTINEL_WORLD_OBJECT.value ||
        sampled_values[0U].channel != AnimationRuntimeChannel::ScaleZ ||
        sampled_values[0U].value != 123.0F) {
        return Fail("timeline sampler default target mutated sampled output");
    }

    tracks[0U] = TrackRecord(WORLD_OBJECT_UNMAPPED, AnimationRuntimeChannel::TranslationX, 0U);
    sampled_values[0U].target = SENTINEL_WORLD_OBJECT;
    sampled_values[0U].channel = AnimationRuntimeChannel::ScaleZ;
    sampled_values[0U].value = 123.0F;
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest unmapped_target_request =
        MakeTimelineTransformSampleRequest(
            &fixture,
            &transform_destination,
            std::span<const AnimationRuntimeClipRecord>(clips.data(), clips.size()),
            std::span<const AnimationRuntimeTrackRecord>(tracks.data(), tracks.size()),
            std::span<const AnimationRuntimeKeyframeRecord>(keyframes.data(), keyframes.size()),
            sampled_scratch.data(),
            sampled_value_count,
            sampled_values.data(),
            sampled_value_count);
    const RuntimeAssetWorldObjectAdapterResult unmapped_result =
        adapter_bridge.BuildTimelineTransformApplicationRequest(
            unmapped_target_request,
            &transform_application_request);
    if (unmapped_result.status != RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound) {
        return Fail("timeline sampler unmapped target status mismatch");
    }

    if (transform_application_request.sampled_value_count != 0U) {
        return Fail("timeline sampler unmapped target exposed request");
    }

    if (sampled_values[0U].target.value != SENTINEL_WORLD_OBJECT.value ||
        sampled_values[0U].channel != AnimationRuntimeChannel::ScaleZ ||
        sampled_values[0U].value != 123.0F) {
        return Fail("timeline sampler unmapped target mutated sampled output");
    }

    if (sampled_scratch[0U].target.value != WORLD_OBJECT_UNMAPPED.value ||
        sampled_scratch[0U].channel != AnimationRuntimeChannel::TranslationX ||
        sampled_scratch[0U].value != 95.0F) {
        return Fail("timeline sampler unmapped target scratch value mismatch");
    }

    if (!ObjectIdentitySnapshotsMatch(identity_before, identity_destination.Snapshot())) {
        return Fail("timeline sampler target failure mutated identity destination");
    }

    if (!TransformSnapshotsMatch(transform_before, transform_destination.Snapshot())) {
        return Fail("timeline sampler target failure mutated transform destination");
    }

    const WorldTransformResult unmapped_query = transform_destination.Query(WORLD_OBJECT_UNMAPPED);
    if (!unmapped_query.Succeeded()) {
        return Fail("timeline sampler unmapped target query failed");
    }

    if (!TransformsMatch(unmapped_query.transform_state, unmapped_transform)) {
        return Fail("timeline sampler unmapped target mutated existing target");
    }

    const auto adapter_snapshot = adapter_bridge.Snapshot();
    if (adapter_snapshot.transform_sampler_bridge_attempt_count != 2U) {
        return Fail("timeline sampler target failure attempt count mismatch");
    }

    if (adapter_snapshot.failed_operation_count != 2U) {
        return Fail("timeline sampler target failure count mismatch");
    }

    if (adapter_snapshot.last_status != RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound) {
        return Fail("timeline sampler target failure snapshot status mismatch");
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

int TestRejectUnmappedSampledTransformBeforeWorldRestore() {
    HandoffFixture fixture = MakeFixture();
    WorldInstance world = MakeWorld();
    ObjectRegistry object_registry = MakeObjectRegistry();
    if (PrepareWorldAndObjects(&world, &object_registry, &fixture, true) != 0) {
        return 1;
    }

    if (RegisterWorldObject(&world, WORLD_OBJECT_UNMAPPED) != 0) {
        return 1;
    }

    ResourceRegistry resource_registry{};
    WorldObjectIdentityBridge identity_destination(world, object_registry);
    WorldTransformBridge transform_destination(world);
    WorldComponentAttachmentBridge attachment_destination{};
    WorldComponentResourceBindingBridge binding_destination{};
    const WorldTransformState unmapped_transform = Transform(70.0F);
    if (RegisterTransform(&transform_destination, WORLD_OBJECT_UNMAPPED, unmapped_transform) != 0) {
        return 1;
    }

    const WorldObjectIdentitySnapshot identity_before = identity_destination.Snapshot();
    const WorldTransformSnapshot transform_before = transform_destination.Snapshot();
    std::array<AnimationRuntimeSampledValue, 1U> sampled_values{};
    sampled_values[0U].target = WORLD_OBJECT_UNMAPPED;
    sampled_values[0U].channel = AnimationRuntimeChannel::TranslationX;
    sampled_values[0U].value = 99.0F;
    RuntimeAssetWorldObjectTransformApplicationRequest transform_application_request =
        MakeTransformApplicationRequest(
            &fixture,
            &transform_destination,
            sampled_values.data(),
            static_cast<std::uint32_t>(sampled_values.size()));
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
    handoff_request.transform_application_request = &transform_application_request;

    RuntimeAssetWorldObjectRestoreHandoffBridge bridge{};
    const RuntimeAssetWorldObjectRestoreHandoffResult result = bridge.ApplyRestore(handoff_request);
    if (result.status != RuntimeAssetWorldObjectRestoreHandoffStatus::TransformApplicationFailed) {
        return Fail("unmapped sampled transform handoff status mismatch");
    }

    if (result.adapter_status != RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound) {
        return Fail("unmapped sampled transform adapter status mismatch");
    }

    if (!OutputRecordsHaveSentinelValues(fixture)) {
        return Fail("unmapped sampled transform mutated output records");
    }

    if (!ObjectIdentitySnapshotsMatch(identity_before, identity_destination.Snapshot())) {
        return Fail("unmapped sampled transform mutated identity destination");
    }

    if (!TransformSnapshotsMatch(transform_before, transform_destination.Snapshot())) {
        return Fail("unmapped sampled transform mutated transform destination");
    }

    const WorldTransformResult unmapped_result = transform_destination.Query(WORLD_OBJECT_UNMAPPED);
    if (!unmapped_result.Succeeded()) {
        return Fail("unmapped sampled transform query failed");
    }

    if (!TransformsMatch(unmapped_result.transform_state, unmapped_transform)) {
        return Fail("unmapped sampled transform mutated existing target");
    }

    const RuntimeAssetWorldObjectRestoreHandoffSnapshot snapshot = bridge.Snapshot();
    if (snapshot.failed_operation_count != 1U) {
        return Fail("unmapped sampled transform snapshot failure count mismatch");
    }

    if (snapshot.rejected_operation_count != 1U) {
        return Fail("unmapped sampled transform snapshot rejection mismatch");
    }

    if (snapshot.last_status != RuntimeAssetWorldObjectRestoreHandoffStatus::TransformApplicationFailed) {
        return Fail("unmapped sampled transform snapshot handoff status mismatch");
    }

    if (snapshot.last_adapter_status != RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound) {
        return Fail("unmapped sampled transform snapshot adapter status mismatch");
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
    constexpr std::array<TestCase, 22U> TESTS{{
        {TEST_APPLY_RESTORE, TestApplyRestore},
        {TEST_APPLY_SAMPLED_TRANSFORMS, TestApplySampledTransformsThroughAdapterBridge},
        {TEST_APPLY_RUNTIME_ANIMATION_SAMPLES, TestApplyRuntimeAnimationSamplesThroughSamplerBridge},
        {TEST_APPLY_TIMELINE_SAMPLES, TestApplyTimelineSamplesThroughSamplerRequest},
        {TEST_SNAPSHOT_TIMELINE_APPLICATION_COUNT, TestSnapshotTimelineTransformApplicationCountWithoutMutation},
        {TEST_APPLY_PRODUCER_PLAYBACK, TestApplyProducerPlaybackThroughScratchHandoff},
        {TEST_APPLY_PRODUCER_PLAYBACK_BATCH, TestApplyProducerPlaybackBatchThroughScratchHandoff},
        {TEST_SNAPSHOT_PRODUCER_PLAYBACK_BATCH_COUNT, TestSnapshotProducerPlaybackBatchCountWithoutMutation},
        {TEST_APPLY_TARGET_FAMILY_ALIASES, TestApplyTargetFamilyAliases},
        {TEST_APPLY_ATTACHMENT_BINDING_GATE_RECORDS, TestApplyAttachmentAndBindingGateRecords},
        {TEST_APPLY_SIDECAR_ASSEMBLY_RESTORE, TestApplySidecarAssemblyRestore},
        {TEST_REJECT_SIDECAR_ASSEMBLY_FAILURE_STATUS, TestRejectSidecarAssemblyFailureStatus},
        {TEST_REJECT_ADAPTER_PREFLIGHT, TestRejectAdapterPreflight},
        {TEST_REJECT_UNMAPPED_SAMPLED_TRANSFORM, TestRejectUnmappedSampledTransformBeforeWorldRestore},
        {TEST_REJECT_TIMELINE_OUTPUT_ATOMIC, TestRejectTimelineSamplerOutputFailureWithoutMutation},
        {TEST_REJECT_TIMELINE_APPLICATION_COUNT, TestRejectTimelineTransformApplicationCountFailureWithoutMutation},
        {TEST_REJECT_PRODUCER_PLAYBACK_ATOMIC, TestRejectProducerPlaybackFailureWithoutMutation},
        {TEST_REJECT_PRODUCER_PLAYBACK_BATCH_ATOMIC, TestRejectProducerPlaybackBatchFailureWithoutMutation},
        {TEST_REJECT_PRODUCER_PLAYBACK_BATCH_COUNT, TestRejectProducerPlaybackBatchCountFailureWithoutMutation},
        {TEST_REJECT_TIMELINE_SAMPLE_TARGETS, TestRejectTimelineSampleTargetsBeforeWorldRestore},
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

    if (RuntimeAssetWorldObjectRecordStreamHandoffFixtureTestNameMatches(test_name)) {
        return RunRuntimeAssetWorldObjectRecordStreamHandoffFixtureTest(test_name);
    }

    if (RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTestNameMatches(test_name)) {
        return RunRuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest(test_name);
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
