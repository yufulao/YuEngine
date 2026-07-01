// 模块: Tests Animation
// 文件: Tests/Animation/AnimationRuntimeFoundationTests.cpp

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"
#include "YuEngine/Kernel/RuntimeFrameMode.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryKind.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRecord.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameDrawRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameEntityRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameResult.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectDesc.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldRegistrationResult.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformBridgeDesc.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldTransformStatus.h"

using yuengine::animation::AnimationRuntimeChannel;
using yuengine::animation::AnimationRuntimeClipRecord;
using yuengine::animation::AnimationRuntimeInterpolation;
using yuengine::animation::AnimationRuntimeKeyframeRecord;
using yuengine::animation::AnimationRuntimeSampledValue;
using yuengine::animation::AnimationRuntimeSampler;
using yuengine::animation::AnimationRuntimeSampleRequest;
using yuengine::animation::AnimationRuntimeSampleResult;
using yuengine::animation::AnimationRuntimeStatus;
using yuengine::animation::AnimationRuntimeTrackRecord;
using yuengine::animation::AnimationRuntimeTransformApplyRequest;
using yuengine::animation::AnimationRuntimeTransformApplyResult;
using yuengine::asset::AssetHandle;
using yuengine::kernel::RuntimeFrameContext;
using yuengine::kernel::RuntimeFrameMode;
using yuengine::renderscene::RenderSceneCameraBindingResult;
using yuengine::renderscene::RenderScenePrimitiveGeometryBuilder;
using yuengine::renderscene::RenderScenePrimitiveGeometryKind;
using yuengine::renderscene::RenderScenePrimitiveGeometryRecord;
using yuengine::renderscene::RenderScenePrimitiveGeometryRequest;
using yuengine::renderscene::RenderSceneRuntimeFrameBuilder;
using yuengine::renderscene::RenderSceneRuntimeFrameDrawRecord;
using yuengine::renderscene::RenderSceneRuntimeFrameEntityRequest;
using yuengine::renderscene::RenderSceneRuntimeFrameRequest;
using yuengine::renderscene::RenderSceneRuntimeFrameResult;
using yuengine::renderscene::RenderSceneRuntimeFrameStatus;
using yuengine::renderscene::RenderSceneRuntimeMaterialBuilder;
using yuengine::renderscene::RenderSceneRuntimeMaterialRecord;
using yuengine::renderscene::RenderSceneRuntimeMaterialRequest;
using yuengine::renderscene::RenderSceneRuntimeMaterialTextureSlot;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveTopology;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::RhiVertexBufferView;
using yuengine::world::WorldInstance;
using yuengine::world::WorldObjectDesc;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldRegistrationResult;
using yuengine::world::WorldTransformBridge;
using yuengine::world::WorldTransformBridgeDesc;
using yuengine::world::WorldTransformResult;
using yuengine::world::WorldTransformState;
using yuengine::world::WorldTransformStatus;

namespace {
constexpr const char *TEST_RECORDS = "Animation_RuntimeRecordsSampleClipTrackKeyframes";
constexpr const char *TEST_INTERPOLATION = "Animation_RuntimeSamplerInterpolatesDeterministicTransformChannels";
constexpr const char *TEST_FRAME_CONTEXT = "Animation_RuntimeSamplerUsesFrameContextTime";
constexpr const char *TEST_APPLY_RENDER_SCENE =
    "Animation_RuntimeSamplerAppliesTransformBeforeRenderSceneConsumes";
constexpr const char *TEST_FAILURE_STATES = "Animation_RuntimeSamplerReportsFailureStates";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t CLIP_ID = 1101U;
constexpr std::uint32_t TRACK_ID = 1201U;
constexpr std::uint32_t WORLD_OBJECT_VALUE = 1301U;
constexpr std::uint32_t FRAME_ID = 1401U;
constexpr std::uint32_t CAMERA_ID = 1501U;
constexpr std::uint32_t MATERIAL_ID = 1601U;
constexpr std::uint32_t MATERIAL_ASSET_SLOT = 1701U;
constexpr std::uint32_t TEXTURE_ASSET_SLOT = 1801U;
constexpr std::size_t VERTEX_STRIDE_BYTES = 32U;
constexpr std::size_t VERTEX_BUFFER_BYTES = VERTEX_STRIDE_BYTES * 64U;
constexpr std::size_t INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * 128U;
constexpr std::uint64_t HALF_SECOND_NANOSECONDS = 500000000ULL;
constexpr std::uint64_t ONE_SECOND_NANOSECONDS = 1000000000ULL;
constexpr float TOLERANCE = 0.0001F;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool Approx(float left, float right) {
    const float delta = std::fabs(left - right);
    return delta <= TOLERANCE;
}

RuntimeFrameContext FrameContext(std::uint64_t fixed_time_nanoseconds) {
    RuntimeFrameContext context{};
    context.frame_index = 7U;
    context.delta_time_nanoseconds = HALF_SECOND_NANOSECONDS;
    context.fixed_time_nanoseconds = fixed_time_nanoseconds;
    context.frame_mode = RuntimeFrameMode::Fixed;
    return context;
}

AnimationRuntimeClipRecord ClipRecord(std::size_t track_count=1U) {
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
    AnimationRuntimeChannel channel,
    std::size_t first_keyframe_index=0U,
    std::size_t keyframe_count=2U,
    AnimationRuntimeInterpolation interpolation=AnimationRuntimeInterpolation::Linear) {
    AnimationRuntimeTrackRecord track{};
    track.track_id = TRACK_ID + static_cast<std::uint32_t>(first_keyframe_index);
    track.target = WorldObjectId{WORLD_OBJECT_VALUE};
    track.channel = channel;
    track.interpolation = interpolation;
    track.first_keyframe_index = first_keyframe_index;
    track.keyframe_count = keyframe_count;
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

AnimationRuntimeSampleRequest SampleRequest(
    std::span<const AnimationRuntimeClipRecord> clips,
    std::span<const AnimationRuntimeTrackRecord> tracks,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes,
    std::uint64_t fixed_time_nanoseconds=HALF_SECOND_NANOSECONDS,
    std::uint64_t clip_start_time_nanoseconds=0U) {
    AnimationRuntimeSampleRequest request{};
    request.clip_id = CLIP_ID;
    request.clips = clips;
    request.tracks = tracks;
    request.keyframes = keyframes;
    request.frame_context = FrameContext(fixed_time_nanoseconds);
    request.clip_start_time_nanoseconds = clip_start_time_nanoseconds;
    return request;
}

WorldTransformState TransformState() {
    WorldTransformState transform{};
    transform.translation_x = 0.0F;
    transform.translation_y = 0.0F;
    transform.translation_z = 0.0F;
    transform.rotation_x = 0.0F;
    transform.rotation_y = 0.0F;
    transform.rotation_z = 0.0F;
    transform.rotation_w = 1.0F;
    transform.scale_x = 1.0F;
    transform.scale_y = 1.0F;
    transform.scale_z = 1.0F;
    return transform;
}

AssetHandle MakeAsset(std::uint32_t slot) {
    return AssetHandle{slot, 1U};
}

RhiVertexBufferView VertexBufferView() {
    RhiVertexBufferView view{};
    view.buffer = RhiBufferHandle{1U, 1U};
    view.stride_bytes = VERTEX_STRIDE_BYTES;
    view.size_bytes = VERTEX_BUFFER_BYTES;
    return view;
}

RhiIndexBufferView IndexBufferView() {
    RhiIndexBufferView view{};
    view.buffer = RhiBufferHandle{2U, 1U};
    view.size_bytes = INDEX_BUFFER_BYTES;
    view.format = RhiIndexFormat::Uint16;
    return view;
}

RenderScenePrimitiveGeometryRecord GeometryRecord() {
    RenderScenePrimitiveGeometryRequest request{};
    request.geometry_asset = MakeAsset(3U);
    request.kind = RenderScenePrimitiveGeometryKind::Cube;
    request.segment_count = 16U;
    request.draw_id = 4U;
    request.pass_id = 5U;
    request.material_id = MATERIAL_ID;
    request.vertex_buffer = VertexBufferView();
    request.index_buffer = IndexBufferView();

    RenderScenePrimitiveGeometryRecord record{};
    RenderScenePrimitiveGeometryBuilder builder;
    builder.Build(request, &record);
    return record;
}

RenderSceneRuntimeMaterialTextureSlot MaterialTextureSlot(std::uint32_t slot) {
    RenderSceneRuntimeMaterialTextureSlot texture_slot{};
    texture_slot.slot = slot;
    texture_slot.texture_asset = MakeAsset(TEXTURE_ASSET_SLOT + slot);
    texture_slot.sampled_texture.texture = RhiTextureHandle{10U + slot, 1U};
    texture_slot.sampled_texture.slot = slot;
    texture_slot.sampler.sampler = RhiSamplerHandle{20U + slot, 1U};
    texture_slot.sampler.slot = slot;
    return texture_slot;
}

RhiPipelineHandle PipelineHandle() {
    return RhiPipelineHandle{30U, 1U};
}

RenderSceneRuntimeMaterialRecord MaterialRecord() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MaterialTextureSlot(0U),
        MaterialTextureSlot(1U),
        MaterialTextureSlot(2U)};
    RenderSceneRuntimeMaterialRequest request{};
    request.material_asset = MakeAsset(MATERIAL_ASSET_SLOT);
    request.material_id = MATERIAL_ID;
    request.pipeline = PipelineHandle();
    request.texture_slots = slots;

    RenderSceneRuntimeMaterialRecord record{};
    RenderSceneRuntimeMaterialBuilder builder;
    builder.Build(request, &record);
    return record;
}

RenderSceneCameraBindingResult CameraBinding() {
    RenderSceneCameraBindingResult result{};
    result.camera.camera_id = CAMERA_ID;
    result.camera.is_active = true;
    return result;
}

AnimationRuntimeStatus SampleStatus(
    const AnimationRuntimeSampleRequest &request,
    std::span<AnimationRuntimeSampledValue> values) {
    AnimationRuntimeSampler sampler;
    AnimationRuntimeSampleResult result{};
    return sampler.Sample(request, values, &result);
}

int AnimationRuntimeRecordsSampleClipTrackKeyframes() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord()};
    const std::array<AnimationRuntimeTrackRecord, 1U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX)};
    const std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{
        Keyframe(0.0F, 2.0F),
        Keyframe(1.0F, 6.0F)};
    std::array<AnimationRuntimeSampledValue, 1U> values{};
    AnimationRuntimeSampleResult result{};

    AnimationRuntimeSampler sampler;
    const AnimationRuntimeStatus status =
        sampler.Sample(SampleRequest(clips, tracks, keyframes), values, &result);
    if (status != AnimationRuntimeStatus::Success) {
        return Fail("runtime animation record sample failed");
    }

    if (result.clip_id != CLIP_ID || result.sampled_value_count != 1U) {
        return Fail("runtime animation sample result identity mismatch");
    }

    if (values[0U].target.value != WORLD_OBJECT_VALUE) {
        return Fail("runtime animation sample target mismatch");
    }

    if (values[0U].channel != AnimationRuntimeChannel::TranslationX) {
        return Fail("runtime animation sample channel mismatch");
    }

    if (!Approx(values[0U].value, 4.0F)) {
        return Fail("runtime animation keyframe interpolation mismatch");
    }

    return 0;
}

int AnimationRuntimeSamplerInterpolatesDeterministicTransformChannels() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord(3U)};
    const std::array<AnimationRuntimeTrackRecord, 3U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U, 2U),
        TrackRecord(AnimationRuntimeChannel::RotationW, 2U, 2U),
        TrackRecord(AnimationRuntimeChannel::ScaleZ, 4U, 2U, AnimationRuntimeInterpolation::Step)};
    const std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 10.0F),
        Keyframe(0.0F, 1.0F),
        Keyframe(1.0F, 0.0F),
        Keyframe(0.0F, 2.0F),
        Keyframe(1.0F, 5.0F)};
    std::array<AnimationRuntimeSampledValue, 3U> values{};

    const AnimationRuntimeStatus status = SampleStatus(SampleRequest(clips, tracks, keyframes), values);
    if (status != AnimationRuntimeStatus::Success) {
        return Fail("runtime animation deterministic interpolation failed");
    }

    if (!Approx(values[0U].value, 5.0F)) {
        return Fail("runtime animation linear translation mismatch");
    }

    if (!Approx(values[1U].value, 0.5F)) {
        return Fail("runtime animation linear rotation mismatch");
    }

    if (!Approx(values[2U].value, 2.0F)) {
        return Fail("runtime animation step scale mismatch");
    }

    return 0;
}

int AnimationRuntimeSamplerUsesFrameContextTime() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord()};
    const std::array<AnimationRuntimeTrackRecord, 1U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationY)};
    const std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 12.0F)};
    std::array<AnimationRuntimeSampledValue, 1U> values{};
    AnimationRuntimeSampleResult result{};

    const AnimationRuntimeSampleRequest request =
        SampleRequest(clips, tracks, keyframes, ONE_SECOND_NANOSECONDS + HALF_SECOND_NANOSECONDS,
            ONE_SECOND_NANOSECONDS);
    AnimationRuntimeSampler sampler;
    const AnimationRuntimeStatus status = sampler.Sample(request, values, &result);
    if (status != AnimationRuntimeStatus::Success) {
        return Fail("runtime animation frame context sampling failed");
    }

    if (!Approx(result.sample_time_seconds, 0.5F)) {
        return Fail("runtime animation frame context local time mismatch");
    }

    if (!Approx(values[0U].value, 6.0F)) {
        return Fail("runtime animation frame context value mismatch");
    }

    return 0;
}

int AnimationRuntimeSamplerAppliesTransformBeforeRenderSceneConsumes() {
    WorldInstance world;
    const WorldObjectId object_id{WORLD_OBJECT_VALUE};
    const WorldRegistrationResult registration = world.RegisterObject(WorldObjectDesc{object_id, true});
    if (!registration.Succeeded()) {
        return Fail("runtime animation world object registration failed");
    }

    WorldTransformBridge bridge(world, WorldTransformBridgeDesc{4U});
    if (bridge.Register(object_id, TransformState()).status != WorldTransformStatus::Success) {
        return Fail("runtime animation transform register failed");
    }

    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord(2U)};
    const std::array<AnimationRuntimeTrackRecord, 2U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U, 2U),
        TrackRecord(AnimationRuntimeChannel::RotationY, 2U, 2U)};
    const std::array<AnimationRuntimeKeyframeRecord, 4U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 8.0F),
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 1.0F)};
    std::array<AnimationRuntimeSampledValue, 2U> values{};
    AnimationRuntimeSampleResult sample_result{};

    AnimationRuntimeSampler sampler;
    AnimationRuntimeStatus status =
        sampler.Sample(SampleRequest(clips, tracks, keyframes), values, &sample_result);
    if (status != AnimationRuntimeStatus::Success) {
        return Fail("runtime animation sample before apply failed");
    }

    AnimationRuntimeTransformApplyResult apply_result{};
    const AnimationRuntimeTransformApplyRequest apply_request{&bridge, values};
    status = sampler.ApplySampledTransform(apply_request, &apply_result);
    if (status != AnimationRuntimeStatus::Success) {
        return Fail("runtime animation apply failed");
    }

    if (apply_result.applied_value_count != 2U || apply_result.updated_object_count != 1U) {
        return Fail("runtime animation apply counters mismatch");
    }

    const WorldTransformResult transform_result = bridge.Query(object_id);
    if (transform_result.status != WorldTransformStatus::Success) {
        return Fail("runtime animation applied transform query failed");
    }

    const RenderSceneRuntimeFrameEntityRequest entity{
        object_id,
        transform_result.transform_state,
        GeometryRecord(),
        true,
        true};
    const std::array<RenderSceneRuntimeFrameEntityRequest, 1U> entities{entity};
    RenderSceneRuntimeFrameRequest frame_request{};
    frame_request.frame_id = FRAME_ID;
    frame_request.camera = CameraBinding();
    frame_request.material = MaterialRecord();
    frame_request.entities = entities;

    std::array<RenderSceneRuntimeFrameDrawRecord, 1U> draws{};
    RenderSceneRuntimeFrameResult frame_result{};
    RenderSceneRuntimeFrameBuilder frame_builder;
    const RenderSceneRuntimeFrameStatus frame_status =
        frame_builder.Build(frame_request, draws, &frame_result);
    if (frame_status != RenderSceneRuntimeFrameStatus::Success) {
        return Fail("runtime animation render scene consumption failed");
    }

    if (!Approx(draws[0U].transform.translation_x, 4.0F)) {
        return Fail("runtime animation render scene consumed wrong translation");
    }

    if (!Approx(draws[0U].transform.rotation_y, 0.5F)) {
        return Fail("runtime animation render scene consumed wrong rotation");
    }

    return 0;
}

int AnimationRuntimeSamplerReportsFailureStates() {
    const std::array<AnimationRuntimeClipRecord, 1U> clips{ClipRecord()};
    const std::array<AnimationRuntimeTrackRecord, 1U> tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX)};
    const std::array<AnimationRuntimeKeyframeRecord, 2U> keyframes{
        Keyframe(0.0F, 0.0F),
        Keyframe(1.0F, 1.0F)};
    std::array<AnimationRuntimeSampledValue, 1U> values{};

    const std::array<AnimationRuntimeClipRecord, 0U> empty_clips{};
    if (SampleStatus(SampleRequest(empty_clips, tracks, keyframes), values) !=
        AnimationRuntimeStatus::MissingClip) {
        return Fail("runtime animation missing clip status mismatch");
    }

    std::array<AnimationRuntimeClipRecord, 1U> missing_track_clips{ClipRecord(0U)};
    if (SampleStatus(SampleRequest(missing_track_clips, tracks, keyframes), values) !=
        AnimationRuntimeStatus::MissingTrack) {
        return Fail("runtime animation missing track status mismatch");
    }

    std::array<AnimationRuntimeTrackRecord, 1U> missing_key_tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX, 0U, 0U)};
    if (SampleStatus(SampleRequest(clips, missing_key_tracks, keyframes), values) !=
        AnimationRuntimeStatus::MissingKeyframe) {
        return Fail("runtime animation missing keyframe status mismatch");
    }

    std::array<AnimationRuntimeTrackRecord, 1U> unsupported_interpolation_tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX)};
    unsupported_interpolation_tracks[0U].interpolation =
        static_cast<AnimationRuntimeInterpolation>(99);
    if (SampleStatus(SampleRequest(clips, unsupported_interpolation_tracks, keyframes), values) !=
        AnimationRuntimeStatus::UnsupportedInterpolation) {
        return Fail("runtime animation unsupported interpolation status mismatch");
    }

    std::array<AnimationRuntimeTrackRecord, 1U> unsupported_channel_tracks{
        TrackRecord(AnimationRuntimeChannel::TranslationX)};
    unsupported_channel_tracks[0U].channel = static_cast<AnimationRuntimeChannel>(99);
    if (SampleStatus(SampleRequest(clips, unsupported_channel_tracks, keyframes), values) !=
        AnimationRuntimeStatus::UnsupportedChannel) {
        return Fail("runtime animation unsupported channel status mismatch");
    }

    if (SampleStatus(SampleRequest(clips, tracks, keyframes, ONE_SECOND_NANOSECONDS * 2ULL), values) !=
        AnimationRuntimeStatus::TimeOutOfRange) {
        return Fail("runtime animation out-of-range status mismatch");
    }

    std::array<AnimationRuntimeSampledValue, 0U> small_output{};
    AnimationRuntimeSampleResult result{};
    AnimationRuntimeSampler sampler;
    if (sampler.Sample(SampleRequest(clips, tracks, keyframes), small_output, &result) !=
        AnimationRuntimeStatus::OutputCapacityExceeded) {
        return Fail("runtime animation output capacity status mismatch");
    }

    if (result.required_sampled_value_count != 1U) {
        return Fail("runtime animation output required count mismatch");
    }

    std::array<AnimationRuntimeClipRecord, 1U> layered_clips{ClipRecord()};
    layered_clips[0U].layer_count = 2U;
    if (sampler.Sample(SampleRequest(layered_clips, tracks, keyframes), values, &result) !=
        AnimationRuntimeStatus::LayerCapacityExceeded) {
        return Fail("runtime animation layer capacity status mismatch");
    }

    if (result.required_layer_count != 2U) {
        return Fail("runtime animation layer required count mismatch");
    }

    if (SampleStatus(SampleRequest(clips, tracks, keyframes, HALF_SECOND_NANOSECONDS,
        ONE_SECOND_NANOSECONDS), values) != AnimationRuntimeStatus::InvalidTime) {
        return Fail("runtime animation invalid time status mismatch");
    }

    WorldInstance world;
    WorldTransformBridge bridge(world, WorldTransformBridgeDesc{1U});
    const std::array<AnimationRuntimeSampledValue, 1U> invalid_target{
        AnimationRuntimeSampledValue{WorldObjectId{}, AnimationRuntimeChannel::TranslationX, 1.0F}};
    AnimationRuntimeTransformApplyResult apply_result{};
    if (sampler.ApplySampledTransform(AnimationRuntimeTransformApplyRequest{&bridge, invalid_target},
        &apply_result) != AnimationRuntimeStatus::InvalidTarget) {
        return Fail("runtime animation invalid target status mismatch");
    }

    const std::array<AnimationRuntimeSampledValue, 1U> missing_target{
        AnimationRuntimeSampledValue{WorldObjectId{WORLD_OBJECT_VALUE}, AnimationRuntimeChannel::TranslationX, 1.0F}};
    if (sampler.ApplySampledTransform(AnimationRuntimeTransformApplyRequest{&bridge, missing_target},
        &apply_result) != AnimationRuntimeStatus::TargetNotFound) {
        return Fail("runtime animation missing target status mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_RECORDS) {
        return AnimationRuntimeRecordsSampleClipTrackKeyframes();
    }

    if (name == TEST_INTERPOLATION) {
        return AnimationRuntimeSamplerInterpolatesDeterministicTransformChannels();
    }

    if (name == TEST_FRAME_CONTEXT) {
        return AnimationRuntimeSamplerUsesFrameContextTime();
    }

    if (name == TEST_APPLY_RENDER_SCENE) {
        return AnimationRuntimeSamplerAppliesTransformBeforeRenderSceneConsumes();
    }

    if (name == TEST_FAILURE_STATES) {
        return AnimationRuntimeSamplerReportsFailureStates();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
