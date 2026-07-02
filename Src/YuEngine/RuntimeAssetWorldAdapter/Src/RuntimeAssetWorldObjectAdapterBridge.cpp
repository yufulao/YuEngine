// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Src/RuntimeAssetWorldObjectAdapterBridge.cpp

#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterBridge.h"

#include <cmath>
#include <span>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

using yuengine::animation::AnimationRuntimeSampleRequest;
using yuengine::animation::AnimationRuntimeSampledValue;
using yuengine::animation::AnimationRuntimeSampleResult;
using yuengine::animation::AnimationRuntimeSampler;
using yuengine::animation::AnimationRuntimeChannel;
using yuengine::animation::AnimationRuntimeStatus;
using yuengine::animation::AnimationRuntimeTransformApplyRequest;
using yuengine::animation::AnimationRuntimeTransformApplyResult;
using yuengine::object::ObjectHandle;
using yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord;
using yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord;
using yuengine::runtimeasset::RuntimeAssetTargetIdentityKind;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;
using yuengine::world::WorldTransformResult;
using yuengine::world::WorldTransformState;
using yuengine::world::WorldTransformStatus;

namespace yuengine::runtimeassetworldadapter {
namespace {
bool WorldObjectIdsMatch(WorldObjectId left, WorldObjectId right) {
    return left.value == right.value;
}

bool RuntimeAssetTargetKindIsSupported(RuntimeAssetTargetIdentityKind target_kind) {
    if (target_kind == RuntimeAssetTargetIdentityKind::SceneNode) {
        return true;
    }

    if (target_kind == RuntimeAssetTargetIdentityKind::ModelNode) {
        return true;
    }

    return target_kind == RuntimeAssetTargetIdentityKind::SkeletonJoint;
}

bool RuntimeSampledTransformChannelIsSupported(AnimationRuntimeChannel channel) {
    switch (channel) {
    case AnimationRuntimeChannel::TranslationX:
    case AnimationRuntimeChannel::TranslationY:
    case AnimationRuntimeChannel::TranslationZ:
    case AnimationRuntimeChannel::RotationX:
    case AnimationRuntimeChannel::RotationY:
    case AnimationRuntimeChannel::RotationZ:
    case AnimationRuntimeChannel::RotationW:
    case AnimationRuntimeChannel::ScaleX:
    case AnimationRuntimeChannel::ScaleY:
    case AnimationRuntimeChannel::ScaleZ:
        return true;
    default:
        break;
    }

    return false;
}

RuntimeAssetWorldObjectAdapterStatus MapWorldTransformStatus(WorldTransformStatus status) {
    if (status == WorldTransformStatus::Success) {
        return RuntimeAssetWorldObjectAdapterStatus::Success;
    }

    if (status == WorldTransformStatus::InvalidWorldObjectId) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
    }

    if (status == WorldTransformStatus::TransformNotFound) {
        return RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound;
    }

    return RuntimeAssetWorldObjectAdapterStatus::TransformApplyFailed;
}

void ApplyRuntimeSampledTransformChannel(
    AnimationRuntimeChannel channel,
    float value,
    WorldTransformState *transform_state) {
    if (transform_state == nullptr) {
        return;
    }

    switch (channel) {
    case AnimationRuntimeChannel::TranslationX:
        transform_state->translation_x = value;
        break;
    case AnimationRuntimeChannel::TranslationY:
        transform_state->translation_y = value;
        break;
    case AnimationRuntimeChannel::TranslationZ:
        transform_state->translation_z = value;
        break;
    case AnimationRuntimeChannel::RotationX:
        transform_state->rotation_x = value;
        break;
    case AnimationRuntimeChannel::RotationY:
        transform_state->rotation_y = value;
        break;
    case AnimationRuntimeChannel::RotationZ:
        transform_state->rotation_z = value;
        break;
    case AnimationRuntimeChannel::RotationW:
        transform_state->rotation_w = value;
        break;
    case AnimationRuntimeChannel::ScaleX:
        transform_state->scale_x = value;
        break;
    case AnimationRuntimeChannel::ScaleY:
        transform_state->scale_y = value;
        break;
    case AnimationRuntimeChannel::ScaleZ:
        transform_state->scale_z = value;
        break;
    default:
        break;
    }
}
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterResult::Success(
    RuntimeAssetWorldObjectAdapterState state) {
    return RuntimeAssetWorldObjectAdapterResult{
        RuntimeAssetWorldObjectAdapterStatus::Success,
        state,
        0U,
        0U,
        0U,
        0U,
        0U,
        0U};
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterResult::Failure(
    RuntimeAssetWorldObjectAdapterStatus status) {
    return RuntimeAssetWorldObjectAdapterResult{
        status,
        RuntimeAssetWorldObjectAdapterState{},
        0U,
        0U,
        0U,
        0U,
        0U,
        0U};
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterResult::Failure(
    RuntimeAssetWorldObjectAdapterStatus status,
    std::uint32_t failed_mapping_index,
    std::uint64_t failed_target_id) {
    return RuntimeAssetWorldObjectAdapterResult{
        status,
        RuntimeAssetWorldObjectAdapterState{},
        failed_mapping_index,
        0U,
        failed_target_id,
        0U,
        0U,
        0U};
}

bool RuntimeAssetWorldObjectAdapterResult::Succeeded() const {
    return status == RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::BuildRestoreRecords(
    const RuntimeAssetWorldObjectAdapterRequest &request) {
    ++snapshot_.build_attempt_count;

    std::uint32_t failed_mapping_index = 0U;
    std::uint64_t failed_target_id = 0U;
    std::uint32_t required_identity_output_count = 0U;
    std::uint32_t required_transform_output_count = 0U;
    const RuntimeAssetWorldObjectAdapterStatus request_status = ValidateRequest(
        request,
        &failed_mapping_index,
        &failed_target_id,
        &required_identity_output_count,
        &required_transform_output_count);
    if (request_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        return RecordFailure(
            request_status,
            failed_mapping_index,
            failed_target_id,
            required_identity_output_count,
            required_transform_output_count);
    }

    std::uint32_t mapping_index = 0U;
    while (mapping_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_record =
            FindIdentityRecord(request, mapping.target_id);
        const RuntimeAssetSceneTransformOutputRecord &scene_transform =
            request.scene_transforms[mapping.scene_transform_index];
        WorldSceneObjectTransformRestoreIdentityRecord identity_output{};
        identity_output.world_object_id = identity_record->world_object_id;
        identity_output.object_handle = identity_record->object_handle;
        request.output_identities[mapping_index] = identity_output;

        WorldSceneObjectTransformRestoreTransformRecord transform_output{};
        transform_output.world_object_id = scene_transform.world_object_id;
        transform_output.transform_state = scene_transform.transform;
        request.output_transforms[mapping_index] = transform_output;

        ++mapping_index;
    }

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = request.runtime_instance_mapping_count;
    state.output_identity_count = request.runtime_instance_mapping_count;
    state.output_transform_count = request.runtime_instance_mapping_count;
    return RecordSuccess(state);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::BuildProducerPlaybackTransformApplicationRequest(
    const RuntimeAssetWorldObjectProducerPlaybackRequest &request,
    RuntimeAssetWorldObjectTransformApplicationRequest *out_request) {
    RuntimeAssetWorldObjectTimelineTransformSampleRequest timeline_request{};
    timeline_request.runtime_instance_mappings = request.runtime_instance_mappings;
    timeline_request.runtime_instance_mapping_count = request.runtime_instance_mapping_count;
    timeline_request.identity_records = request.identity_records;
    timeline_request.identity_record_count = request.identity_record_count;
    timeline_request.transform_destination = request.transform_destination;
    timeline_request.clip_id = request.export_clip_id;
    timeline_request.clips = request.export_clips;
    timeline_request.tracks = request.export_tracks;
    timeline_request.keyframes = request.export_keyframes;
    timeline_request.frame_context = request.playback_frame_context;
    timeline_request.clip_start_time_nanoseconds = request.export_clip_start_time_nanoseconds;
    timeline_request.sampled_value_scratch = request.sampled_value_scratch;
    timeline_request.sampled_value_scratch_capacity = request.sampled_value_scratch_capacity;
    timeline_request.sampled_value_output = request.sampled_value_output;
    timeline_request.sampled_value_output_capacity = request.sampled_value_output_capacity;
    return BuildTimelineTransformApplicationRequest(timeline_request, out_request);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::BuildProducerPlaybackBatchTransformApplicationRequest(
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest &request,
    RuntimeAssetWorldObjectTransformApplicationRequest *out_request) {
    ++snapshot_.transform_sampler_bridge_attempt_count;

    const RuntimeAssetWorldObjectAdapterStatus batch_status =
        ValidateProducerPlaybackBatchRequest(request, out_request);
    if (batch_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        return RecordFailure(batch_status);
    }

    const RuntimeAssetWorldObjectProducerPlaybackRequest &root_request =
        request.producer_playback_requests[0U];
    const AnimationRuntimeSampler sampler{};
    std::uint32_t sampled_value_count = 0U;
    std::uint32_t output_capacity_failed_index = request.producer_playback_request_count;
    std::uint32_t request_index = 0U;
    while (request_index < request.producer_playback_request_count) {
        const RuntimeAssetWorldObjectProducerPlaybackRequest &playback_request =
            request.producer_playback_requests[request_index];
        const RuntimeAssetWorldObjectAdapterStatus request_status =
            ValidateProducerPlaybackRequest(playback_request);
        if (request_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            RuntimeAssetWorldObjectAdapterResult result = RecordFailure(request_status);
            result.failed_playback_request_index = request_index;
            return result;
        }

        if (playback_request.runtime_instance_mappings != root_request.runtime_instance_mappings ||
            playback_request.runtime_instance_mapping_count != root_request.runtime_instance_mapping_count) {
            RuntimeAssetWorldObjectAdapterResult result =
                RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput);
            result.failed_playback_request_index = request_index;
            return result;
        }

        if (playback_request.identity_records != root_request.identity_records ||
            playback_request.identity_record_count != root_request.identity_record_count) {
            RuntimeAssetWorldObjectAdapterResult result =
                RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput);
            result.failed_playback_request_index = request_index;
            return result;
        }

        if (playback_request.transform_destination != root_request.transform_destination) {
            RuntimeAssetWorldObjectAdapterResult result =
                RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidTransformDestination);
            result.failed_playback_request_index = request_index;
            return result;
        }

        AnimationRuntimeSampleRequest sample_request{};
        sample_request.clip_id = playback_request.export_clip_id;
        sample_request.clips = playback_request.export_clips;
        sample_request.tracks = playback_request.export_tracks;
        sample_request.keyframes = playback_request.export_keyframes;
        sample_request.frame_context = playback_request.playback_frame_context;
        sample_request.clip_start_time_nanoseconds = playback_request.export_clip_start_time_nanoseconds;

        AnimationRuntimeSampledValue *sampled_value_scratch = nullptr;
        if (sampled_value_count < request.sampled_value_scratch_capacity) {
            sampled_value_scratch = request.sampled_value_scratch + sampled_value_count;
        }

        const std::uint32_t sampled_value_scratch_capacity =
            request.sampled_value_scratch_capacity - sampled_value_count;
        AnimationRuntimeSampleResult sample_result{};
        const AnimationRuntimeStatus animation_status = sampler.Sample(
            sample_request,
            std::span<AnimationRuntimeSampledValue>(
                sampled_value_scratch,
                sampled_value_scratch_capacity),
            &sample_result);
        const RuntimeAssetWorldObjectAdapterStatus adapter_status = MapAnimationSampleStatus(animation_status);
        if (adapter_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            RuntimeAssetWorldObjectAdapterResult result = RecordFailure(adapter_status);
            result.failed_playback_request_index = request_index;
            result.required_sampled_transform_value_count =
                sampled_value_count + static_cast<std::uint32_t>(sample_result.required_sampled_value_count);
            snapshot_.required_sampled_transform_value_count = result.required_sampled_transform_value_count;
            return result;
        }

        const std::uint32_t request_sampled_value_count =
            static_cast<std::uint32_t>(sample_result.sampled_value_count);
        RuntimeAssetWorldObjectTransformApplicationRequest application_request{};
        application_request.runtime_instance_mappings = playback_request.runtime_instance_mappings;
        application_request.runtime_instance_mapping_count = playback_request.runtime_instance_mapping_count;
        application_request.identity_records = playback_request.identity_records;
        application_request.identity_record_count = playback_request.identity_record_count;
        application_request.transform_destination = playback_request.transform_destination;
        application_request.sampled_values = sampled_value_scratch;
        application_request.sampled_value_count = request_sampled_value_count;

        const RuntimeAssetWorldObjectAdapterResult preflight_result =
            PreflightSampledTransforms(application_request);
        if (!preflight_result.Succeeded()) {
            RuntimeAssetWorldObjectAdapterResult result = preflight_result;
            result.failed_playback_request_index = request_index;
            return result;
        }

        const std::uint32_t next_sampled_value_count = sampled_value_count + request_sampled_value_count;
        if (output_capacity_failed_index == request.producer_playback_request_count &&
            next_sampled_value_count > request.sampled_value_output_capacity) {
            output_capacity_failed_index = request_index;
        }

        sampled_value_count = next_sampled_value_count;
        ++request_index;
    }

    if (sampled_value_count > request.sampled_value_output_capacity) {
        RuntimeAssetWorldObjectAdapterResult result =
            RecordFailure(RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded);
        result.failed_playback_request_index = output_capacity_failed_index;
        result.required_sampled_transform_value_count = sampled_value_count;
        snapshot_.required_sampled_transform_value_count = result.required_sampled_transform_value_count;
        return result;
    }

    std::uint32_t sampled_value_index = 0U;
    while (sampled_value_index < sampled_value_count) {
        request.sampled_value_output[sampled_value_index] = request.sampled_value_scratch[sampled_value_index];
        ++sampled_value_index;
    }

    RuntimeAssetWorldObjectTransformApplicationRequest application_request{};
    application_request.runtime_instance_mappings = root_request.runtime_instance_mappings;
    application_request.runtime_instance_mapping_count = root_request.runtime_instance_mapping_count;
    application_request.identity_records = root_request.identity_records;
    application_request.identity_record_count = root_request.identity_record_count;
    application_request.transform_destination = root_request.transform_destination;
    application_request.sampled_values = request.sampled_value_output;
    application_request.sampled_value_count = sampled_value_count;
    *out_request = application_request;

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = root_request.runtime_instance_mapping_count;
    state.sampled_transform_value_count = sampled_value_count;
    return RecordTransformApplicationRequestSuccess(state);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::SnapshotProducerPlaybackBatchTransformApplicationCount(
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest &request) {
    ++snapshot_.transform_sampler_bridge_attempt_count;

    const RuntimeAssetWorldObjectAdapterStatus batch_status =
        ValidateProducerPlaybackBatchCountRequest(request);
    if (batch_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        return RecordFailure(batch_status);
    }

    const RuntimeAssetWorldObjectProducerPlaybackRequest &root_request =
        request.producer_playback_requests[0U];
    const AnimationRuntimeSampler sampler{};
    std::uint32_t sampled_value_count = 0U;
    std::uint32_t request_index = 0U;
    while (request_index < request.producer_playback_request_count) {
        const RuntimeAssetWorldObjectProducerPlaybackRequest &playback_request =
            request.producer_playback_requests[request_index];
        const RuntimeAssetWorldObjectAdapterStatus request_status =
            ValidateProducerPlaybackRequest(playback_request);
        if (request_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            RuntimeAssetWorldObjectAdapterResult result = RecordFailure(request_status);
            result.failed_playback_request_index = request_index;
            return result;
        }

        if (playback_request.runtime_instance_mappings != root_request.runtime_instance_mappings ||
            playback_request.runtime_instance_mapping_count != root_request.runtime_instance_mapping_count) {
            RuntimeAssetWorldObjectAdapterResult result =
                RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput);
            result.failed_playback_request_index = request_index;
            return result;
        }

        if (playback_request.identity_records != root_request.identity_records ||
            playback_request.identity_record_count != root_request.identity_record_count) {
            RuntimeAssetWorldObjectAdapterResult result =
                RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput);
            result.failed_playback_request_index = request_index;
            return result;
        }

        if (playback_request.transform_destination != root_request.transform_destination) {
            RuntimeAssetWorldObjectAdapterResult result =
                RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidTransformDestination);
            result.failed_playback_request_index = request_index;
            return result;
        }

        AnimationRuntimeSampleRequest sample_request{};
        sample_request.clip_id = playback_request.export_clip_id;
        sample_request.clips = playback_request.export_clips;
        sample_request.tracks = playback_request.export_tracks;
        sample_request.keyframes = playback_request.export_keyframes;
        sample_request.frame_context = playback_request.playback_frame_context;
        sample_request.clip_start_time_nanoseconds = playback_request.export_clip_start_time_nanoseconds;

        AnimationRuntimeSampledValue *sampled_value_scratch = nullptr;
        if (sampled_value_count < request.sampled_value_scratch_capacity) {
            sampled_value_scratch = request.sampled_value_scratch + sampled_value_count;
        }

        const std::uint32_t sampled_value_scratch_capacity =
            request.sampled_value_scratch_capacity - sampled_value_count;
        AnimationRuntimeSampleResult sample_result{};
        const AnimationRuntimeStatus animation_status = sampler.Sample(
            sample_request,
            std::span<AnimationRuntimeSampledValue>(
                sampled_value_scratch,
                sampled_value_scratch_capacity),
            &sample_result);
        const RuntimeAssetWorldObjectAdapterStatus adapter_status = MapAnimationSampleStatus(animation_status);
        if (adapter_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            RuntimeAssetWorldObjectAdapterResult result = RecordFailure(adapter_status);
            result.failed_playback_request_index = request_index;
            result.required_sampled_transform_value_count =
                sampled_value_count + static_cast<std::uint32_t>(sample_result.required_sampled_value_count);
            snapshot_.required_sampled_transform_value_count = result.required_sampled_transform_value_count;
            return result;
        }

        const std::uint32_t request_sampled_value_count =
            static_cast<std::uint32_t>(sample_result.sampled_value_count);
        RuntimeAssetWorldObjectTransformApplicationRequest application_request{};
        application_request.runtime_instance_mappings = playback_request.runtime_instance_mappings;
        application_request.runtime_instance_mapping_count = playback_request.runtime_instance_mapping_count;
        application_request.identity_records = playback_request.identity_records;
        application_request.identity_record_count = playback_request.identity_record_count;
        application_request.transform_destination = playback_request.transform_destination;
        application_request.sampled_values = sampled_value_scratch;
        application_request.sampled_value_count = request_sampled_value_count;

        const RuntimeAssetWorldObjectAdapterResult preflight_result =
            PreflightSampledTransforms(application_request);
        if (!preflight_result.Succeeded()) {
            RuntimeAssetWorldObjectAdapterResult result = preflight_result;
            result.failed_playback_request_index = request_index;
            return result;
        }

        sampled_value_count += request_sampled_value_count;
        ++request_index;
    }

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = root_request.runtime_instance_mapping_count;
    state.sampled_transform_value_count = sampled_value_count;
    RuntimeAssetWorldObjectAdapterResult result =
        RuntimeAssetWorldObjectAdapterResult::Success(state);
    result.required_sampled_transform_value_count = sampled_value_count;
    snapshot_.required_identity_output_count = 0U;
    snapshot_.required_transform_output_count = 0U;
    snapshot_.required_sampled_transform_value_count = sampled_value_count;
    snapshot_.last_status = RuntimeAssetWorldObjectAdapterStatus::Success;
    return result;
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::BuildTimelineTransformApplicationRequest(
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest &request,
    RuntimeAssetWorldObjectTransformApplicationRequest *out_request) {
    ++snapshot_.transform_sampler_bridge_attempt_count;

    const RuntimeAssetWorldObjectAdapterStatus request_status =
        ValidateTimelineTransformSampleRequest(request, out_request);
    if (request_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        return RecordFailure(request_status);
    }

    AnimationRuntimeSampleRequest sample_request{};
    sample_request.clip_id = request.clip_id;
    sample_request.clips = request.clips;
    sample_request.tracks = request.tracks;
    sample_request.keyframes = request.keyframes;
    sample_request.frame_context = request.frame_context;
    sample_request.clip_start_time_nanoseconds = request.clip_start_time_nanoseconds;

    AnimationRuntimeSampleResult sample_result{};
    const AnimationRuntimeSampler sampler{};
    const AnimationRuntimeStatus animation_status = sampler.Sample(
        sample_request,
        std::span<AnimationRuntimeSampledValue>(
            request.sampled_value_scratch,
            request.sampled_value_scratch_capacity),
        &sample_result);
    const RuntimeAssetWorldObjectAdapterStatus adapter_status = MapAnimationSampleStatus(animation_status);
    if (adapter_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        RuntimeAssetWorldObjectAdapterResult result = RecordFailure(adapter_status);
        result.required_sampled_transform_value_count =
            static_cast<std::uint32_t>(sample_result.required_sampled_value_count);
        snapshot_.required_sampled_transform_value_count = result.required_sampled_transform_value_count;
        return result;
    }

    RuntimeAssetWorldObjectTransformApplicationRequest application_request{};
    application_request.runtime_instance_mappings = request.runtime_instance_mappings;
    application_request.runtime_instance_mapping_count = request.runtime_instance_mapping_count;
    application_request.identity_records = request.identity_records;
    application_request.identity_record_count = request.identity_record_count;
    application_request.transform_destination = request.transform_destination;
    application_request.sampled_values = request.sampled_value_scratch;
    application_request.sampled_value_count = static_cast<std::uint32_t>(sample_result.sampled_value_count);

    if (application_request.sampled_value_count > request.sampled_value_output_capacity) {
        RuntimeAssetWorldObjectAdapterResult result =
            RecordFailure(RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded);
        result.required_sampled_transform_value_count = application_request.sampled_value_count;
        snapshot_.required_sampled_transform_value_count = result.required_sampled_transform_value_count;
        return result;
    }

    const RuntimeAssetWorldObjectAdapterResult preflight_result = PreflightSampledTransforms(application_request);
    if (!preflight_result.Succeeded()) {
        return preflight_result;
    }

    std::uint32_t sampled_value_index = 0U;
    while (sampled_value_index < application_request.sampled_value_count) {
        request.sampled_value_output[sampled_value_index] = request.sampled_value_scratch[sampled_value_index];
        ++sampled_value_index;
    }

    application_request.sampled_values = request.sampled_value_output;
    *out_request = application_request;

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = request.runtime_instance_mapping_count;
    state.sampled_transform_value_count = application_request.sampled_value_count;
    return RecordTransformApplicationRequestSuccess(state);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::SnapshotTimelineTransformApplicationCount(
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest &request) {
    ++snapshot_.transform_sampler_bridge_attempt_count;

    const RuntimeAssetWorldObjectAdapterStatus request_status =
        ValidateTimelineTransformSampleCountRequest(request);
    if (request_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        return RecordFailure(request_status);
    }

    AnimationRuntimeSampleRequest sample_request{};
    sample_request.clip_id = request.clip_id;
    sample_request.clips = request.clips;
    sample_request.tracks = request.tracks;
    sample_request.keyframes = request.keyframes;
    sample_request.frame_context = request.frame_context;
    sample_request.clip_start_time_nanoseconds = request.clip_start_time_nanoseconds;

    AnimationRuntimeSampleResult sample_result{};
    const AnimationRuntimeSampler sampler{};
    std::span<AnimationRuntimeSampledValue> sampled_value_span(
        request.sampled_value_scratch,
        request.sampled_value_scratch_capacity);
    const AnimationRuntimeStatus animation_status = sampler.Sample(
        sample_request,
        sampled_value_span,
        &sample_result);
    const RuntimeAssetWorldObjectAdapterStatus adapter_status = MapAnimationSampleStatus(animation_status);
    if (adapter_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        RuntimeAssetWorldObjectAdapterResult result = RecordFailure(adapter_status);
        result.required_sampled_transform_value_count =
            static_cast<std::uint32_t>(sample_result.required_sampled_value_count);
        snapshot_.required_sampled_transform_value_count = result.required_sampled_transform_value_count;
        return result;
    }

    RuntimeAssetWorldObjectTransformApplicationRequest application_request{};
    application_request.runtime_instance_mappings = request.runtime_instance_mappings;
    application_request.runtime_instance_mapping_count = request.runtime_instance_mapping_count;
    application_request.identity_records = request.identity_records;
    application_request.identity_record_count = request.identity_record_count;
    application_request.transform_destination = request.transform_destination;
    application_request.sampled_values = request.sampled_value_scratch;
    application_request.sampled_value_count = static_cast<std::uint32_t>(sample_result.sampled_value_count);

    if (application_request.sampled_value_count > request.sampled_value_output_capacity) {
        RuntimeAssetWorldObjectAdapterResult result =
            RecordFailure(RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded);
        result.required_sampled_transform_value_count = application_request.sampled_value_count;
        snapshot_.required_sampled_transform_value_count = result.required_sampled_transform_value_count;
        return result;
    }

    const RuntimeAssetWorldObjectAdapterResult preflight_result =
        PreflightSampledTransformsWithTargetDiagnostics(application_request);
    if (!preflight_result.Succeeded()) {
        return preflight_result;
    }

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = request.runtime_instance_mapping_count;
    state.sampled_transform_value_count = application_request.sampled_value_count;
    RuntimeAssetWorldObjectAdapterResult result =
        RuntimeAssetWorldObjectAdapterResult::Success(state);
    result.required_sampled_transform_value_count = application_request.sampled_value_count;
    snapshot_.required_identity_output_count = 0U;
    snapshot_.required_transform_output_count = 0U;
    snapshot_.required_sampled_transform_value_count = application_request.sampled_value_count;
    snapshot_.last_status = RuntimeAssetWorldObjectAdapterStatus::Success;
    return result;
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::BuildTransformApplicationRequest(
    const RuntimeAssetWorldObjectTransformSamplerBridgeRequest &request,
    RuntimeAssetWorldObjectTransformApplicationRequest *out_request) {
    ++snapshot_.transform_sampler_bridge_attempt_count;

    const RuntimeAssetWorldObjectAdapterStatus request_status =
        ValidateTransformSamplerRequest(request, out_request);
    if (request_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        return RecordFailure(request_status);
    }

    AnimationRuntimeSampleResult sample_result{};
    const AnimationRuntimeSampler sampler{};
    const AnimationRuntimeStatus animation_status = sampler.Sample(
        *request.sample_request,
        std::span<AnimationRuntimeSampledValue>(
            request.sampled_value_output,
            request.sampled_value_output_capacity),
        &sample_result);
    const RuntimeAssetWorldObjectAdapterStatus adapter_status = MapAnimationSampleStatus(animation_status);
    if (adapter_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        RuntimeAssetWorldObjectAdapterResult result = RecordFailure(adapter_status);
        result.required_sampled_transform_value_count =
            static_cast<std::uint32_t>(sample_result.required_sampled_value_count);
        snapshot_.required_sampled_transform_value_count = result.required_sampled_transform_value_count;
        return result;
    }

    RuntimeAssetWorldObjectTransformApplicationRequest application_request{};
    application_request.runtime_instance_mappings = request.runtime_instance_mappings;
    application_request.runtime_instance_mapping_count = request.runtime_instance_mapping_count;
    application_request.identity_records = request.identity_records;
    application_request.identity_record_count = request.identity_record_count;
    application_request.transform_destination = request.transform_destination;
    application_request.sampled_values = request.sampled_value_output;
    application_request.sampled_value_count = static_cast<std::uint32_t>(sample_result.sampled_value_count);

    const RuntimeAssetWorldObjectAdapterResult preflight_result = PreflightSampledTransforms(application_request);
    if (!preflight_result.Succeeded()) {
        return preflight_result;
    }

    *out_request = application_request;

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = request.runtime_instance_mapping_count;
    state.sampled_transform_value_count = application_request.sampled_value_count;
    return RecordTransformApplicationRequestSuccess(state);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::ApplySampledTransforms(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request) {
    ++snapshot_.transform_application_attempt_count;

    const RuntimeAssetWorldObjectAdapterStatus request_status =
        ValidateTransformApplicationRequest(request);
    if (request_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        return RecordFailure(request_status);
    }

    AnimationRuntimeTransformApplyResult apply_result{};
    const AnimationRuntimeSampler sampler{};
    const AnimationRuntimeStatus animation_status = sampler.ApplySampledTransform(
        AnimationRuntimeTransformApplyRequest{
            request.transform_destination,
            std::span<const AnimationRuntimeSampledValue>(
                request.sampled_values,
                request.sampled_value_count)},
        &apply_result);
    const RuntimeAssetWorldObjectAdapterStatus adapter_status = MapAnimationStatus(animation_status);
    if (adapter_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        return RecordFailure(adapter_status);
    }

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = request.runtime_instance_mapping_count;
    state.applied_transform_value_count = static_cast<std::uint32_t>(apply_result.applied_value_count);
    state.updated_world_object_count = static_cast<std::uint32_t>(apply_result.updated_object_count);
    return RecordTransformApplicationSuccess(state);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::ApplyRuntimeSampledTransforms(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request) {
    ++snapshot_.transform_application_attempt_count;

    std::uint64_t failed_target_id = 0U;
    const RuntimeAssetWorldObjectAdapterStatus request_status =
        ValidateRuntimeSampledTransformApplicationRequest(request, &failed_target_id);
    if (request_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        RuntimeAssetWorldObjectAdapterResult result = RecordFailure(request_status);
        result.failed_target_id = failed_target_id;
        return result;
    }

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = request.runtime_instance_mapping_count;

    std::uint32_t sampled_value_index = 0U;
    while (sampled_value_index < request.sampled_value_count) {
        const RuntimeAssetWorldObjectRuntimeSampledTransformValue &sampled_value =
            request.sampled_values[sampled_value_index];
        WorldObjectId world_object_id{};
        if (!ResolveRuntimeSampledTransformTarget(request, sampled_value.target_id, &world_object_id)) {
            RuntimeAssetWorldObjectAdapterResult result =
                RecordFailure(RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound);
            result.failed_target_id = sampled_value.target_id;
            return result;
        }

        const WorldTransformResult query_result = request.transform_destination->Query(world_object_id);
        RuntimeAssetWorldObjectAdapterStatus adapter_status = MapWorldTransformStatus(query_result.status);
        if (adapter_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            RuntimeAssetWorldObjectAdapterResult result = RecordFailure(adapter_status);
            result.failed_target_id = sampled_value.target_id;
            return result;
        }

        WorldTransformState transform_state = query_result.transform_state;
        ApplyRuntimeSampledTransformChannel(sampled_value.channel, sampled_value.value, &transform_state);
        const WorldTransformStatus world_status = request.transform_destination->Set(world_object_id, transform_state);
        adapter_status = MapWorldTransformStatus(world_status);
        if (adapter_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            RuntimeAssetWorldObjectAdapterResult result = RecordFailure(adapter_status);
            result.failed_target_id = sampled_value.target_id;
            return result;
        }

        ++state.applied_transform_value_count;
        if (IsFirstRuntimeSampledTransformTargetOccurrence(request, sampled_value_index)) {
            ++state.updated_world_object_count;
        }

        ++sampled_value_index;
    }

    return RecordTransformApplicationSuccess(state);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::PreflightSampledTransforms(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request) {
    const RuntimeAssetWorldObjectAdapterStatus request_status =
        ValidateTransformApplicationRequest(request);
    if (request_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
        return RecordFailure(request_status);
    }

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = request.runtime_instance_mapping_count;
    return RuntimeAssetWorldObjectAdapterResult::Success(state);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::PreflightSampledTransformsWithTargetDiagnostics(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request) {
    if (request.runtime_instance_mapping_count > 0U && request.runtime_instance_mappings == nullptr) {
        return RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput);
    }

    if (request.runtime_instance_mapping_count > 0U && request.identity_records == nullptr) {
        return RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput);
    }

    if (request.transform_destination == nullptr) {
        return RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidTransformDestination);
    }

    if (request.sampled_value_count == 0U) {
        return RecordFailure(RuntimeAssetWorldObjectAdapterStatus::MissingSampledTransform);
    }

    if (request.sampled_values == nullptr) {
        return RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput);
    }

    std::uint32_t mapping_index = 0U;
    while (mapping_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetWorldObjectAdapterStatus mapping_status = ValidateRuntimeInstanceMapping(
            request,
            mapping_index);
        if (mapping_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            const std::uint64_t failed_target_id =
                request.runtime_instance_mappings[mapping_index].target_id;
            return RecordFailure(mapping_status, mapping_index, failed_target_id, 0U, 0U);
        }

        ++mapping_index;
    }

    std::uint32_t sampled_value_index = 0U;
    while (sampled_value_index < request.sampled_value_count) {
        const AnimationRuntimeSampledValue &sampled_value = request.sampled_values[sampled_value_index];
        if (!sampled_value.target.IsValid()) {
            RuntimeAssetWorldObjectAdapterResult result =
                RecordFailure(RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId);
            result.failed_target_id = sampled_value.target.value;
            return result;
        }

        if (!HasMappedWorldObject(request, sampled_value.target)) {
            RuntimeAssetWorldObjectAdapterResult result =
                RecordFailure(RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound);
            result.failed_target_id = sampled_value.target.value;
            return result;
        }

        ++sampled_value_index;
    }

    RuntimeAssetWorldObjectAdapterState state{};
    state.input_mapping_count = request.runtime_instance_mapping_count;
    return RuntimeAssetWorldObjectAdapterResult::Success(state);
}

RuntimeAssetWorldObjectAdapterSnapshot RuntimeAssetWorldObjectAdapterBridge::Snapshot() const {
    return snapshot_;
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::RecordFailure(
    RuntimeAssetWorldObjectAdapterStatus status) {
    return RecordFailure(status, 0U, 0U, 0U, 0U);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::RecordFailure(
    RuntimeAssetWorldObjectAdapterStatus status,
    std::uint32_t failed_mapping_index,
    std::uint64_t failed_target_id,
    std::uint32_t required_identity_output_count,
    std::uint32_t required_transform_output_count) {
    ++snapshot_.failed_operation_count;
    ++snapshot_.rejected_record_count;
    snapshot_.required_identity_output_count = required_identity_output_count;
    snapshot_.required_transform_output_count = required_transform_output_count;
    snapshot_.required_sampled_transform_value_count = 0U;
    snapshot_.last_status = status;
    RuntimeAssetWorldObjectAdapterResult result =
        RuntimeAssetWorldObjectAdapterResult::Failure(status, failed_mapping_index, failed_target_id);
    result.required_identity_output_count = required_identity_output_count;
    result.required_transform_output_count = required_transform_output_count;
    return result;
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::RecordSuccess(
    const RuntimeAssetWorldObjectAdapterState &state) {
    snapshot_.built_identity_count += state.output_identity_count;
    snapshot_.built_transform_count += state.output_transform_count;
    snapshot_.required_identity_output_count = 0U;
    snapshot_.required_transform_output_count = 0U;
    snapshot_.required_sampled_transform_value_count = 0U;
    snapshot_.last_status = RuntimeAssetWorldObjectAdapterStatus::Success;
    return RuntimeAssetWorldObjectAdapterResult::Success(state);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::RecordTransformApplicationRequestSuccess(
    const RuntimeAssetWorldObjectAdapterState &state) {
    snapshot_.sampled_transform_value_count += state.sampled_transform_value_count;
    snapshot_.required_identity_output_count = 0U;
    snapshot_.required_transform_output_count = 0U;
    snapshot_.required_sampled_transform_value_count = 0U;
    snapshot_.last_status = RuntimeAssetWorldObjectAdapterStatus::Success;
    return RuntimeAssetWorldObjectAdapterResult::Success(state);
}

RuntimeAssetWorldObjectAdapterResult RuntimeAssetWorldObjectAdapterBridge::RecordTransformApplicationSuccess(
    const RuntimeAssetWorldObjectAdapterState &state) {
    snapshot_.applied_transform_value_count += state.applied_transform_value_count;
    snapshot_.updated_world_object_count += state.updated_world_object_count;
    snapshot_.required_identity_output_count = 0U;
    snapshot_.required_transform_output_count = 0U;
    snapshot_.required_sampled_transform_value_count = 0U;
    snapshot_.last_status = RuntimeAssetWorldObjectAdapterStatus::Success;
    return RuntimeAssetWorldObjectAdapterResult::Success(state);
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateRequest(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint32_t *out_failed_mapping_index,
    std::uint64_t *out_failed_target_id,
    std::uint32_t *out_required_identity_output_count,
    std::uint32_t *out_required_transform_output_count) const {
    if (request.runtime_instance_mapping_count > 0U && request.runtime_instance_mappings == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.scene_entities == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSceneEntityInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.scene_transforms == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSceneTransformInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.identity_records == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.output_identities == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityOutput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.output_transforms == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformOutput;
    }

    if (request.runtime_instance_mapping_count > request.output_identity_capacity) {
        if (out_required_identity_output_count != nullptr) {
            *out_required_identity_output_count = request.runtime_instance_mapping_count;
        }

        return RuntimeAssetWorldObjectAdapterStatus::IdentityOutputCapacityExceeded;
    }

    if (request.runtime_instance_mapping_count > request.output_transform_capacity) {
        if (out_required_transform_output_count != nullptr) {
            *out_required_transform_output_count = request.runtime_instance_mapping_count;
        }

        return RuntimeAssetWorldObjectAdapterStatus::TransformOutputCapacityExceeded;
    }

    std::uint32_t mapping_index = 0U;
    while (mapping_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetWorldObjectAdapterStatus mapping_status = ValidateRuntimeInstanceMapping(
            request,
            mapping_index);
        if (mapping_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            if (out_failed_mapping_index != nullptr) {
                *out_failed_mapping_index = mapping_index;
            }

            if (out_failed_target_id != nullptr) {
                *out_failed_target_id = request.runtime_instance_mappings[mapping_index].target_id;
            }

            return mapping_status;
        }

        ++mapping_index;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateTransformApplicationRequest(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request) const {
    if (request.runtime_instance_mapping_count > 0U && request.runtime_instance_mappings == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.identity_records == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput;
    }

    if (request.transform_destination == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformDestination;
    }

    if (request.sampled_value_count == 0U) {
        return RuntimeAssetWorldObjectAdapterStatus::MissingSampledTransform;
    }

    if (request.sampled_values == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput;
    }

    std::uint32_t mapping_index = 0U;
    while (mapping_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetWorldObjectAdapterStatus mapping_status = ValidateRuntimeInstanceMapping(
            request,
            mapping_index);
        if (mapping_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            return mapping_status;
        }

        ++mapping_index;
    }

    return ValidateSampledTransformTargets(request);
}

RuntimeAssetWorldObjectAdapterStatus
RuntimeAssetWorldObjectAdapterBridge::ValidateRuntimeSampledTransformApplicationRequest(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
    std::uint64_t *out_failed_target_id) const {
    if (out_failed_target_id != nullptr) {
        *out_failed_target_id = 0U;
    }

    if (request.runtime_instance_mapping_count > 0U && request.runtime_instance_mappings == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.identity_records == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput;
    }

    if (request.transform_destination == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformDestination;
    }

    if (request.sampled_value_count == 0U) {
        return RuntimeAssetWorldObjectAdapterStatus::MissingSampledTransform;
    }

    if (request.sampled_values == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput;
    }

    std::uint32_t mapping_index = 0U;
    while (mapping_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetWorldObjectAdapterStatus mapping_status = ValidateRuntimeInstanceMapping(
            request,
            mapping_index);
        if (mapping_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            if (out_failed_target_id != nullptr) {
                *out_failed_target_id = request.runtime_instance_mappings[mapping_index].target_id;
            }

            return mapping_status;
        }

        ++mapping_index;
    }

    return ValidateRuntimeSampledTransformTargets(request, out_failed_target_id);
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateTransformSamplerRequest(
    const RuntimeAssetWorldObjectTransformSamplerBridgeRequest &request,
    const RuntimeAssetWorldObjectTransformApplicationRequest *out_request) const {
    if (out_request == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformApplicationOutput;
    }

    if (request.sample_request == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformSampleRequest;
    }

    if (request.sampled_value_output_capacity > 0U && request.sampled_value_output == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.runtime_instance_mappings == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.identity_records == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput;
    }

    if (request.transform_destination == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformDestination;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateProducerPlaybackBatchRequest(
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest &request,
    const RuntimeAssetWorldObjectTransformApplicationRequest *out_request) const {
    if (out_request == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformApplicationOutput;
    }

    if (request.producer_playback_request_count == 0U) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformSampleRequest;
    }

    if (request.producer_playback_requests == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformSampleRequest;
    }

    if (request.sampled_value_scratch_capacity > 0U && request.sampled_value_scratch == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput;
    }

    if (request.sampled_value_output_capacity > 0U && request.sampled_value_output == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateProducerPlaybackBatchCountRequest(
    const RuntimeAssetWorldObjectProducerPlaybackBatchRequest &request) const {
    if (request.producer_playback_request_count == 0U) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformSampleRequest;
    }

    if (request.producer_playback_requests == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformSampleRequest;
    }

    if (request.sampled_value_scratch_capacity > 0U && request.sampled_value_scratch == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateProducerPlaybackRequest(
    const RuntimeAssetWorldObjectProducerPlaybackRequest &request) const {
    if (request.runtime_instance_mapping_count > 0U && request.runtime_instance_mappings == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.identity_records == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput;
    }

    if (request.transform_destination == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformDestination;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateTimelineTransformSampleRequest(
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest &request,
    const RuntimeAssetWorldObjectTransformApplicationRequest *out_request) const {
    if (out_request == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformApplicationOutput;
    }

    if (request.sampled_value_output_capacity > 0U && request.sampled_value_output == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput;
    }

    if (request.sampled_value_scratch_capacity > 0U && request.sampled_value_scratch == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput;
    }

    if (request.sampled_value_scratch_capacity < request.sampled_value_output_capacity) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.runtime_instance_mappings == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.identity_records == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput;
    }

    if (request.transform_destination == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformDestination;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateTimelineTransformSampleCountRequest(
    const RuntimeAssetWorldObjectTimelineTransformSampleRequest &request) const {
    if (request.sampled_value_scratch_capacity > 0U && request.sampled_value_scratch == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.runtime_instance_mappings == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceInput;
    }

    if (request.runtime_instance_mapping_count > 0U && request.identity_records == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidIdentityInput;
    }

    if (request.transform_destination == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformDestination;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateRuntimeInstanceMapping(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint32_t mapping_index) const {
    const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
    if (!mapping.is_valid) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceMapping;
    }

    if (!RuntimeAssetTargetKindIsSupported(mapping.target_kind)) {
        return RuntimeAssetWorldObjectAdapterStatus::UnsupportedTargetKind;
    }

    if (mapping.scene_entity_index >= request.scene_entity_count) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSceneEntityIndex;
    }

    if (mapping.scene_transform_index >= request.scene_transform_count) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSceneTransformIndex;
    }

    const RuntimeAssetSceneEntityRecord &scene_entity = request.scene_entities[mapping.scene_entity_index];
    if (scene_entity.entity_id != mapping.scene_entity_id) {
        return RuntimeAssetWorldObjectAdapterStatus::MissingSceneEntity;
    }

    if (!scene_entity.world_object_id.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
    }

    const RuntimeAssetSceneTransformOutputRecord &scene_transform =
        request.scene_transforms[mapping.scene_transform_index];
    if (!scene_transform.world_object_id.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
    }

    if (!WorldObjectIdsMatch(scene_entity.world_object_id, scene_transform.world_object_id)) {
        return RuntimeAssetWorldObjectAdapterStatus::MissingSceneTransform;
    }

    const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_record =
        FindIdentityRecord(request, mapping.target_id);
    if (identity_record == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::MissingIdentityRecord;
    }

    if (!identity_record->world_object_id.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
    }

    if (!WorldObjectIdsMatch(scene_entity.world_object_id, identity_record->world_object_id)) {
        return RuntimeAssetWorldObjectAdapterStatus::WorldObjectMismatch;
    }

    if (!identity_record->object_handle.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidObjectHandle;
    }

    if (HasDuplicateMappingTargetId(request, mapping_index)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateTargetId;
    }

    if (HasDuplicateWorldObjectId(request, mapping_index, identity_record->world_object_id)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateWorldObjectId;
    }

    if (HasDuplicateObjectHandle(request, mapping_index, identity_record->object_handle)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateObjectHandle;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateRuntimeInstanceMapping(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request,
    std::uint32_t mapping_index) const {
    const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
    if (!mapping.is_valid) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceMapping;
    }

    if (!RuntimeAssetTargetKindIsSupported(mapping.target_kind)) {
        return RuntimeAssetWorldObjectAdapterStatus::UnsupportedTargetKind;
    }

    const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_record =
        FindIdentityRecord(request, mapping.target_id);
    if (identity_record == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::MissingIdentityRecord;
    }

    if (!identity_record->world_object_id.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
    }

    if (!identity_record->object_handle.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidObjectHandle;
    }

    if (HasDuplicateMappingTargetId(request, mapping_index)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateTargetId;
    }

    if (HasDuplicateWorldObjectId(request, mapping_index, identity_record->world_object_id)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateWorldObjectId;
    }

    if (HasDuplicateObjectHandle(request, mapping_index, identity_record->object_handle)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateObjectHandle;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateRuntimeInstanceMapping(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
    std::uint32_t mapping_index) const {
    const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
    if (!mapping.is_valid) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidRuntimeInstanceMapping;
    }

    if (!RuntimeAssetTargetKindIsSupported(mapping.target_kind)) {
        return RuntimeAssetWorldObjectAdapterStatus::UnsupportedTargetKind;
    }

    const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_record =
        FindIdentityRecord(request, mapping.target_id);
    if (identity_record == nullptr) {
        return RuntimeAssetWorldObjectAdapterStatus::MissingIdentityRecord;
    }

    if (!identity_record->world_object_id.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
    }

    if (!identity_record->object_handle.IsValid()) {
        return RuntimeAssetWorldObjectAdapterStatus::InvalidObjectHandle;
    }

    if (HasDuplicateMappingTargetId(request, mapping_index)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateTargetId;
    }

    if (HasDuplicateWorldObjectId(request, mapping_index, identity_record->world_object_id)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateWorldObjectId;
    }

    if (HasDuplicateObjectHandle(request, mapping_index, identity_record->object_handle)) {
        return RuntimeAssetWorldObjectAdapterStatus::DuplicateObjectHandle;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateSampledTransformTargets(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request) const {
    std::uint32_t sampled_value_index = 0U;
    while (sampled_value_index < request.sampled_value_count) {
        const AnimationRuntimeSampledValue &sampled_value = request.sampled_values[sampled_value_index];
        if (!sampled_value.target.IsValid()) {
            return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
        }

        if (!HasMappedWorldObject(request, sampled_value.target)) {
            return RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound;
        }

        ++sampled_value_index;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::ValidateRuntimeSampledTransformTargets(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
    std::uint64_t *out_failed_target_id) const {
    std::uint32_t sampled_value_index = 0U;
    while (sampled_value_index < request.sampled_value_count) {
        const RuntimeAssetWorldObjectRuntimeSampledTransformValue &sampled_value =
            request.sampled_values[sampled_value_index];
        if (out_failed_target_id != nullptr) {
            *out_failed_target_id = sampled_value.target_id;
        }

        WorldObjectId world_object_id{};
        if (!ResolveRuntimeSampledTransformTarget(request, sampled_value.target_id, &world_object_id)) {
            return RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound;
        }

        if (!RuntimeSampledTransformChannelIsSupported(sampled_value.channel)) {
            return RuntimeAssetWorldObjectAdapterStatus::UnsupportedSampledTransformChannel;
        }

        if (!std::isfinite(sampled_value.value)) {
            return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformValue;
        }

        const WorldTransformResult query_result = request.transform_destination->Query(world_object_id);
        const RuntimeAssetWorldObjectAdapterStatus query_status = MapWorldTransformStatus(query_result.status);
        if (query_status != RuntimeAssetWorldObjectAdapterStatus::Success) {
            return query_status;
        }

        ++sampled_value_index;
    }

    if (out_failed_target_id != nullptr) {
        *out_failed_target_id = 0U;
    }

    return RuntimeAssetWorldObjectAdapterStatus::Success;
}

const RuntimeAssetWorldObjectAdapterIdentityRecord *RuntimeAssetWorldObjectAdapterBridge::FindIdentityRecord(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint64_t target_id) const {
    std::uint32_t identity_index = 0U;
    while (identity_index < request.identity_record_count) {
        const RuntimeAssetWorldObjectAdapterIdentityRecord &identity_record =
            request.identity_records[identity_index];
        if (identity_record.target_id == target_id) {
            return &identity_record;
        }

        ++identity_index;
    }

    return nullptr;
}

const RuntimeAssetWorldObjectAdapterIdentityRecord *RuntimeAssetWorldObjectAdapterBridge::FindIdentityRecord(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request,
    std::uint64_t target_id) const {
    std::uint32_t identity_index = 0U;
    while (identity_index < request.identity_record_count) {
        const RuntimeAssetWorldObjectAdapterIdentityRecord &identity_record =
            request.identity_records[identity_index];
        if (identity_record.target_id == target_id) {
            return &identity_record;
        }

        ++identity_index;
    }

    return nullptr;
}

const RuntimeAssetWorldObjectAdapterIdentityRecord *RuntimeAssetWorldObjectAdapterBridge::FindIdentityRecord(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
    std::uint64_t target_id) const {
    std::uint32_t identity_index = 0U;
    while (identity_index < request.identity_record_count) {
        const RuntimeAssetWorldObjectAdapterIdentityRecord &identity_record =
            request.identity_records[identity_index];
        if (identity_record.target_id == target_id) {
            return &identity_record;
        }

        ++identity_index;
    }

    return nullptr;
}

const RuntimeAssetRuntimeInstanceMappingRecord *RuntimeAssetWorldObjectAdapterBridge::FindRuntimeInstanceMapping(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
    std::uint64_t target_id) const {
    std::uint32_t mapping_index = 0U;
    while (mapping_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &mapping =
            request.runtime_instance_mappings[mapping_index];
        if (mapping.target_id == target_id) {
            return &mapping;
        }

        ++mapping_index;
    }

    return nullptr;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateMappingTargetId(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint32_t mapping_index) const {
    const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        if (other_mapping.target_id == mapping.target_id) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateMappingTargetId(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request,
    std::uint32_t mapping_index) const {
    const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        if (other_mapping.target_id == mapping.target_id) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateMappingTargetId(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
    std::uint32_t mapping_index) const {
    const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        if (other_mapping.target_id == mapping.target_id) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateWorldObjectId(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint32_t mapping_index,
    WorldObjectId world_object_id) const {
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *other_identity =
            FindIdentityRecord(request, other_mapping.target_id);
        if (other_identity != nullptr && WorldObjectIdsMatch(other_identity->world_object_id, world_object_id)) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateWorldObjectId(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request,
    std::uint32_t mapping_index,
    WorldObjectId world_object_id) const {
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *other_identity =
            FindIdentityRecord(request, other_mapping.target_id);
        if (other_identity != nullptr && WorldObjectIdsMatch(other_identity->world_object_id, world_object_id)) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateWorldObjectId(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
    std::uint32_t mapping_index,
    WorldObjectId world_object_id) const {
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *other_identity =
            FindIdentityRecord(request, other_mapping.target_id);
        if (other_identity != nullptr && WorldObjectIdsMatch(other_identity->world_object_id, world_object_id)) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateObjectHandle(
    const RuntimeAssetWorldObjectAdapterRequest &request,
    std::uint32_t mapping_index,
    ObjectHandle object_handle) const {
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *other_identity =
            FindIdentityRecord(request, other_mapping.target_id);
        if (other_identity != nullptr && ObjectHandlesMatch(other_identity->object_handle, object_handle)) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateObjectHandle(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request,
    std::uint32_t mapping_index,
    ObjectHandle object_handle) const {
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *other_identity =
            FindIdentityRecord(request, other_mapping.target_id);
        if (other_identity != nullptr && ObjectHandlesMatch(other_identity->object_handle, object_handle)) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasDuplicateObjectHandle(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
    std::uint32_t mapping_index,
    ObjectHandle object_handle) const {
    std::uint32_t other_index = mapping_index + 1U;
    while (other_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &other_mapping =
            request.runtime_instance_mappings[other_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *other_identity =
            FindIdentityRecord(request, other_mapping.target_id);
        if (other_identity != nullptr && ObjectHandlesMatch(other_identity->object_handle, object_handle)) {
            return true;
        }

        ++other_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::HasMappedWorldObject(
    const RuntimeAssetWorldObjectTransformApplicationRequest &request,
    WorldObjectId world_object_id) const {
    std::uint32_t mapping_index = 0U;
    while (mapping_index < request.runtime_instance_mapping_count) {
        const RuntimeAssetRuntimeInstanceMappingRecord &mapping = request.runtime_instance_mappings[mapping_index];
        const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_record =
            FindIdentityRecord(request, mapping.target_id);
        if (identity_record != nullptr && WorldObjectIdsMatch(identity_record->world_object_id, world_object_id)) {
            return true;
        }

        ++mapping_index;
    }

    return false;
}

bool RuntimeAssetWorldObjectAdapterBridge::ResolveRuntimeSampledTransformTarget(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
    std::uint64_t target_id,
    WorldObjectId *out_world_object_id) const {
    if (out_world_object_id == nullptr) {
        return false;
    }

    const RuntimeAssetRuntimeInstanceMappingRecord *mapping = FindRuntimeInstanceMapping(request, target_id);
    if (mapping == nullptr) {
        return false;
    }

    const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_record =
        FindIdentityRecord(request, mapping->target_id);
    if (identity_record == nullptr) {
        return false;
    }

    if (!identity_record->world_object_id.IsValid()) {
        return false;
    }

    *out_world_object_id = identity_record->world_object_id;
    return true;
}

bool RuntimeAssetWorldObjectAdapterBridge::IsFirstRuntimeSampledTransformTargetOccurrence(
    const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
    std::uint32_t sampled_value_index) const {
    const RuntimeAssetWorldObjectRuntimeSampledTransformValue &current =
        request.sampled_values[sampled_value_index];
    std::uint32_t index = 0U;
    while (index < sampled_value_index) {
        const RuntimeAssetWorldObjectRuntimeSampledTransformValue &candidate = request.sampled_values[index];
        if (candidate.target_id == current.target_id) {
            return false;
        }

        ++index;
    }

    return true;
}

bool RuntimeAssetWorldObjectAdapterBridge::ObjectHandlesMatch(
    ObjectHandle left,
    ObjectHandle right) const {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::MapAnimationStatus(
    AnimationRuntimeStatus status) const {
    switch (status) {
    case AnimationRuntimeStatus::Success:
        return RuntimeAssetWorldObjectAdapterStatus::Success;
    case AnimationRuntimeStatus::NullPointer:
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformDestination;
    case AnimationRuntimeStatus::MissingSample:
        return RuntimeAssetWorldObjectAdapterStatus::MissingSampledTransform;
    case AnimationRuntimeStatus::InvalidTarget:
        return RuntimeAssetWorldObjectAdapterStatus::InvalidWorldObjectId;
    case AnimationRuntimeStatus::TargetNotFound:
        return RuntimeAssetWorldObjectAdapterStatus::TransformTargetNotFound;
    case AnimationRuntimeStatus::UnsupportedChannel:
        return RuntimeAssetWorldObjectAdapterStatus::UnsupportedSampledTransformChannel;
    case AnimationRuntimeStatus::InvalidKeyframe:
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformValue;
    case AnimationRuntimeStatus::TransformApplyFailed:
    case AnimationRuntimeStatus::MissingClip:
    case AnimationRuntimeStatus::InvalidClip:
    case AnimationRuntimeStatus::MissingTrack:
    case AnimationRuntimeStatus::InvalidTrack:
    case AnimationRuntimeStatus::MissingKeyframe:
    case AnimationRuntimeStatus::InvalidTime:
    case AnimationRuntimeStatus::TimeOutOfRange:
    case AnimationRuntimeStatus::UnsupportedInterpolation:
    case AnimationRuntimeStatus::OutputCapacityExceeded:
    case AnimationRuntimeStatus::LayerCapacityExceeded:
    default:
        break;
    }

    return RuntimeAssetWorldObjectAdapterStatus::TransformApplyFailed;
}

RuntimeAssetWorldObjectAdapterStatus RuntimeAssetWorldObjectAdapterBridge::MapAnimationSampleStatus(
    AnimationRuntimeStatus status) const {
    switch (status) {
    case AnimationRuntimeStatus::Success:
        return RuntimeAssetWorldObjectAdapterStatus::Success;
    case AnimationRuntimeStatus::UnsupportedChannel:
        return RuntimeAssetWorldObjectAdapterStatus::UnsupportedSampledTransformChannel;
    case AnimationRuntimeStatus::InvalidKeyframe:
        return RuntimeAssetWorldObjectAdapterStatus::InvalidSampledTransformValue;
    case AnimationRuntimeStatus::OutputCapacityExceeded:
        return RuntimeAssetWorldObjectAdapterStatus::SampledTransformOutputCapacityExceeded;
    case AnimationRuntimeStatus::NullPointer:
        return RuntimeAssetWorldObjectAdapterStatus::InvalidTransformSampleRequest;
    case AnimationRuntimeStatus::MissingClip:
    case AnimationRuntimeStatus::InvalidClip:
    case AnimationRuntimeStatus::MissingTrack:
    case AnimationRuntimeStatus::InvalidTrack:
    case AnimationRuntimeStatus::MissingKeyframe:
    case AnimationRuntimeStatus::InvalidTime:
    case AnimationRuntimeStatus::TimeOutOfRange:
    case AnimationRuntimeStatus::UnsupportedInterpolation:
    case AnimationRuntimeStatus::LayerCapacityExceeded:
    case AnimationRuntimeStatus::MissingSample:
    case AnimationRuntimeStatus::InvalidTarget:
    case AnimationRuntimeStatus::TargetNotFound:
    case AnimationRuntimeStatus::TransformApplyFailed:
    default:
        break;
    }

    return RuntimeAssetWorldObjectAdapterStatus::TransformSamplingFailed;
}
}
